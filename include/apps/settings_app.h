#ifndef APPS_SETTINGS_APP_H
#define APPS_SETTINGS_APP_H

#include "kernel/app_base.h"
#include "services/wifi_service.h"
#include <vector>

enum SettingsTab {
    TAB_WIFI = 0,
    TAB_DISPLAY,
    TAB_SYSTEM
};

class SettingsApp : public AppBase {
public:
    void on_enter() override;
    void on_exit() override;
    void on_update() override;
    void on_draw() override;
    bool on_touch(int16_t tx, int16_t ty, bool is_new_press) override;
    void on_nav_action() override;
    const char* get_title() const override { return "AJUSTES"; }

private:
    SettingsTab current_tab;
    std::vector<ScannedNetwork> scanned_list;
    bool is_scanning;

    void draw_tab_bar();
    void draw_wifi_tab();
    void draw_display_tab();
    void draw_system_tab();
};

#endif // APPS_SETTINGS_APP_H
