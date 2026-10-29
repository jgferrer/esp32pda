#include "hal/buzzer.h"

static bool buzzer_enabled = true;

void buzzer_init() {
    ledcSetup(BUZZER_CHANNEL, 2000, 8);
    ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
    ledcWrite(BUZZER_CHANNEL, 0); // Silencio
}

void buzzer_click() {
    if (!buzzer_enabled) return;
    ledcWriteTone(BUZZER_CHANNEL, 2400);
    delay(4);
    ledcWrite(BUZZER_CHANNEL, 0);
}

void buzzer_beep(uint16_t freq, uint16_t duration_ms) {
    if (!buzzer_enabled) return;
    ledcWriteTone(BUZZER_CHANNEL, freq);
    delay(duration_ms);
    ledcWrite(BUZZER_CHANNEL, 0);
}

void buzzer_notification() {
    if (!buzzer_enabled) return;
    ledcWriteTone(BUZZER_CHANNEL, 1760); // A6
    delay(60);
    ledcWriteTone(BUZZER_CHANNEL, 2637); // E7
    delay(90);
    ledcWrite(BUZZER_CHANNEL, 0);
}

void buzzer_alarm_step() {
    if (!buzzer_enabled) return;
    ledcWriteTone(BUZZER_CHANNEL, 2093); // C7
    delay(100);
    ledcWrite(BUZZER_CHANNEL, 0);
    delay(80);
    ledcWriteTone(BUZZER_CHANNEL, 2093);
    delay(100);
    ledcWrite(BUZZER_CHANNEL, 0);
}

void buzzer_set_enabled(bool enabled) {
    buzzer_enabled = enabled;
    if (!enabled) ledcWrite(BUZZER_CHANNEL, 0);
}

bool buzzer_is_enabled() {
    return buzzer_enabled;
}
