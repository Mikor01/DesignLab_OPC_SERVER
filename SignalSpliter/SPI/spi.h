#ifndef __SPI_H__
#define __SPI_H__

#include <hardware/clocks.h>
#include <hardware/spi.h>
#include <hardware/gpio.h>

// #define SPI_CHANNEL  spi1
// #define SPI_TXD_GPIO 15
// #define SPI_SCK_GPIO 14
#define SPI_CHANNEL  spi0
#define SPI_TXD_GPIO 2
#define SPI_SCK_GPIO 3
#define SPI_SPEED    (1 * MHZ)


static inline void SPI_select(uint8_t chipSelect_GPIO){
    gpio_put(chipSelect_GPIO, 0);
}

static inline void SPI_deselect(uint8_t chipSelect_GPIO){
    gpio_put(chipSelect_GPIO, 1);
}



void SPI_Init();
int SPI_write(uint8_t *data, size_t size, uint8_t chipSelect_GPIO);
int SPI_write_holdLine(uint8_t *data, size_t size, uint8_t chipSelect_GPIO);




#endif