#ifndef APPS_LAUNCHER_APP_H
#define APPS_LAUNCHER_APP_H

#include "kernel/app_base.h"
#include "ui/widgets.h"

class LauncherApp : public AppBase {
public:
    void on_enter() override;
    void on_exit() override;
    void on_update() override;
    void on_draw() override;
    bool on_touch(int16_t tx, int16_t ty, bool is_new_press) override;
    const char* get_title() const override { return "ESP32-PDA"; }
};

#endif // APPS_LAUNCHER_APP_H
