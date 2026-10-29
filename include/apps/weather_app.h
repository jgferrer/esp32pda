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

struct CityPreset {
    const char* name;
    float lat;
    float lon;
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
    static bool geocode_and_set_city(const String& city_query);
    static void set_city(const String& name, float lat, float lon);
    static void load_city_config();
    static void save_city_config();

    static String get_city_name();

private:
    static WeatherData current_weather;
    static String current_city_name;
    static float current_lat;
    static float current_lon;
    bool is_selecting_city;

    void draw_weather_card();
    void draw_city_selector();
};

#endif // APPS_WEATHER_APP_H
