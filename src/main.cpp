#include <Arduino.h>
#include <AmbientSensor.h>
#include <Relay.h>
#include <LCD.h>

#include <iostream>
#include <string>

#include <wiringPi.h>
#include <pthread.h>
#include <curses.h>

#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <string.h>

#define UART_BUS "/dev/serial0"
#define I2C_BUS "/dev/i2c-1"

#define I2C_DEVICE_ADDR 0x76

#define COOLER_PIN 24
#define RESISTOR_PIN 23

#define CSV_FORMAT "%f, %f, %f, %02d/%02d/%02d:%02d:%02d:%02d\n"

static bool RUNNING = true;

static unsigned char AUTH[4] =  {1, 3, 9, 9};

static int HISTERESE = 4;

volatile float REFERENCE_TEMPERATURE = 0;
volatile float INTERNAL_TEMPERATURE = 0;
volatile float AMBIENT_TEMPERATURE = 0;
volatile float POTENTIOMETER_REFERENCE = 0;
volatile float USER_INPUT = 0;

volatile bool POTENTIOMETER_STATUS = false; // false - off / true - on
static std::string COOLER_STATUS = "Off";
static std::string RESISTOR_STATUS = "Off";

pthread_mutex_t ARDUINO_LOCK;

void* potentiometer_thread(void*);
void* internal_sensor_thread(void*);
void* ambient_sensor_thread(void*);
void* lcd_thread(void*);
void* csv_thread(void*);
void* input_thread(void*);

void sighandler(int);

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
    pthread_t input_tid;

    /* Initialize ncurses */
    initscr();
    nodelay(stdscr, TRUE);
    cbreak();
    curs_set(0);

    pthread_create(&potentiometer_tid, NULL, potentiometer_thread, (void *)&arduino);
    pthread_create(&internal_sensor_tid, NULL, internal_sensor_thread, (void *)&arduino);
    pthread_create(&ambient_sensor_tid, NULL, ambient_sensor_thread, (void *)&ambient_sensor);
    pthread_create(&lcd_tid, NULL, lcd_thread, (void*)&lcd);
    pthread_create(&csv_tid, NULL, csv_thread, NULL);
    pthread_create(&input_tid, NULL, input_thread, NULL);

    signal(SIGINT, sighandler);

    while(RUNNING) {
        if(cooler.status()) {
            COOLER_STATUS = "On ";
        } else {
            COOLER_STATUS = "Off";
        }

        if(resistor.status()) {
            RESISTOR_STATUS = "On ";
        } else {
            RESISTOR_STATUS = "Off";
        }

        if(POTENTIOMETER_STATUS) {
            REFERENCE_TEMPERATURE = POTENTIOMETER_REFERENCE;
        } else {
            REFERENCE_TEMPERATURE = USER_INPUT;
        }

        if(INTERNAL_TEMPERATURE < REFERENCE_TEMPERATURE - HISTERESE) {
            // Increasing temperature
            resistor.turn_on();
            cooler.turn_off();
        } else if(INTERNAL_TEMPERATURE > REFERENCE_TEMPERATURE + HISTERESE) {
            // Decreasing temperature
            cooler.turn_on();
            resistor.turn_off();
        }
    }
    
    endwin();
    return 0;
}

void sighandler(int signum) {
    RUNNING = false;
}

void* potentiometer_thread(void* arg) {
    Arduino* arduino = (Arduino*)arg;

    while(RUNNING) {
        pthread_mutex_lock(&ARDUINO_LOCK);
        POTENTIOMETER_REFERENCE = arduino->read_potentiometer(AUTH);
        pthread_mutex_unlock(&ARDUINO_LOCK);
        sleep(0.4);
    }

    return NULL;
}

void* internal_sensor_thread(void* arg) {
    Arduino* arduino = (Arduino*)arg;
    
    while(RUNNING) {
        pthread_mutex_lock(&ARDUINO_LOCK);
        INTERNAL_TEMPERATURE = arduino->read_internal_temperature(AUTH);
        pthread_mutex_unlock(&ARDUINO_LOCK);
        sleep(0.4);
    }

    return NULL;
}

void* ambient_sensor_thread(void* arg) {
    AmbientSensor* ambient_sensor = (AmbientSensor*)arg;
    
    while(RUNNING) {
        AMBIENT_TEMPERATURE = ambient_sensor->read_temperature();
        sleep(0.4);
    }
    
    return NULL;
}

void* lcd_thread(void* arg) {
    LCD* lcd = (LCD*)arg;
    
    while(RUNNING) {
        lcd->write(INTERNAL_TEMPERATURE, AMBIENT_TEMPERATURE, REFERENCE_TEMPERATURE);
        sleep(1);
    }
    
    return NULL;
}

void* csv_thread(void* arg) {
    FILE *f = fopen("log.csv", "a");
    
    while(RUNNING) {
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
    return NULL;
}

void* input_thread(void* arg) {
    int row, col;
    getmaxyx(stdscr, row, col);
    
    WINDOW *menu_window = newwin(10, 40, row/2, col/2-20);
    
    WINDOW *temperature_window = newwin(10, 40, row/2-10, col/2-60);
    
    WINDOW *status_window = newwin(10, 40, row/2-10, col/2+20);
    
    int key_pressed;
    
    while(RUNNING) {
        noecho();
        box(menu_window, 0, 0);
        box(temperature_window, 0, 0);
        box(status_window, 0, 0);
        
        mvwprintw(menu_window, 1, 5, "P - Switch potentiometer state");
        mvwprintw(menu_window, 2, 5, "I - Insert desired temperature");
        mvwprintw(menu_window, 3, 5, "H - Insert desired histerese");
        
        key_pressed = getch();
       
        switch((char)key_pressed) {
            case 'i':
                echo();
                mvwprintw(menu_window, 6, 5, "Temperature value: ");
                wscanw(menu_window, "%f", &USER_INPUT);
                wclear(menu_window);
                break;
            case 'p':
                POTENTIOMETER_STATUS = !POTENTIOMETER_STATUS;
                break;
            case 'h':
                echo();
                mvwprintw(menu_window, 6, 5, "Histerese value: ");
                wscanw(menu_window, "%d", &HISTERESE);
                wclear(menu_window);
                break;
            default:
                break;
        }
        
        mvwprintw(temperature_window, 2, 5, "Internal Temperature: %.2f", INTERNAL_TEMPERATURE);
        mvwprintw(temperature_window, 4, 5, "Ambient Temperature: %.2f", AMBIENT_TEMPERATURE);
        mvwprintw(temperature_window, 6, 5, "Reference Temperature: %.2f", REFERENCE_TEMPERATURE);
        mvwprintw(temperature_window, 8, 5, "Histerese: %d", HISTERESE);

        if(POTENTIOMETER_STATUS) {
            mvwprintw(status_window, 2, 5, "Potentiometer: %s", "On ");
        } else {
            mvwprintw(status_window, 2, 5, "Potentiometer: %s", "Off");
        }

        mvwprintw(status_window, 4, 5, "Cooler: %s", COOLER_STATUS.c_str());
        mvwprintw(status_window, 6, 5, "Resistor: %s", RESISTOR_STATUS.c_str());
        
        wrefresh(menu_window); 
        wrefresh(temperature_window);
        wrefresh(status_window);
       
        // 60 fps
        napms(1000 / 60);
    }
    
    endwin();
    return NULL;
}
