#ifndef UI_WIDGETS_H
#define UI_WIDGETS_H

#include <Arduino.h>
#include "config.h"
#include "hal/display.h"

struct Button {
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
    const char* label;
    uint16_t bg_color;
    uint16_t text_color;
    uint16_t border_color;
    bool is_pressed;

    Button(int16_t _x = 0, int16_t _y = 0, int16_t _w = 0, int16_t _h = 0, 
           const char* _label = "", uint16_t _bg = COLOR_BTN_DEFAULT, 
           uint16_t _txt = COLOR_TEXT, uint16_t _brd = COLOR_CARD_BORDER)
        : x(_x), y(_y), w(_w), h(_h), label(_label), 
          bg_color(_bg), text_color(_txt), border_color(_brd), is_pressed(false) {}

    bool contains(int16_t tx, int16_t ty) const {
        return (tx >= x && tx <= (x + w) && ty >= y && ty <= (y + h));
    }

    void draw(bool pressed = false) const {
        uint16_t fill = pressed ? COLOR_BTN_PRESSED : bg_color;
        tft.fillRoundRect(x, y, w, h, 4, fill);
        tft.drawRoundRect(x, y, w, h, 4, border_color);
        tft.setTextColor(text_color, fill);
        tft.setTextDatum(MC_DATUM);
        tft.drawString(label, x + w / 2, y + h / 2, &fonts::Font2);
    }
};

void draw_card(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t bg = COLOR_CARD, uint16_t border = COLOR_CARD_BORDER);

#endif // UI_WIDGETS_H
