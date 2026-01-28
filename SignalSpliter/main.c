/*************************************************************************
This code is the adapted for OPC server version of Łukasz Przystupa's code
GitHub: https://github.com/Luk9091/SignalSpliter
*************************************************************************/

#include <pico/stdio.h>
#include <pico/stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pico/error.h>
#include <hardware/dma.h>

#include <FreeRTOS.h>
#include <task.h>
#include "FreeRTOSConfig.h"


#include "led.h"
#include "encoder.h"

#include "spi.h"

#include "draw.h"
#include "interface.h"
#include "screensaver.h"

#include "i2c.h"
#include "expander.h"


typedef enum SELECT_STATE{
    SELECT_NONE,
    SELECT_INPUT,
    SELECT_OUTPUT,
} SELECT_STATE_t;

static SELECT_STATE_t state = SELECT_NONE;
static int channel = 0;
static int channel_out0;
static int channel_out1;
static int out = 0;

static Expander_t expander_0;
static Expander_t expander_1;


TaskHandle_t hand_controller_handler;
bool lock;



void blink_task(__unused void *param){
    while (1){
        LED_toggle();
        vTaskDelay(pdTICKS_TO_MS(250));
    }
}


static inline void change_input_channel(int *channel, int rotation){
    INTERFACE_channel_frame(*channel, 0);
    *channel =(*channel + rotation) % 2; 
    INTERFACE_channel_frame(*channel, DISPLAY_BRIGHTNESS);
}


int64_t auto_disable(alarm_id_t id, __unused void *data){

    int tmp;
    if (channel == 0){
        tmp = channel_out0;
    } else {
        tmp = channel_out1;
    }

    INTERFACE_channel_fill_frame(channel, 0);
    INTERFACE_draw_text(channel, DISPLAY_BRIGHTNESS);

    INTERFACE_set_out_number(tmp + 1, channel, DISPLAY_BRIGHTNESS);
    INTERFACE_set_rect(tmp, channel, DISPLAY_BRIGHTNESS);
    INTERFACE_set_rect(out, channel, 0);

    state = SELECT_NONE;
    INTERFACE_update();
    return 0;
}

static inline void change_output_channel(int channel, int *out, int rotation){
    INTERFACE_set_rect(*out, channel, 0);
    *out = (*out + rotation) % 16;

    if (*out < 0){
        *out += 16;
    }

    if (channel == 0){
        if (*out == channel_out1){
            *out = (*out + rotation) % 16;
        }
    } else {
        if (*out == channel_out0){
            *out = (*out + rotation) % 16;
        }
    }

    if (*out < 0){
        *out += 16;
    }
    INTERFACE_set_rect(*out, channel, DISPLAY_BRIGHTNESS);
    INTERFACE_set_smallRect(*out, channel, 0);

    INTERFACE_clear_out_number(channel, DISPLAY_BRIGHTNESS);
    INTERFACE_set_out_number(*out + 1, channel, 0);
}

void update_expanders(int channel_0, int channel_1){
    EXPANDER_put(&expander_0, 0);
    EXPANDER_put(&expander_1, 0);

    uint16_t out0 = 0;
    uint16_t out1 = 0;

    if (channel_0 >= 0){
        if (channel_0 < 8) {
            out0 |= 1 << channel_0*2;
        } else {
            out1 |= 1 << ((channel_0 - 8)*2);
        }
    }

    if (channel_1 >= 0){
        if (channel_1 < 8){
            out0 |= 1 << (channel_1*2 + 1);
        } else {
            out1 |= 1 << ((channel_1 - 8)*2 + 1);
        }
    }

    EXPANDER_put(&expander_0, out0);
    EXPANDER_put(&expander_1, out1);
}


