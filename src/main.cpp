#include <Arduino.h>
#include <AmbientSensor.h>
#include <Relay.h>
#include <LCD.h>

#include <wiringPi.h>
#include <pthread.h>
#include <curses.h>

#include <iostream>
#include <unistd.h>
#include <time.h>

#define UART_BUS "/dev/serial0"
#define I2C_BUS "/dev/i2c-1"

#define I2C_DEVICE_ADDR 0x76

#define COOLER_PIN 24
#define RESISTOR_PIN 23

#define CSV_FORMAT "%f, %f, %f, %02d/%02d/%02d:%02d:%02d:%02d\n"

static unsigned char AUTH[4] =  {1,3,9,9};

static int HISTERESE = 4;

volatile float REFERENCE_TEMPERATURE = 0;
volatile float INTERNAL_TEMPERATURE = 0;
volatile float AMBIENT_TEMPERATURE = 0;
volatile float POTENTIOMETER_REFERENCE = 0;

void* potentiometer_thread(void* arg) {
    Arduino* arduino = (Arduino*)arg;

    while(1) {
        /* fprintf(stderr, "potentiometer thread\n"); */
        POTENTIOMETER_REFERENCE = arduino->read_potentiometer(AUTH);
        sleep(1);
    }
}

void* internal_sensor_thread(void* arg) {
    Arduino* arduino = (Arduino*)arg;
    while(1) {
        INTERNAL_TEMPERATURE = arduino->read_internal_temperature(AUTH);
        sleep(1);
    }
}

void* ambient_sensor_thread(void* arg) {
    AmbientSensor* ambient_sensor = (AmbientSensor*)arg;
    
    while(1) {
        AMBIENT_TEMPERATURE = ambient_sensor->read_temperature();
        sleep(1);
    }
}

void* lcd_thread(void* arg) {
    LCD* lcd = (LCD*)arg;
    while(1) {
        lcd->write(INTERNAL_TEMPERATURE, AMBIENT_TEMPERATURE, POTENTIOMETER_REFERENCE);
        sleep(2);
    }
}

void* csv_thread(void* arg) {
    FILE *f = fopen("log.csv", "a");
    
    while(1) {
        time_t t = time(NULL);
        struct tm time_stamp = *localtime(&t);


        fprintf(f, CSV_FORMAT,
            INTERNAL_TEMPERATURE, AMBIENT_TEMPERATURE, POTENTIOMETER_REFERENCE, 
            time_stamp.tm_mday, time_stamp.tm_mon + 1,
            time_stamp.tm_year + 1900, time_stamp.tm_hour,
            time_stamp.tm_min, time_stamp.tm_sec
        );

        sleep(2);
    }

    fclose(f);
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
    pthread_t lcd_tid;
    pthread_t csv_tid;

    /* pthread_create(&potentiometer_tid, NULL, potentiometer_thread, (void *)&arduino); */
    pthread_create(&internal_sensor_tid, NULL, internal_sensor_thread, (void *)&arduino);
    pthread_create(&ambient_sensor_tid, NULL, ambient_sensor_thread, (void *)&ambient_sensor);
    pthread_create(&lcd_tid, NULL, lcd_thread, (void*)&lcd);
    pthread_create(&csv_tid, NULL, csv_thread, NULL);

    /* Initialize ncurses */
    int row, col;
    /* initscr(); */
    /* noecho(); */
    /* nodelay(stdscr, TRUE); */
    /* cbreak(); */
    /* getmaxyx(stdscr, row, col); */
    /* curs_set(0); */

    /* WINDOW *temperature_window = newwin(10, 40, row/2-10, col/2-20); */
    /* box(temperature_window, 0, 0); */

    while(1) {
        if(INTERNAL_TEMPERATURE < REFERENCE_TEMPERATURE - HISTERESE) {
            resistor.turn_on();
            cooler.turn_off();
        } else if(INTERNAL_TEMPERATURE > REFERENCE_TEMPERATURE + HISTERESE) {
            resistor.turn_off();
            cooler.turn_on();
        }
        
        /* mvwprintw(temperature_window, 2, 5, "Internal Temperature: %.2f", INTERNAL_TEMPERATURE); */
        /* mvwprintw(temperature_window, 6, 5, "Ambient Temperature: %.2f", AMBIENT_TEMPERATURE); */

        /* wrefresh(temperature_window); */
    }

    return 0;
}
