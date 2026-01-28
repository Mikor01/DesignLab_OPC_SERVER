#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#include <pico/stdio.h>
#include "draw.h"

#define INTERFACE_DRAW_GRID             false
#define INTERFACE_UPDATE_VIA_DMA        true

#define DISPLAY_WIDTH                   (256)
#define DISPLAY_HEIGHT                  (64)
#define DISPLAY_BRIGHTNESS              (15)

#define INTERFACE_CHANNEL_BORDER_SIZE   (16)
#define INTERFACE_CHANNEL_RECT_SIZE     (13)
#define INTERFACE_CHANNEL_SMALLRECT_SIZE (7)

#define INTERFACE_GRID_1_Y_TOP          (DISPLAY_HEIGHT-2*INTERFACE_CHANNEL_BORDER_SIZE)
#define INTERFACE_GRID_1_Y_BOTTOM       (INTERFACE_GRID_1_Y_TOP + INTERFACE_CHANNEL_BORDER_SIZE - 1)
#define INTERFACE_GRID_2_Y_TOP          (DISPLAY_HEIGHT-INTERFACE_CHANNEL_BORDER_SIZE)
#define INTERFACE_GRID_2_Y_BOTTOM       (INTERFACE_GRID_2_Y_TOP + INTERFACE_CHANNEL_BORDER_SIZE - 1)

#define INTERFACE_FONT_SIZE             (12)
#define INTERFACE_FONT_WIDTH            (10)
#define INTERFACE_TEXT_BORDER_PADDING   (3)
#define INTERFACE_TEXT_1_X              (DISPLAY_WIDTH * 1/16)
#define INTERFACE_TEXT_1_Y              (DISPLAY_HEIGHT/4 + INTERFACE_FONT_SIZE/4)

#define INTERFACE_TEXT_2_X              (DISPLAY_WIDTH * 9/16)
#define INTERFACE_TEXT_2_Y              (INTERFACE_TEXT_1_Y)

#define INTERFACE_OUT_NUM_1_X           (INTERFACE_TEXT_1_X + INTERFACE_FONT_WIDTH*7)
#define INTERFACE_OUT_NUM_1_Y           (INTERFACE_TEXT_1_Y)

#define INTERFACE_OUT_NUM_2_X           (INTERFACE_TEXT_2_X + INTERFACE_FONT_WIDTH*7)
#define INTERFACE_OUT_NUM_2_Y           (INTERFACE_TEXT_2_Y)

#define INTERFACE_LOCK_X                (INTERFACE_CHANNEL_BORDER_SIZE*7.5)
#define INTERFACE_LOCK_Y                (DISPLAY_HEIGHT/10)
#define INTERFACE_LOCK_WITH             (16)
#define INTERFACE_LOCK_HEIGHT           (16)

#define INTERFACE_SCREENSAVER_WIDTH     (32)
#define INTERFACE_SCREENSAVER_HEIGHT    (52)


void INTERFACE_Init();
void INTERFACE_update();

void INTERFACE_clear();

void INTERFACE_draw(int channel_0, int channel_1);
void INTERFACE_clear_out_number(bool channel, uint8_t brightness);
void INTERFACE_set_out_number(int number, bool channel, uint8_t brightness);

void INTERFACE_set_rect(int number, bool channel, uint8_t brightness);
void INTERFACE_set_smallRect(int number, bool channel, uint8_t brightness);

void INTERFACE_channel_frame(bool channel, uint8_t brightness);
void INTERFACE_channel_fill_frame(bool channel, uint8_t brightness);

void INTERFACE_draw_text(bool channel, uint8_t brightness);

void INTERFACE_lock(bool lock);


void INTERFACE_clear_screensaver(uint8_t x, uint8_t y);
void INTERFACE_draw_screensaver(uint8_t x, uint8_t y);

#endif