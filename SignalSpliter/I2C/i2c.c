#include "i2c.h"

#include <pico/stdio.h>
#include <hardware/gpio.h>

#define TIMEOUT_US (20*4)

void I2C_Init(i2c_inst_t *channel, uint SDA, uint SCL){
    i2c_init(channel, I2C_FREQUENCY);

    gpio_set_function(SDA, GPIO_FUNC_I2C);
    gpio_set_function(SCL, GPIO_FUNC_I2C);

    gpio_pull_up(SDA);
    gpio_pull_up(SCL);
}


int I2C_writeRegister(i2c_inst_t *channel, uint8_t device, uint8_t address, uint8_t data){
    uint8_t table[2] = {address, data};
    return i2c_write_timeout_per_char_us(channel, device, table, 2, false, TIMEOUT_US);
}

int I2C_writeNRegister(i2c_inst_t *channel, uint8_t device, uint8_t address, uint8_t *data, uint size){
    int status = i2c_write_timeout_per_char_us(channel, device, &address, 1, true, TIMEOUT_US);
    if (status < 0){
        return status;
    }

    status = i2c_write_timeout_per_char_us(channel, device, data, size, false, TIMEOUT_US);
    return status;
}




int I2C_readRegister(i2c_inst_t *channel, uint8_t device, uint8_t address, uint8_t *data){

    int status = i2c_write_timeout_per_char_us(channel, device, &address, 1, true, TIMEOUT_US);
    if (status < 0) {
        return status;
    }

    status = i2c_read_timeout_per_char_us(channel, device, data, 1, false, TIMEOUT_US);
    return status;
}

int I2C_readNRegister(i2c_inst_t *channel, uint8_t device, uint8_t address, uint8_t *data, uint size){

    int status = i2c_write_timeout_per_char_us(channel, device, &address, 1, true, TIMEOUT_US);
    if (status < 0) {
        return status;
    }

    status = i2c_read_timeout_per_char_us(channel, device, data, size, false, TIMEOUT_US);
    return status;
}