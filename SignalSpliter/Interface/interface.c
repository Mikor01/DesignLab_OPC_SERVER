#include "interface.h"
#include "Font/DejaVuSansMono8pt7b.h"

#include <stdlib.h>
#include <stdio.h>
#include <pico/stdlib.h>
#include <pico/stdio.h>


#include "display.h"
#include "draw_dma.h"

static uint8_t tx_buf[256 * 64 / 2];
#include "logo_agh.h"
static uint8_t lockIcon[128] = {
0x00,0x00,0x0F,0xFF,0xFF,0x00,0x00,0x00,
0x00,0x00,0xFF,0xFF,0xFF,0xF0,0x00,0x00,
0x00,0x0F,0xFF,0x00,0x0F,0xFF,0x00,0x00,
0x00,0xFF,0xF0,0x00,0x00,0xFF,0xF0,0x00,
0x00,0xFF,0xF0,0x00,0x00,0xFF,0xF0,0x00,
0x00,0xFF,0xF0,0x00,0x00,0xFF,0xF0,0x00,
0x00,0xFF,0xF0,0x00,0x00,0xFF,0xF0,0x00,
0x0F,0xFF,0xFF,0x00,0x0F,0xFF,0xFF,0x00,
0x0F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,
0x0F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,
0x0F,0xFF,0xFF,0xF0,0xFF,0xFF,0xFF,0x00,
0x0F,0xFF,0xFF,0xF0,0xFF,0xFF,0xFF,0x00,
0x0F,0xFF,0xFF,0xF0,0xFF,0xFF,0xFF,0x00,
0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xF0,0x00,
0x00,0x0F,0xFF,0xFF,0xFF,0xFF,0x00,0x00,
0x00,0x00,0xFF,0xFF,0xFF,0xF0,0x00,0x00,
};
// static uint font_char_width;

void INTERFACE_Init(){
    DISPLAY_Init();
    set_buffer_size(256, 64);
    select_font(&DejaVuSansMono8pt7b);

#if INTERFACE_UPDATE_VIA_DMA
    DRAW_DMA_Init(tx_buf, 8192);
#endif
}

void INTERFACE_clear(){
    draw_rect_filled(tx_buf, 0, 0, DISPLAY_WIDTH-1, DISPLAY_HEIGHT-1, 0);
}

void INTERFACE_update(){
#if INTERFACE_UPDATE_VIA_DMA
    send_buffer_to_OLED_via_dma(tx_buf);
#else
    send_buffer_to_OLED(tx_buf, 0, 0);
#endif
}

#if INTERFACE_DRAW_GRID
static inline void draw_channel_grid(){
    for (uint i = 0; i < 16; i++){
        draw_rect(tx_buf, i*INTERFACE_CHANNEL_BORDER_SIZE, INTERFACE_GRID_1_Y_TOP, (i+1) * INTERFACE_CHANNEL_BORDER_SIZE-1, INTERFACE_GRID_1_Y_BOTTOM, DISPLAY_BRIGHTNESS);
        draw_rect(tx_buf, i*INTERFACE_CHANNEL_BORDER_SIZE, INTERFACE_GRID_2_Y_TOP, (i+1) * INTERFACE_CHANNEL_BORDER_SIZE-1, INTERFACE_GRID_2_Y_BOTTOM, DISPLAY_BRIGHTNESS);
    }
}
#endif

void INTERFACE_draw(int channel_0, int channel_1){
#if INTERFACE_DRAW_GRID
    draw_channel_grid();
#endif
    INTERFACE_channel_fill_frame(0, 0);
    INTERFACE_channel_fill_frame(1, 0);
    INTERFACE_draw_text(0, DISPLAY_BRIGHTNESS);
    INTERFACE_draw_text(1, DISPLAY_BRIGHTNESS);

    if (channel_0 >= 0)
        INTERFACE_set_out_number(channel_0+1, 0, DISPLAY_BRIGHTNESS);
    if (channel_1 >= 0)
        INTERFACE_set_out_number(channel_1+1, 1, DISPLAY_BRIGHTNESS);

    for (uint i = 0; i < 16; i++){
        INTERFACE_set_rect(i, 0, 0);
        INTERFACE_set_rect(i, 1, 0);
    }

    INTERFACE_set_rect(channel_0, 0, DISPLAY_BRIGHTNESS);
    INTERFACE_set_rect(channel_1, 1, DISPLAY_BRIGHTNESS);
}

 
void INTERFACE_clear_out_number(bool channel, uint8_t brightness){
    if (channel == 0){
        draw_rect_filled(tx_buf, INTERFACE_OUT_NUM_1_X, INTERFACE_OUT_NUM_1_Y- INTERFACE_FONT_SIZE, INTERFACE_OUT_NUM_1_X+INTERFACE_FONT_WIDTH*2, INTERFACE_OUT_NUM_1_Y, brightness);
    } else {
        draw_rect_filled(tx_buf, INTERFACE_OUT_NUM_2_X, INTERFACE_OUT_NUM_2_Y- INTERFACE_FONT_SIZE, INTERFACE_OUT_NUM_2_X+INTERFACE_FONT_WIDTH*2, INTERFACE_OUT_NUM_2_Y, brightness);
    }
}

void INTERFACE_set_out_number(int number, bool channel, uint8_t brightness){
    char str[2];
    sprintf(str, "%i", number);

    if (channel == 0){
        draw_text(tx_buf, str, INTERFACE_OUT_NUM_1_X, INTERFACE_OUT_NUM_1_Y, brightness);
    } else {
        draw_text(tx_buf, str, INTERFACE_OUT_NUM_2_X, INTERFACE_OUT_NUM_2_Y, brightness);
    }
}


