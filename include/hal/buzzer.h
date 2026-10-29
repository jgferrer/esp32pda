#ifndef HAL_BUZZER_H
#define HAL_BUZZER_H

#include <Arduino.h>
#include "config.h"

void buzzer_init();
void buzzer_click();
void buzzer_beep(uint16_t freq, uint16_t duration_ms);
void buzzer_notification();
void buzzer_alarm_step();
void buzzer_set_enabled(bool enabled);
bool buzzer_is_enabled();

#endif // HAL_BUZZER_H
