#include <stdlib.h>
#include <stdio.h>

#include <linux/i2c-dev.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include <AmbientSensor.h>

int8_t i2c_read(uint8_t reg_addr, uint8_t *data, uint32_t len, void *intf_ptr) {
    struct identifier id = *((struct identifier *)intf_ptr);

    write(id.fd, &reg_addr, 1);
    read(id.fd, data, len);

    return 0;
}

int8_t i2c_write(uint8_t reg_addr, const uint8_t *data, uint32_t len, void *intf_ptr) {
    uint8_t *buf;
    struct identifier id;

    id = *((struct identifier *)intf_ptr);

    buf = malloc(len + 1);
    buf[0] = reg_addr;
    memcpy(buf + 1, data, len);
    
    if (write(id.fd, buf, len + 1) < (uint16_t)len) {
        return BME280_E_COMM_FAIL;
    }
    
    free(buf);

    return BME280_OK;
}

void delay_us(uint32_t period, void *intf_ptr) {
    usleep(period);
}

AmbientSensor::AmbientSensor(const char* bus, unsigned char device_addr) {
    id.dev_addr = device_addr;

    int8_t result = BME280_OK;

    if((id.fd = open(bus, O_RDWR)) < 0) {
        fprintf(stderr, "Failed to open i2c bus %s\n", bus);
        exit(1);
    }

    if(ioctl(id.fd, I2C_SLAVE, id.dev_addr) < 0) {
        fprintf(stderr, "Failed to acquire bus access\n");
        exit(1);
    }

    bme.intf = BME280_I2C_INTF;
    bme.read = i2c_read;
    bme.write = i2c_write;
    bme.delay_us = delay_us;
    bme.intf_ptr = &id;

    result = bme280_init(&bme);
    
    if(result != BME280_OK) {
        fprintf(stderr, "Failed to initialize the device (code %+d).\n", result);
        exit(1);
    }

}

AmbientSensor::~AmbientSensor() {
    close(id.fd);
}

float AmbientSensor::read_temperature() {
    struct bme280_data complete_data;
    uint8_t settings_selection = BME280_OSR_TEMP_SEL | BME280_FILTER_SEL;

    int8_t result = bme280_set_sensor_settings(settings_selection, &bme);

    if (result != BME280_OK) {
        fprintf(stderr, "Failed to set sensor settings (code %+d).", result);
        exit(1);
    }

    uint32_t request_delay = bme280_cal_meas_delay(&bme.settings);

    result = bme280_set_sensor_mode(BME280_NORMAL_MODE, &bme);
    if (result != BME280_OK) {
        fprintf(stderr, "Failed to set sensor mode (code %+d).", result);
        exit(1);
    }
    
    bme.delay_us(request_delay, &bme.intf_ptr);
    result = bme280_get_sensor_data(BME280_TEMP, &complete_data, &bme);

    if (result != BME280_OK) {
        fprintf(stderr, "Failed to get sensor data (code %+d).", result);
        exit(1);
    }

    return (float) complete_data.temperature;
}
