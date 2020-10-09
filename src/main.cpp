#include <Arduino.h>
#include <AmbientSensor.h>
#include <Relay.h>

#include <wiringPi.h>
#include <pthread.h>

#include <update_threads.h>

#include <iostream>


#define UART_BUS "/dev/serial0"
#define I2C_BUS "/dev/i2c-1"

#define I2C_DEVICE_ADDR 0x76

#define COOLER_PIN 18
#define RESISTOR_PIN 16

volatile float INTERNAL_TEMPERATURE = 0;
volatile float AMBIENT_TEMPERATURE = 0;
volatile float POTENTIOMETER_REFERENCE = 0;

int main(void) {
    Arduino arduino = Arduino(UART_BUS);
    AmbientSensor ambient_sensor = AmbientSensor(I2C_BUS, I2C_DEVICE_ADDR);
    Relay cooler = Relay(COOLER_PIN);
    Relay resistor = Relay(RESISTOR_PIN);

    unsigned char auth[4] =  {1,3,9,9};
    
    wiringPiSetup();

    pthread_t potentiometer_tid;
    pthread_t internal_sensor_tid;
    pthread_t ambient_sensor_tid;

    pthread_create(&potentiometer_tid, NULL, potentiometer_thread, (void *)&POTENTIOMETER_REFERENCE);
    pthread_create(&internal_sensor_tid, NULL, internal_sensor_thread, (void *)&INTERNAL_TEMPERATURE);
    pthread_create(&external_sensor_tid, NULL, ambient_sensor_thread, (void *)&AMBIENT_TEMPERATURE);

    std::cout << arduino.read_internal_temperature(auth) << std::endl;

    std::cout << ambient_sensor.read_temperature() <<std::endl;
    cooler.turn_on();
    return 0;
}
