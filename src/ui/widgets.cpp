#include "ui/widgets.h"

void draw_card(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t bg, uint16_t border) {
    tft.fillRoundRect(x, y, w, h, 6, bg);
    tft.drawRoundRect(x, y, w, h, 6, border);
}
