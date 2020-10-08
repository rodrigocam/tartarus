#include <stdlib.h>

#define TEMPERATURE_COMMAND 0xA1
#define POTENTIOMETER_COMMAND 0xA2

typedef union {
    struct {
        unsigned char cmd;
        unsigned char* auth;
    } data;
    unsigned char bytes[5];
} RequestPackage;


typedef union {
    float as_float;
    unsigned char bytes[4];
} ResponsePackage;

class Arduino {
    public:
        Arduino(char* bus);
        ~Arduino();
        float read_potentiometer(unsigned char* auth_key);
        float read_internal_temperature(unsigned char* auth_key);

    private:
        int serial_fd;
        void send_package(RequestPackage package, size_t package_size);
        void receive_package(ResponsePackage* response, size_t response_size);
        float read_float(unsigned char command, unsigned char* auth_key);
};
