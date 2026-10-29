#ifndef HAL_POWER_H
#define HAL_POWER_H

#include <Arduino.h>

void power_init();
void power_notify_activity();
void power_update();
bool power_is_screen_dimmed();
void power_wake_up();
void power_set_timeout_seconds(uint16_t seconds);
uint16_t power_get_timeout_seconds();

#endif // HAL_POWER_H
