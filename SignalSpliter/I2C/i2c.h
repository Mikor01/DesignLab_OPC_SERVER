#ifndef __I2C_H__
#define __I2C_H__

#include <pico/stdlib.h>
#include <hardware/i2c.h>

#define I2C_FREQUENCY (400*1000)
#define I2C_CHANNEL_0 i2c0
#define I2C_CHANNEL_1 i2c1


void I2C_Init(i2c_inst_t *channel, uint SDA_GPIO, uint SCL_GPIO);

int I2C_writeRegister(i2c_inst_t *channel, uint8_t device, uint8_t address, uint8_t data);
int I2C_readRegister(i2c_inst_t *channel, uint8_t device, uint8_t address, uint8_t *data);

int I2C_writeNRegister(i2c_inst_t *channel, uint8_t device, uint8_t address, uint8_t *data, uint size);
int I2C_readNRegister(i2c_inst_t *channel, uint8_t device, uint8_t address, uint8_t *data, uint size);

#endif