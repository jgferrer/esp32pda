#ifndef APPS_WEATHER_APP_H
#define APPS_WEATHER_APP_H

#include "kernel/app_base.h"

struct WeatherData {
    float temp;
    float windspeed;
    int weather_code;
    bool valid;
    String condition_text;
};

class WeatherApp : public AppBase {
public:
    void on_enter() override;
    void on_exit() override;
    void on_update() override;
    void on_draw() override;
    bool on_touch(int16_t tx, int16_t ty, bool is_new_press) override;
    void on_nav_action() override;
    const char* get_title() const override { return "EL TIEMPO"; }

    static bool fetch_weather();

private:
    static WeatherData current_weather;
    void draw_weather_card();
};

#endif // APPS_WEATHER_APP_H
