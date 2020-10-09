#include <stdio.h>

#include <wiringPi.h>
#include <wiringPiI2C.h>

#include <i2clcd.h>
#include <LCD.h>

LCD::LCD(unsigned char addr) {
    device_addr = addr;
    file_descriptor = wiringPiI2CSetup(device_addr);
    lcd_init(file_descriptor);
}

LCD::~LCD(){
    ClrLcd(file_descriptor);
}

void LCD::write(float internal_temperature, float ambient_temperature, float reference_temperature) {
    ClrLcd(file_descriptor);
    lcdLoc(file_descriptor, 0x80);

    char line1_buffer[20];
    sprintf(line1_buffer, "TI:%.1f TE:%.1f", internal_temperature, ambient_temperature);
    typeln(file_descriptor, line1_buffer);

    lcdLoc(file_descriptor, 0xC0);
    char line2_buffer[20];
    sprintf(line2_buffer, "TR:%.1f", reference_temperature);
    typeln(file_descriptor, line2_buffer);
}
