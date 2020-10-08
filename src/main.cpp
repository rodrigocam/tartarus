#include <Arduino.h>

#include <iostream>

int main(void) {
    Arduino arduino = Arduino("/dev/serial0");
    unsigned char auth[4] =  {1,3,9,9};
    std::cout << arduino.read_internal_temperature(auth) << std::endl;
    return 0;
}
