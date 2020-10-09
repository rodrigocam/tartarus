#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#include <Arduino.h>

Arduino::Arduino(char* bus) {
    struct termios options;

    serial_fd = -1;

    serial_fd = open(bus, O_RDWR | O_NOCTTY);

    if(serial_fd == -1) {
        fprintf(stderr, "Failed to open `%s`\n", bus);
        exit(1);
    }

    tcgetattr(serial_fd, &options);
    options.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(serial_fd, TCIFLUSH);
    tcsetattr(serial_fd, TCSANOW, &options);
}

Arduino::~Arduino() {
    close(serial_fd);
}

void Arduino::send_package(RequestPackage package, size_t package_size) {
    const int writed_size = write(serial_fd, package.bytes, package_size);

    if(writed_size < 0) {
        perror("Failed to send package");
        exit(1);
    }

    if(writed_size < (int) package_size) {
        fprintf(stderr, "Failed to send package, impossible to write the entire package content\n");
        exit(1);
    }
}

void Arduino::receive_package(ResponsePackage* response, size_t response_size) {
    const int read_size = read(serial_fd, response->bytes, response_size);
    fprintf(stderr, "saiu\n");

    if(read_size < 0) {
        perror("Failed to receive package, error number %d: \n");
        exit(1);
    }

    if(read_size < (int) response_size) {
        fprintf(stderr, "Failed to receive package, impossible to read entire response. Specified size: %d, Received size: %d\n", (int) response_size, read_size);
        exit(1);
    }
}

float Arduino::read_float(unsigned char command, unsigned char* auth_key) {
    ResponsePackage* response = (ResponsePackage*)malloc(sizeof(ResponsePackage));
    RequestPackage* package = (RequestPackage*)malloc(sizeof(RequestPackage));

    package->data.cmd = command;
    package->data.auth = (unsigned char*)malloc(4);
    memcpy(package->data.auth, auth_key, 4);
    
    fprintf(stderr, "sending\n");
    Arduino::send_package(*package, sizeof(package->bytes));
    sleep(1);
    fprintf(stderr, "receiveing\n");
    Arduino::receive_package(response, sizeof(float));

    return response->as_float;
}

float Arduino::read_potentiometer(unsigned char* auth_key) {
    return Arduino::read_float(POTENTIOMETER_COMMAND, auth_key);
}

float Arduino::read_internal_temperature(unsigned char* auth_key) {
    return Arduino::read_float(TEMPERATURE_COMMAND, auth_key);
}
