#include "hal/led.h"

void led_init() {
    pinMode(LED_RED_PIN, OUTPUT);
    pinMode(LED_GREEN_PIN, OUTPUT);
    pinMode(LED_BLUE_PIN, OUTPUT);
    led_set(LED_OFF);
}

void led_set(LedColor color) {
    // LED RGB Integrado es Active-LOW: 0 = Encendido, 1 = Apagado
    bool r = HIGH, g = HIGH, b = HIGH;

    switch (color) {
        case LED_RED:     r = LOW; break;
        case LED_GREEN:   g = LOW; break;
        case LED_BLUE:    b = LOW; break;
        case LED_CYAN:    g = LOW; b = LOW; break;
        case LED_YELLOW:  r = LOW; g = LOW; break;
        case LED_MAGENTA: r = LOW; b = LOW; break;
        case LED_WHITE:   r = LOW; g = LOW; b = LOW; break;
        case LED_OFF:
        default:          break;
    }

    digitalWrite(LED_RED_PIN, r);
    digitalWrite(LED_GREEN_PIN, g);
    digitalWrite(LED_BLUE_PIN, b);
}

void led_blink(LedColor color, uint16_t duration_ms) {
    led_set(color);
    delay(duration_ms);
    led_set(LED_OFF);
}
