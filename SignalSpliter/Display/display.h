#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <pico/stdio.h>
#include <pico/stdlib.h>

#include "spi.h"
#include "draw_dma.h"

#define DISPLAY_DC_GPIO     6
#define DISPLAY_CS_GPIO     5
#define DISPLAY_RS_GPIO     7

typedef enum DISPLAY_MODE{
    DISPLAY_MODE_ALL_OFF    = 0,
    DISPLAY_MODE_ALL_ON     = 1,
    DISPLAY_MODE_NORMAL     = 2,
    DISPLAY_MODE_INVERTED   = 3,
} DISPLAY_MODE_t;


static inline void command_mode(){
    gpio_put(DISPLAY_DC_GPIO, 0);
}

static inline void data_mode(){
    gpio_put(DISPLAY_DC_GPIO, 1);
}

static inline void write_command(uint8_t command){
    command_mode();
    SPI_write(&command, 1, DISPLAY_CS_GPIO);
}

static inline void write_data(uint8_t data){
    data_mode();
    SPI_write(&data, 1, DISPLAY_CS_GPIO);
}

static inline void write_commandWithParam(uint8_t command, uint8_t param){
    write_command(command);
    write_data(param);
}

static inline void write_commandWith2Param(uint8_t command, uint8_t param, uint8_t param2){
    write_command(command);
    write_data(param);
    write_data(param2);
}




void DISPLAY_Init();
void DISPLAY_reset();

void DISPLAY_setWindow(uint8_t start_column, uint8_t end_column, uint8_t start_row, uint8_t end_row);
void DISPLAY_setContrast(uint8_t contrast);
void DISPLAY_setMode(DISPLAY_MODE_t mode);

void DISPLAY_sendBuffer(uint8_t *buffer, uint32_t buffer_size);





#endif