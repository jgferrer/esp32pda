#ifndef APPS_CLOCK_APP_H
#define APPS_CLOCK_APP_H

#include "kernel/app_base.h"

class ClockApp : public AppBase {
public:
    void on_enter() override;
    void on_exit() override;
    void on_update() override;
    void on_draw() override;
    bool on_touch(int16_t tx, int16_t ty, bool is_new_press) override;
    void on_nav_action() override;
    const char* get_title() const override { return "RELOJ"; }

private:
    unsigned long last_draw_sec;
    // Cronómetro
    bool stopwatch_running;
    unsigned long stopwatch_start_ms;
    unsigned long stopwatch_elapsed_ms;

    void draw_clock_face();
    void draw_stopwatch();
};

#endif // APPS_CLOCK_APP_H
