#include "expander.h"
#include "expander_reg_map.h"

#include <hardware/gpio.h>

static inline int writeReg8(Expander_t *expander, uint8_t address, uint8_t data){
    return I2C_writeRegister(expander->channel, expander->address, address, data);
}

static inline int readReg8(Expander_t *expander, uint8_t address, uint8_t *data){
    return I2C_readRegister(expander->channel, expander->address, address, data);
}

static inline int writeReg16(Expander_t *expander, uint8_t address, uint16_t data){
    return I2C_writeNRegister(expander->channel, expander->address, address, (uint8_t*)(&data), 2);
}

static inline int readReg16(Expander_t *expander, uint8_t address, uint16_t *data){
    uint8_t read[2];
    int status = I2C_readNRegister(expander->channel, expander->address, address, read, 2);

    *data = (read[0] << 8) | (read[1] << 0);
    return status;
}





Expander_t EXPANDER_init(i2c_inst_t *I2C_channel, uint8_t device_address, uint8_t reset_gpio){
    Expander_t expander = {
        .channel = I2C_channel,
        .address = device_address,
        .reset_gpio = reset_gpio,
    };

    gpio_init(reset_gpio);
    gpio_set_dir(reset_gpio, GPIO_OUT);
    gpio_put(reset_gpio, 1);

    EXPANDER_config(&expander);

    return expander;
}


int EXPANDER_config(Expander_t *expander){
    EXPANDER_reset(expander);
    uint8_t config = 0;
#if !(EXPANDER_SEQUENTIAL_BANK)
    config |= 1 << 7;
#else
    config |= 0 << 7;
#endif

    config |= (EXPANDER_IRQ_GPIO_OD) << 2;
    config |= (EXPANDER_IRQ_POLARITY) << 1;

    return I2C_writeRegister(expander->channel, expander->address, 0x0A, config);
}


void EXPANDER_reset(Expander_t *expander){
    gpio_put(expander->reset_gpio, 0);
    sleep_ms(1);
    gpio_put(expander->reset_gpio, 1);
    sleep_ms(1);
}



int EXPANDER_put(Expander_t *expander, uint16_t out){
#if EXPANDER_SEQUENTIAL_BANK
    return writeReg16(expander, GPIO, out);
#else
    EXPANDER_put_A(expander, out >> 0);
    EXPANDER_put_B(expander, out >> 8);
#endif
}

int EXPANDER_set_dir(Expander_t *expander, uint16_t dir){
#if EXPANDER_SEQUENTIAL_BANK
    return writeReg16(expander, IODIR, dir);
#else
    EXPANDER_set_dir_A(expander, dir >> 0);
    EXPANDER_set_dir_B(expander, dir >> 8);
#endif
}



int EXPANDER_put_A(Expander_t *expander, uint8_t out){
    writeReg8(expander, GPIO_A, out);
}

int EXPANDER_put_B(Expander_t *expander, uint8_t out){
    writeReg8(expander, GPIO_B, out);
}

int EXPANDER_set_dir_A(Expander_t *expander, uint8_t dir){
    writeReg8(expander, IODIR_A, dir);
}

int EXPANDER_set_dir_B(Expander_t *expander, uint8_t dir){
    writeReg8(expander, IODIR_B, dir);
}