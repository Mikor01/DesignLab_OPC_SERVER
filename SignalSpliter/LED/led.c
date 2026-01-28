#include "led.h"
#include <hardware/gpio.h>


#if !PICO_CYW43_ARCH_THREADSAFE_BACKGROUND
#define GPIO_INBUILD_LED    25

void LED_Init(){
    gpio_init(GPIO_INBUILD_LED);
    gpio_set_dir(GPIO_INBUILD_LED, GPIO_OUT);
}

void LED_on(){
    gpio_put(GPIO_INBUILD_LED, 1);
}

void LED_off(){
    gpio_put(GPIO_INBUILD_LED, 1);
}

void LED_toggle(){
    gpio_put(GPIO_INBUILD_LED, !gpio_get(GPIO_INBUILD_LED));
}

#else
#include <pico/cyw43_arch.h>

void LED_Init(){
    cyw43_arch_init();
}

void LED_on(){
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
}

void LED_off(){
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
}

void LED_toggle(){
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, !cyw43_arch_gpio_get(CYW43_WL_GPIO_LED_PIN));
}

#endif