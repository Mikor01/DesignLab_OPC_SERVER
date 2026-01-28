#include "encoder.h"

#define RIGHT_LEFT (-1)
#define DEBOUNCE_TIME (120)


static volatile int rotation;
static volatile bool button;

static void detect_first_edge(uint gpio){
    uint state = gpio_get_all();
}

int64_t ENCODER_debouncer_timer(alarm_id_t id, __unused void *data){
    ENCODER_enable_interrupts();
    return 0;
}

void ENCODER_callback(uint gpio, uint32_t events){
    static bool A_fall = false, B_fall = false;
    uint8_t status = (gpio_get(ENCODER_A) << 1) | gpio_get(ENCODER_B);

    switch (gpio)
    {
    case ENCODER_A:{
        if (!B_fall && status == 0){
            A_fall = true;
        } else if (status == 0b01) {
            // printf("B");
            B_fall = false;
            rotation = RIGHT_LEFT;
            ENCODER_disable_interrupts();
            add_alarm_in_ms(DEBOUNCE_TIME, ENCODER_debouncer_timer, NULL, false);
        }
    } break;
    case ENCODER_B:{
        if (!A_fall && status == 0) {
            B_fall = true;
        } else if(status == 0b10) {
            // printf("A");
            A_fall = false;
            rotation = -RIGHT_LEFT;
            ENCODER_disable_interrupts();
            add_alarm_in_ms(DEBOUNCE_TIME, ENCODER_debouncer_timer, NULL, false);
        }
    }break;
    case ENCODER_SWITCH:{
        // printf("Push\n");
        button = true;
    }break;
    
    default:
        break;
    }
}

void ENCODER_enable_interrupts(){
    gpio_set_irq_enabled_with_callback(ENCODER_SWITCH, GPIO_IRQ_EDGE_FALL, true, &ENCODER_callback);
    gpio_set_irq_enabled(ENCODER_A, GPIO_IRQ_EDGE_FALL, true);
	gpio_set_irq_enabled(ENCODER_B, GPIO_IRQ_EDGE_FALL, true);
}

void ENCODER_disable_interrupts(){
    gpio_set_irq_enabled(ENCODER_A, GPIO_IRQ_EDGE_FALL, false);
	gpio_set_irq_enabled(ENCODER_B, GPIO_IRQ_EDGE_FALL, false);
    gpio_set_irq_enabled(ENCODER_SWITCH, GPIO_IRQ_EDGE_FALL, false);
}

void ENCODER_Init(){
    uint32_t mask = 1 << ENCODER_A | 1 << ENCODER_B | 1 << ENCODER_SWITCH;
    gpio_init_mask(mask);
    gpio_set_dir_in_masked(mask);

    gpio_pull_up(ENCODER_A);
    gpio_pull_up(ENCODER_B);
    gpio_pull_up(ENCODER_SWITCH);

    rotation = 0;
    button = false;
    ENCODER_enable_interrupts();
}

int ENCODER_getValue(){
    int status = rotation;
    rotation = 0;
    return status;
}

bool ENCODER_getAccept(){
    bool status = button;
    button = false;
    return status;
}

