#ifndef __DRAW_DMA_H__
#define __DRAW_DMA_H__

#include <pico/stdio.h>
#include <pico/stdlib.h>

void DRAW_DMA_Init(uint8_t *tx_buffer, uint size);
bool send_buffer_to_OLED_via_dma(uint8_t *buffer);



#endif