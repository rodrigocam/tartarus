#include <Arduino.h>
#include <AmbientSensor.h>

#include <iostream>

int main(void) {
    Arduino arduino = Arduino("/dev/serial0");
    AmbientSensor ambient_sensor = AmbientSensor("/dev/i2c-1", 0x76);

    unsigned char auth[4] =  {1,3,9,9};
    std::cout << arduino.read_internal_temperature(auth) << std::endl;

    std::cout << ambient_sensor.read_temperature() <<std::endl;
    return 0;
}
