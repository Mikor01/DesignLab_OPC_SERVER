#ifndef __ENCODER_H__
#define __ENCODER_H__

#define ENCODER_A	    18
#define ENCODER_B	    17
#define ENCODER_SWITCH	19

#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/gpio.h>
#include <stdlib.h>

// extern uint8_t left_rotation;
// extern uint8_t right_rotation;
// extern uint8_t digit_to_change;
// extern uint8_t new_digit;

void ENCODER_Init();

void ENCODER_enable_interrupts();
void ENCODER_disable_interrupts();

int ENCODER_getValue();
bool ENCODER_getAccept();

#endif