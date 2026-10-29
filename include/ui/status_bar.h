#ifndef UI_STATUS_BAR_H
#define UI_STATUS_BAR_H

#include <Arduino.h>

void status_bar_init();
void status_bar_set_title(const char* title);
void status_bar_draw(bool force = false);
void status_bar_update();

#endif // UI_STATUS_BAR_H