void hand_controller(__unused void *param){

    while(1){
        if (DRAW_isRun()) vTaskDelay(pdTICKS_TO_MS(20));
        int rotation = ENCODER_getValue();
        bool accept = ENCODER_getAccept();
        if (rotation == 0 && accept == false) {
            vTaskDelay(pdTICKS_TO_MS(50));
            continue;
        }

        if(SCREENSAVER_isRun()){
            SCREENSAVER_disable(channel_out0, channel_out1);
        }
        SCREENSAVER_TIMEOUT_reset();


        switch (state){
        case SELECT_NONE:{
            state = SELECT_INPUT;
            change_input_channel(&channel, 0);
        } break;

        case SELECT_INPUT:{
            if (!accept){
                change_input_channel(&channel, rotation);
            } else {
                INTERFACE_channel_fill_frame(channel, DISPLAY_BRIGHTNESS);
                INTERFACE_draw_text(channel, 0);
                if (channel == 0){
                    out = channel_out0;
                } else {
                    out = channel_out1;
                }

                INTERFACE_set_out_number(out+1, channel, 0);
                INTERFACE_set_smallRect(out, channel, 0);
                state = SELECT_OUTPUT;
            }
        }break;

        case SELECT_OUTPUT:{
            if(!accept){
                change_output_channel(channel, &out, rotation);
            } else {
                INTERFACE_set_rect(out, channel, DISPLAY_BRIGHTNESS);
                INTERFACE_channel_fill_frame(channel, 0);
                INTERFACE_draw_text(channel, DISPLAY_BRIGHTNESS);
                INTERFACE_set_out_number(out + 1, channel, DISPLAY_BRIGHTNESS);

                if (channel == 0){
                    channel_out0 = out;
                } else {
                    channel_out1 = out;
                }
                state = SELECT_INPUT;
                update_expanders(channel_out0, channel_out1);
            }
        }break;
        
        default:
            break;
        }

        INTERFACE_update();
        vTaskDelay(pdTICKS_TO_MS(50));
    }
}

int getline(char *input){
    int i = 0;
    while(i < 64){
        char c = getchar_timeout_us(100);
        if (c > 127 || c < 0) {
            vTaskDelay(pdTICKS_TO_MS(5));
            continue;
        }

        if (c == '\x7F' || c == '\x08'){
            if (i > 0){
                putchar('\x08');
                putchar(' ');
                putchar('\x08');
                i--;
                input[i] = 0;
            }
            continue;
        }


        if (c == '\r' || c == '\n'){
            //putchar('\n');
            return i-1;
        }
        input[i] = c;
        i++;
        input[i] = 0;
        //putchar(c);
    }
    return i;
}

#define NEXT_TOKEN() (strtok(NULL, " "))

int strtoi(const char* str, int *ok, int base){
    char *end_ptr;
    int ret = strtol(str, &end_ptr, base);
    if (end_ptr == str) {
        *ok = PICO_ERROR_INVALID_DATA;
        return 0;
    }
    *ok = PICO_ERROR_NONE;
    return ret;
}

void send_status(void) {
    printf("IN1=");
    if (channel_out0 >= 0) printf("%d", channel_out0 + 1);
    else printf("OFF");

    printf(" IN2=");
    if (channel_out1 >= 0) printf("%d", channel_out1 + 1);
    else printf("OFF");

    printf("\n");
}

void serial_controller(__unused void *param){
    putchar('\n');
    char input[66] = {0};
    int input_channel = 0;
    bool update;
    bool isStatus = 0;
    
    while(true){
        // if (input_channel >= 0){
        //     printf("In %i: ", input_channel + 1);
        // }

        getline(input);
        if (!lock){
            lock = true;
            INTERFACE_lock(true);
            vTaskSuspend(hand_controller_handler);
            // TIMEOUT_cancel();
            state = SELECT_NONE;
            INTERFACE_draw(channel_out0, channel_out1);
        }
        
        char *token = strtok(input, " ");
        SCREENSAVER_TIMEOUT_reset();

        while (token){
            update = false;
            if (strncmp(token, "release", 7) == 0){
                update = true;
                lock = false;
                INTERFACE_lock(false);
            }
            else if (strncmp(token, "clear", 5) == 0) {
                INTERFACE_clear();
                INTERFACE_update();
            }
            else if (strncmp(token, "draw", 4) == 0) {
                INTERFACE_draw(-1, -1);
                INTERFACE_update();
            }
            else if (strncmp(token, "screen", 6) == 0) {
                if (!SCREENSAVER_isRun()){
                    SCREENSAVER_enable();
                    
                    printf("Run screensaver\n");
                    lock = false;
                } 
            }
            else if (strncmp(token, "status", 6) == 0) {
                send_status();
                isStatus = 1;
            }
            else if (strncmp(token, "in", 2) == 0){
                update = true;
                token = NEXT_TOKEN();
                int ok;
                int value = strtoi(token, &ok, 10);

                if (ok != PICO_ERROR_NONE){
                    printf("Invalid value\n");
                    putchar('n');
                    break;
                }
                if (value < 1 || value > 2){
                    printf("Invalid range\n");
                    putchar('n');
                    break;
                }

                input_channel = value - 1;
            }
            else if (strncmp(token, "out", 3) == 0){
                update = true;
                token = NEXT_TOKEN();
                int ok;
                int value = strtoi(token, &ok, 10);

                if (ok != PICO_ERROR_NONE){
                    printf("Invalid value\n");
                    putchar('n');
                    break;
                }
                if (value < 1 || value > 16){
                    printf("Invalid range\n");
                    putchar('n');
                    break;
                }
                value -= 1;
                if (input_channel == 0){
                    if (value == channel_out1){
                        printf("Used port\n");
                        putchar('n');
                        break;
                    }
                    INTERFACE_set_rect(channel_out0, input_channel, 0);
                    channel_out0 = value;
                } else if (input_channel == 1){
                    if (value == channel_out0) {
                        printf("Used port\n");
                        putchar('n');
                        break;
                    }
                    INTERFACE_set_rect(channel_out1, input_channel, 0);
                    channel_out1 = value;
                }
                INTERFACE_clear_out_number(input_channel, 0);
                INTERFACE_set_out_number(value+1, input_channel, DISPLAY_BRIGHTNESS);
                INTERFACE_set_rect(value, input_channel, DISPLAY_BRIGHTNESS);
            } else if (strncmp(token, "off", 3) == 0){
                update = true;
                if (input_channel == 0){
                    INTERFACE_set_rect(channel_out0, input_channel, 0);
                    channel_out0 = -1;
                } else if (input_channel == 1){
                    INTERFACE_set_rect(channel_out1, input_channel, 0);
                    channel_out1 = -1;
                }
                INTERFACE_clear_out_number(input_channel, 0);

            }
            else {
                printf("Invalid command\n");
                putchar('n');
                break;
            }

            token = NEXT_TOKEN();
        }
        if(!isStatus)
        {
            printf("OK\n");
        }
        isStatus = 0;
        
        if (update){
            if (SCREENSAVER_isRun()) {
                SCREENSAVER_disable(channel_out0, channel_out1);
                INTERFACE_lock(true);
            }

            update_expanders(channel_out0, channel_out1);
            INTERFACE_update();
        }

        if (!lock){
            vTaskResume(hand_controller_handler);
        }
        // vTaskDelay(pdTICKS_TO_MS(100));
    }
}


