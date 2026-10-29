#include "ui/nav_bar.h"
#include "hal/display.h"
#include "hal/buzzer.h"
#include "ui/widgets.h"

static char action_label[24] = "";
static bool show_back_btn = true;

static Button btn_home(4, SCREEN_HEIGHT - NAV_BAR_HEIGHT + 2, 60, NAV_BAR_HEIGHT - 4, "INICIO", COLOR_BTN_DEFAULT, COLOR_TEXT, COLOR_CARD_BORDER);
static Button btn_back(68, SCREEN_HEIGHT - NAV_BAR_HEIGHT + 2, 60, NAV_BAR_HEIGHT - 4, "< ATRAS", COLOR_BTN_DEFAULT, COLOR_TEXT, COLOR_CARD_BORDER);
static Button btn_action(134, SCREEN_HEIGHT - NAV_BAR_HEIGHT + 2, 102, NAV_BAR_HEIGHT - 4, "", COLOR_ACCENT, COLOR_TEXT, COLOR_ACCENT);

void nav_bar_init() {
    action_label[0] = '\0';
    show_back_btn = true;
}

void nav_bar_set_action(const char* label) {
    if (label) {
        strncpy(action_label, label, sizeof(action_label) - 1);
        action_label[sizeof(action_label) - 1] = '\0';
    } else {
        action_label[0] = '\0';
    }
    btn_action.label = action_label;
}

void nav_bar_show_back(bool show) {
    show_back_btn = show;
}

void nav_bar_draw() {
    int16_t y_start = SCREEN_HEIGHT - NAV_BAR_HEIGHT;
    tft.fillRect(0, y_start, SCREEN_WIDTH, NAV_BAR_HEIGHT, COLOR_NAV);
    tft.drawFastHLine(0, y_start, SCREEN_WIDTH, COLOR_CARD_BORDER);

    btn_home.draw();
    if (show_back_btn) {
        btn_back.draw();
    }
    if (strlen(action_label) > 0) {
        btn_action.draw();
    }
}

NavEvent nav_bar_handle_touch(int16_t tx, int16_t ty) {
    int16_t y_start = SCREEN_HEIGHT - NAV_BAR_HEIGHT;
    if (ty < y_start) return NAV_NONE;

    if (btn_home.contains(tx, ty)) {
        buzzer_click();
        btn_home.draw(true);
        delay(60);
        btn_home.draw(false);
        return NAV_HOME;
    }

    if (show_back_btn && btn_back.contains(tx, ty)) {
        buzzer_click();
        btn_back.draw(true);
        delay(60);
        btn_back.draw(false);
        return NAV_BACK;
    }

    if (strlen(action_label) > 0 && btn_action.contains(tx, ty)) {
        buzzer_click();
        btn_action.draw(true);
        delay(60);
        btn_action.draw(false);
        return NAV_ACTION;
    }

    return NAV_NONE;
}
