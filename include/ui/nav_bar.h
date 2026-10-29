#ifndef UI_NAV_BAR_H
#define UI_NAV_BAR_H

#include <Arduino.h>

enum NavEvent {
    NAV_NONE = 0,
    NAV_BACK,
    NAV_HOME,
    NAV_ACTION
};

void nav_bar_init();
void nav_bar_set_action(const char* label);
void nav_bar_show_back(bool show);
void nav_bar_draw();
NavEvent nav_bar_handle_touch(int16_t tx, int16_t ty);

#endif // UI_NAV_BAR_H
