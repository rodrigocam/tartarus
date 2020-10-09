#include <Arduino.h>
#include <AmbientSensor.h>
#include <Relay.h>
#include <LCD.h>

#include <wiringPi.h>
#include <pthread.h>

#include <update_threads.h>

#include <iostream>
#include <unistd.h>

#define UART_BUS "/dev/serial0"
#define I2C_BUS "/dev/i2c-1"

#define I2C_DEVICE_ADDR 0x76

#define COOLER_PIN 24
#define RESISTOR_PIN 23

static unsigned char AUTH[4] =  {1,3,9,9};

volatile float INTERNAL_TEMPERATURE = 0;
volatile float AMBIENT_TEMPERATURE = 0;
volatile float POTENTIOMETER_REFERENCE = 0;

void* potentiometer_thread(void* arg) {
    Arduino* arduino = (Arduino*)arg;

    while(1) {
        fprintf(stderr, "potentiometer thread\n");
        POTENTIOMETER_REFERENCE = arduino->read_potentiometer(AUTH);
        sleep(1);
    }
}

void* internal_sensor_thread(void* arg) {
    Arduino* arduino = (Arduino*)arg;

    while(1) {
        fprintf(stderr, "internal sensor thread\n");
        INTERNAL_TEMPERATURE = arduino->read_internal_temperature(AUTH);
        sleep(1);
    }
}

void* ambient_sensor_thread(void* arg) {
    AmbientSensor* ambient_sensor = (AmbientSensor*)arg;
    
    while(1) {
        fprintf(stderr, "ambient sensor thread\n");
        AMBIENT_TEMPERATURE = ambient_sensor->read_temperature();
        sleep(1);
    }
}

int main(void) {
    wiringPiSetupGpio();
    
    Arduino arduino = Arduino(UART_BUS);
    AmbientSensor ambient_sensor = AmbientSensor(I2C_BUS, I2C_DEVICE_ADDR);
    Relay cooler = Relay(COOLER_PIN);
    Relay resistor = Relay(RESISTOR_PIN);
    LCD lcd = LCD(0x27);

    pthread_t potentiometer_tid;
    pthread_t internal_sensor_tid;
    pthread_t ambient_sensor_tid;

    /* pthread_create(&potentiometer_tid, NULL, potentiometer_thread, (void *)&arduino); */
    pthread_create(&internal_sensor_tid, NULL, internal_sensor_thread, (void *)&arduino);
    pthread_create(&ambient_sensor_tid, NULL, ambient_sensor_thread, (void *)&ambient_sensor);

    while(1) {
        lcd.write(0, AMBIENT_TEMPERATURE, 0);
        /* std::cout << "Potenciometer: " << POTENTIOMETER_REFERENCE << std::endl; */
        sleep(2);
    }

    /* cooler.turn_on(); */
    return 0;
}
