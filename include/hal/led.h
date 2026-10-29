#ifndef HAL_LED_H
#define HAL_LED_H

#include <Arduino.h>
#include "config.h"

enum LedColor {
    LED_OFF = 0,
    LED_RED,
    LED_GREEN,
    LED_BLUE,
    LED_CYAN,
    LED_YELLOW,
    LED_MAGENTA,
    LED_WHITE
};

void led_init();
void led_set(LedColor color);
void led_blink(LedColor color, uint16_t duration_ms);

#endif // HAL_LED_H
