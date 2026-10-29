#ifndef APPS_CALENDAR_APP_H
#define APPS_CALENDAR_APP_H

#include "kernel/app_base.h"
#include <vector>

struct CalendarEvent {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    String title;
};

class CalendarApp : public AppBase {
public:
    void on_enter() override;
    void on_exit() override;
    void on_update() override;
    void on_draw() override;
    bool on_touch(int16_t tx, int16_t ty, bool is_new_press) override;
    void on_nav_action() override;
    const char* get_title() const override { return "AGENDA"; }

    static void load_events();
    static void save_events();
    static const std::vector<CalendarEvent>& get_events();

private:
    int16_t view_year;
    int8_t view_month; // 1-12
    int8_t selected_day;

    void draw_calendar_grid();
    void draw_event_list();
    bool has_event_on_day(int day);
};

#endif // APPS_CALENDAR_APP_H