void circularOne(__unused void *param){
    int out = -1;
    while (1){
        INTERFACE_set_rect(out, 1, 0);
        out++;
        if (out > 15) out = 0;
        update_expanders(-1, out);

        INTERFACE_clear_out_number(1, 0);
        INTERFACE_set_out_number(out+1, 1, DISPLAY_BRIGHTNESS);
        INTERFACE_set_rect(out, 1, DISPLAY_BRIGHTNESS);
        INTERFACE_update();
        vTaskDelay(pdTICKS_TO_MS(1000));
    }
    
}





int main(){
    channel_out0 = -1;
    channel_out1 = -1;
    lock = false;

    stdio_init_all();
    LED_Init();

    ENCODER_Init();

    SPI_Init();
    INTERFACE_Init();
    INTERFACE_draw(channel_out0, channel_out1);
    INTERFACE_update();

    uart_init(uart1, 9600);
    gpio_set_function(8, GPIO_FUNC_UART);  // TX  GPIO 8
    gpio_set_function(9, GPIO_FUNC_UART);  // RX  GPIO 9
    stdio_uart_init_full(uart1, 9600, 8, 9);
    //stdio_init_all();

    // I2C_Init(I2C_CHANNEL_0, 12, 13);
    // I2C_Init(I2C_CHANNEL_1, 2, 3);
    I2C_Init(I2C_CHANNEL_0, 0, 1);
    I2C_Init(I2C_CHANNEL_1, 14, 15);
    // expander_0 = EXPANDER_init(I2C_CHANNEL_0, 0x20, 11);
    // expander_1 = EXPANDER_init(I2C_CHANNEL_1, 0x20, 4);
    expander_0 = EXPANDER_init(I2C_CHANNEL_0, 0x20, 28);
    expander_1 = EXPANDER_init(I2C_CHANNEL_1, 0x20, 16);
    EXPANDER_set_dir(&expander_0, 0x0000);
    EXPANDER_set_dir(&expander_1, 0x0000);
    EXPANDER_put(&expander_0, 0);
    EXPANDER_put(&expander_1, 0);

    SCREENSAVER_TIMEOUT_run();

    xTaskCreate(blink_task, "Blink task", configMINIMAL_STACK_SIZE, NULL, 3, NULL);
    xTaskCreate(hand_controller, "hand controller", 2048, NULL, 3, &hand_controller_handler);
    xTaskCreate(serial_controller, "serial controller", 2048, NULL, 3, NULL);
    // xTaskCreate(circularOne, "serial controller", 2048, NULL, 3, NULL);

    // vTaskSuspend(screensaver_hanlder);
    vTaskStartScheduler();

    while (1){}

    return 0;
}