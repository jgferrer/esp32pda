#ifndef HAL_DISPLAY_H
#define HAL_DISPLAY_H

#include <LovyanGFX.hpp>
#include "config.h"

class LGFX : public lgfx::LGFX_Device {
    lgfx::Panel_ST7789  _panel_instance;
    lgfx::Bus_SPI       _bus_instance;
    lgfx::Light_PWM     _light_instance;
    lgfx::Touch_XPT2046 _touch_instance;

public:
    LGFX(void);
};

extern LGFX tft;

void display_init();
bool display_get_touch(int16_t* x, int16_t* y);
void display_set_brightness(uint8_t brightness);
uint8_t display_get_brightness();

#endif // HAL_DISPLAY_H