void INTERFACE_set_rect(int number, bool channel, uint8_t brightness){
    int x = 1 + number*INTERFACE_CHANNEL_BORDER_SIZE;
    int y = INTERFACE_GRID_1_Y_TOP + 1 + channel*INTERFACE_CHANNEL_BORDER_SIZE;

    draw_rect_filled(tx_buf, x, y, x + INTERFACE_CHANNEL_RECT_SIZE, y+INTERFACE_CHANNEL_RECT_SIZE, brightness);
}

void INTERFACE_set_smallRect(int number, bool channel, uint8_t brightness){
    int x = 1 + number*INTERFACE_CHANNEL_BORDER_SIZE + (INTERFACE_CHANNEL_RECT_SIZE - INTERFACE_CHANNEL_SMALLRECT_SIZE)/2;
    int y = INTERFACE_GRID_1_Y_TOP + 1 + channel*INTERFACE_CHANNEL_BORDER_SIZE + (INTERFACE_CHANNEL_RECT_SIZE - INTERFACE_CHANNEL_SMALLRECT_SIZE)/2;

    draw_rect_filled(tx_buf, x, y, x + INTERFACE_CHANNEL_SMALLRECT_SIZE, y+INTERFACE_CHANNEL_SMALLRECT_SIZE, brightness);
}

void INTERFACE_channel_frame(bool channel, uint8_t brightness){
    if (channel == 0){
        draw_rect(tx_buf, 
            INTERFACE_TEXT_1_X - INTERFACE_TEXT_BORDER_PADDING,
            INTERFACE_TEXT_1_Y - INTERFACE_FONT_SIZE - INTERFACE_TEXT_BORDER_PADDING, 
            INTERFACE_TEXT_1_X + INTERFACE_TEXT_BORDER_PADDING + 9 * INTERFACE_FONT_WIDTH,
            INTERFACE_TEXT_1_Y + INTERFACE_TEXT_BORDER_PADDING + 1,
            brightness
        );
    } else {
        draw_rect(tx_buf, 
            INTERFACE_TEXT_2_X - INTERFACE_TEXT_BORDER_PADDING,
            INTERFACE_TEXT_2_Y - INTERFACE_FONT_SIZE - INTERFACE_TEXT_BORDER_PADDING, 
            INTERFACE_TEXT_2_X + INTERFACE_TEXT_BORDER_PADDING + 9 * INTERFACE_FONT_WIDTH,
            INTERFACE_TEXT_2_Y + INTERFACE_TEXT_BORDER_PADDING + 1,
            brightness
        );
    }
}

void INTERFACE_channel_fill_frame(bool channel, uint8_t brightness){
    if (channel == 0){
        draw_rect_filled(tx_buf, 
            INTERFACE_TEXT_1_X - INTERFACE_TEXT_BORDER_PADDING,
            INTERFACE_TEXT_1_Y - INTERFACE_FONT_SIZE - INTERFACE_TEXT_BORDER_PADDING, 
            INTERFACE_TEXT_1_X + INTERFACE_TEXT_BORDER_PADDING + 9 * INTERFACE_FONT_WIDTH,
            INTERFACE_TEXT_1_Y + INTERFACE_TEXT_BORDER_PADDING + 1,
            brightness
        );
    } else {
        draw_rect_filled(tx_buf, 
            INTERFACE_TEXT_2_X - INTERFACE_TEXT_BORDER_PADDING,
            INTERFACE_TEXT_2_Y - INTERFACE_FONT_SIZE - INTERFACE_TEXT_BORDER_PADDING, 
            INTERFACE_TEXT_2_X + INTERFACE_TEXT_BORDER_PADDING + 9 * INTERFACE_FONT_WIDTH,
            INTERFACE_TEXT_2_Y + INTERFACE_TEXT_BORDER_PADDING + 1,
            brightness
        );
    }
}


void INTERFACE_draw_text(bool channel, uint8_t brightness){
    if (channel == 0){
        draw_text(tx_buf, "IN1-OUT", INTERFACE_TEXT_1_X, INTERFACE_TEXT_1_Y, brightness);
    } else {
        draw_text(tx_buf, "IN2-OUT", INTERFACE_TEXT_2_X, INTERFACE_TEXT_2_Y, brightness);
    }
}


void INTERFACE_lock(bool lock){
    if (!lock){
        draw_rect_filled(tx_buf, INTERFACE_LOCK_X, INTERFACE_LOCK_Y, INTERFACE_LOCK_X+INTERFACE_LOCK_WITH, INTERFACE_LOCK_Y+INTERFACE_LOCK_HEIGHT, 0);
    } else {
        draw_bitmap_4bpp(tx_buf, lockIcon, INTERFACE_LOCK_X, INTERFACE_LOCK_Y, INTERFACE_LOCK_WITH, INTERFACE_LOCK_HEIGHT);
    }
}


void INTERFACE_clear_screensaver(uint8_t x, uint8_t y){
    draw_rect_filled(tx_buf, x, y, x+INTERFACE_SCREENSAVER_WIDTH, y+INTERFACE_SCREENSAVER_HEIGHT, 0);
}

void INTERFACE_draw_screensaver(uint8_t x, uint8_t y){
    draw_bitmap_4bpp(tx_buf, logo_agh, x, y, INTERFACE_SCREENSAVER_WIDTH, INTERFACE_SCREENSAVER_HEIGHT);
}