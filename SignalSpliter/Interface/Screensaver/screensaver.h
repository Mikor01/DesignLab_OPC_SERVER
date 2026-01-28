#ifndef __SCREENSAVER_H__
#define __SCREENSAVER_H__

#include <pico/stdlib.h>
#include <FreeRTOS.h>
#include <task.h>
#include "FreeRTOSConfig.h"

#include "interface.h"

// Timeout in ms
#define SCREENSAVER_TIMEOUT     (60*1000)
extern TaskHandle_t hand_controller_handler;
extern bool lock;

void SCREENSAVER_TIMEOUT_run();
void SCREENSAVER_TIMEOUT_reset();

void SCREENSAVER_enable();
void SCREENSAVER_disable(int channel_out0, int channel_out1);
bool SCREENSAVER_isRun();


#endif