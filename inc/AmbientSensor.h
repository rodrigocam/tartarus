#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

#include <bme280.h>

struct identifier {
    uint8_t dev_addr;
    int8_t fd;
};

int8_t i2c_read(uint8_t reg_addr, uint8_t *data, uint32_t len, void *intf_ptr);

int8_t i2c_write(uint8_t reg_addr, const uint8_t *data, uint32_t len, void *intf_ptr);

void delay_us(uint32_t period, void *intf_ptr);

class AmbientSensor {
    public:
        AmbientSensor(const char* bus, unsigned char device_addr);
        ~AmbientSensor();
        float read_temperature();

    private:
        struct bme280_dev bme;
        struct identifier id;
};
