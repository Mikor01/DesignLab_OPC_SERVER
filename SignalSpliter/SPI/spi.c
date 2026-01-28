#include "spi.h"
#include <hardware/gpio.h>


void SPI_Init(){
    spi_init(SPI_CHANNEL, SPI_SPEED);
    spi_set_format(SPI_CHANNEL, 8, 1, 1, SPI_MSB_FIRST);

    gpio_set_function(SPI_TXD_GPIO, GPIO_FUNC_SPI);
    gpio_set_function(SPI_SCK_GPIO, GPIO_FUNC_SPI);

}


int SPI_write(uint8_t *data, size_t size, uint8_t chipSelect_GPIO){
    SPI_select(chipSelect_GPIO);
    int status = spi_write_blocking(SPI_CHANNEL, data, size);
    SPI_deselect(chipSelect_GPIO);

    return status;
}

int SPI_write_holdLine(uint8_t *data, size_t size, uint8_t chipSelect_GPIO){
    int status;
    SPI_select(chipSelect_GPIO);
    status += spi_write_blocking(SPI_CHANNEL, data, size);

    return status;
}
