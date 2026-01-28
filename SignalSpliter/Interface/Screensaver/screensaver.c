#include "screensaver.h"

static TaskHandle_t screensaver_hanlder;

static alarm_id_t screensaver_alarm_id;
static int64_t enable_callback(alarm_id_t id, __unused void *param){
    if (!SCREENSAVER_isRun()){
        SCREENSAVER_enable();
    }
    return 0;
}


void SCREENSAVER_TIMEOUT_run(){
    screensaver_alarm_id = add_alarm_in_ms(SCREENSAVER_TIMEOUT, enable_callback, NULL, false);
}

void SCREENSAVER_TIMEOUT_reset(){
    if (screensaver_alarm_id > 0){
        cancel_alarm(screensaver_alarm_id);
    }
    SCREENSAVER_TIMEOUT_run();
}



void screensaver(__unused void *param){
    int step = 16;
    int delay = 500;
    int repeat = 5;

    int x = 0;
    int y = 0;
    int speed_x = 4;
    int speed_y = 4;

    INTERFACE_clear();
    while (1){
        INTERFACE_clear_screensaver(x, y);
        x = x + speed_x;
        y = y + speed_y;

        if (
            x + speed_x > DISPLAY_WIDTH - INTERFACE_SCREENSAVER_WIDTH ||
            x + speed_x < 0
        ) {
            speed_x = -speed_x;
        }

        if (
            y + speed_y > DISPLAY_HEIGHT - INTERFACE_SCREENSAVER_HEIGHT ||
            y + speed_y < 0
        ) {
            speed_y = -speed_y;
        }

        INTERFACE_draw_screensaver(x, y);
        INTERFACE_update();
        vTaskDelay(pdTICKS_TO_MS(delay));
        while (DRAW_isRun()){
            vTaskDelay(pdTICKS_TO_MS(delay/100));
        }
    }
}

void SCREENSAVER_enable(){
    xTaskCreate(screensaver, "screen saver", 2048, NULL, 3, &screensaver_hanlder);

    if (lock){
        vTaskResume(hand_controller_handler);
        lock = false;
    }
}



void SCREENSAVER_disable(int channel_out0, int channel_out1){
    vTaskDelete(screensaver_hanlder);
    screensaver_hanlder = NULL;

    INTERFACE_clear();
    INTERFACE_draw(channel_out0, channel_out1);
}


bool SCREENSAVER_isRun(){
    return screensaver_hanlder != NULL;
}