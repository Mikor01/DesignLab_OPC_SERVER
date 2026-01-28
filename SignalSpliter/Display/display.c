#include "display.h"
#include "comman.h"

#include <hardware/gpio.h>



void display_GPIOInit(){
    gpio_init(DISPLAY_DC_GPIO);
    gpio_init(DISPLAY_CS_GPIO);
    gpio_init(DISPLAY_RS_GPIO);

    gpio_set_dir(DISPLAY_DC_GPIO, GPIO_OUT);
    gpio_set_dir(DISPLAY_CS_GPIO, GPIO_OUT);
    gpio_set_dir(DISPLAY_RS_GPIO, GPIO_OUT);


    gpio_put(DISPLAY_CS_GPIO, 1);
    gpio_put(DISPLAY_RS_GPIO, 1);
}

static void set_default(){
    // Copied from: https://github.com/olikraus/u8g2/blob/master/csrc/u8x8_d_ssd1322.c
    write_commandWithParam(0xFD, 0x12);            	    /* unlock */
    write_command(0xAE);		                        /* display off */
    write_commandWithParam(0xB3, 0x91);			        /* set display clock divide ratio/oscillator frequency (set clock as 80 frames/sec)  */  
    write_commandWithParam(0xCA, 0x3F);			        /* multiplex ratio 1/64 Duty (0x0F~0x3F) */  
    write_commandWithParam(0xA2, 0x00);			        /* display offset, shift mapping ram counter */  
    write_commandWithParam(0xA1, 0x00);			        /* display start line */  
    // write_commandWith2Param(0xA0, 0x16, 0x011);         //? Set Re-Map / Dual COM Line Mode
    write_commandWith2Param(0xA0, 0x14, 0x011);	        //! Set Re-Map / Dual COM Line Mode
                                                        // ? 
    write_commandWithParam(0xB5, 0x00);                 //! Disable IO input
    write_commandWithParam(0xAB, 0x01);			        /* Enable Internal VDD Regulator */  
    // write_commandWith2Param(0xB4, 0xA0, 0x05|0x0fd);	//? Display Enhancement A
    write_commandWith2Param(0xB4, 0xA0, 0xFD);	        //! Display Enhancement A
    write_commandWithParam(0xC1, 0x9f);			        //? contrast
    // write_commandWithParam(0xC1, 0xFF);			        //! contrast
    write_commandWithParam(0xC7, 0x0F);			        /* Set Scale Factor of Segment Output Current Control */  
    write_command(0xB9);		                        /* linear grayscale */
    write_commandWithParam(0xB1, 0xE2);			        /* Phase 1 (Reset) & Phase 2 (Pre-Charge) Period Adjustment */  
    // write_commandWith2Param(0xD1, 0x082|0x020, 0x020);	//? Display Enhancement B
    write_commandWith2Param(0xD1, 0x082, 0x020);	    //! Display Enhancement B
    write_commandWithParam(0xBB, 0x1F);			        /* precharge  voltage */  
    write_commandWithParam(0xB6, 0x08);			        /* precharge  period */  
    write_commandWithParam(0xBE, 0x07);			        /* vcomh */  
    write_command(0xA6);		                        /* normal display */
    write_command(0xA9);	                            /* exit partial display */

    // write_commandWith2Param(0x75, 0, 63);               // Set row from 0 to 63
    sleep_ms(10);
    write_command(0xAF);		                        /* display on */

    sleep_ms(50);
}

void DISPLAY_setWindow(uint8_t start_column, uint8_t end_column, uint8_t start_row, uint8_t end_row){
    write_commandWith2Param(SET_COLUMN_ADDRESS, start_column + 28, end_column + 28);
    write_commandWith2Param(SET_ROW_ADDRESS, start_row, end_row);
}

void DISPLAY_setContrast(uint8_t contrast){
    write_commandWithParam(SET_CONTRAST_CURRENT, contrast);
}

void DISPLAY_setMode(DISPLAY_MODE_t mode){
    uint8_t setMode = SET_DISPLAY_ALL_OFF + mode;
    write_command(setMode);
}



void DISPLAY_Init(){
    display_GPIOInit();

    DISPLAY_reset();
    set_default();

    // DISPLAY_setMode(DISPLAY_MODE_ALL_OFF);

    // DISPLAY_setWindow(0, 255, 0, 63);
    // display_
}

void DISPLAY_reset(){
    gpio_put(DISPLAY_RS_GPIO, 0);
    sleep_ms(5);
    gpio_put(DISPLAY_RS_GPIO, 1);
    sleep_ms(50);
}


void DISPLAY_sendBuffer(uint8_t *buffer, uint32_t buffer_size){
    write_command(WRITE_RAM_COMMAND);
    data_mode();
    SPI_write(buffer, buffer_size, DISPLAY_CS_GPIO);
}
