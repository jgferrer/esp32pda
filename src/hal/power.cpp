#include "hal/power.h"
#include "hal/display.h"

static unsigned long last_activity_ms = 0;
static uint16_t timeout_seconds = 180; // 3 minutos por defecto
static bool is_dimmed = false;
static uint8_t saved_brightness = 220;

void power_init() {
    last_activity_ms = millis();
    is_dimmed = false;
}

void power_notify_activity() {
    last_activity_ms = millis();
    if (is_dimmed) {
        power_wake_up();
    }
}

void power_update() {
    if (timeout_seconds == 0) return; // Desactivado

    unsigned long elapsed = (millis() - last_activity_ms) / 1000;
    if (!is_dimmed && elapsed >= timeout_seconds) {
        saved_brightness = display_get_brightness();
        display_set_brightness(10); // Atenuar retroiluminación al mínimo
        is_dimmed = true;
    }
}

bool power_is_screen_dimmed() {
    return is_dimmed;
}

void power_wake_up() {
    if (is_dimmed) {
        display_set_brightness(saved_brightness);
        is_dimmed = false;
    }
    last_activity_ms = millis();
}

void power_set_timeout_seconds(uint16_t seconds) {
    timeout_seconds = seconds;
}

uint16_t power_get_timeout_seconds() {
    return timeout_seconds;
}
