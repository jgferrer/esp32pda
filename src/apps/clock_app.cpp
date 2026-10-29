#include "apps/clock_app.h"
#include "hal/display.h"
#include "hal/buzzer.h"
#include "services/time_service.h"
#include "services/wifi_service.h"
#include "ui/nav_bar.h"

void ClockApp::on_enter() {
    last_draw_sec = 0;
    stopwatch_running = false;
    stopwatch_start_ms = 0;
    stopwatch_elapsed_ms = 0;
    nav_bar_set_action("SYNC NTP");
}

void ClockApp::on_exit() {}

void ClockApp::on_update() {
    time_t now = time_service_get_epoch();
    if (now != (time_t)last_draw_sec) {
        last_draw_sec = (unsigned long)now;
        draw_clock_face();
    }

    if (stopwatch_running) {
        draw_stopwatch();
    }
}

void ClockApp::draw_clock_face() {
    // Tarjeta del reloj
    tft.fillRect(8, 30, SCREEN_WIDTH - 16, 110, COLOR_CARD);
    tft.drawRoundRect(8, 30, SCREEN_WIDTH - 16, 110, 6, COLOR_ACCENT);

    // Hora digital grande HH:MM:SS
    String time_str = time_service_get_time_str(true);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(COLOR_HIGHLIGHT, COLOR_CARD);
    tft.drawString(time_str.c_str(), 120, 72, &fonts::Font6);

    // Fecha debajo
    String date_str = time_service_get_full_date_str();
    tft.setTextColor(COLOR_TEXT, COLOR_CARD);
    tft.drawString(date_str.c_str(), 120, 118, &fonts::Font2);

    // Estado NTP
    tft.setTextDatum(TR_DATUM);
    if (time_service_is_synced()) {
        tft.setTextColor(COLOR_SUCCESS, COLOR_CARD);
        tft.drawString("[NTP OK]", SCREEN_WIDTH - 16, 36, &fonts::Font0);
    } else {
        tft.setTextColor(COLOR_WARNING, COLOR_CARD);
        tft.drawString("[NTP PEND]", SCREEN_WIDTH - 16, 36, &fonts::Font0);
    }
}

void ClockApp::draw_stopwatch() {
    int16_t y = 150;
    tft.fillRect(8, y, SCREEN_WIDTH - 16, 134, COLOR_CARD);
    tft.drawRoundRect(8, y, SCREEN_WIDTH - 16, 134, 6, COLOR_CARD_BORDER);

    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(COLOR_ACCENT, COLOR_CARD);
    tft.drawString("CRONOMETRO", 16, y + 8, &fonts::Font2);

    // Tiempo transcurrido
    unsigned long total_ms = stopwatch_elapsed_ms;
    if (stopwatch_running) {
        total_ms += (millis() - stopwatch_start_ms);
    }
    unsigned long total_sec = total_ms / 1000;
    unsigned int mins = total_sec / 60;
    unsigned int secs = total_sec % 60;
    unsigned int tenths = (total_ms % 1000) / 100;

    char sw_buf[16];
    snprintf(sw_buf, sizeof(sw_buf), "%02u:%02u.%u", mins, secs, tenths);

    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(COLOR_TEXT, COLOR_CARD);
    tft.drawString(sw_buf, 120, y + 54, &fonts::Font4);

    // Botones Iniciar/Pausar y Reset
    // Botón Start/Stop
    uint16_t btn_col = stopwatch_running ? COLOR_WARNING : COLOR_SUCCESS;
    const char* btn_txt = stopwatch_running ? "PAUSAR" : "INICIAR";
    tft.fillRoundRect(16, y + 88, 96, 36, 4, btn_col);
    tft.setTextColor(COLOR_TEXT, btn_col);
    tft.drawString(btn_txt, 16 + 48, y + 88 + 18, &fonts::Font2);

    // Botón Reset
    tft.fillRoundRect(128, y + 88, 96, 36, 4, COLOR_BTN_DEFAULT);
    tft.drawRoundRect(128, y + 88, 96, 36, 4, COLOR_CARD_BORDER);
    tft.setTextColor(COLOR_TEXT, COLOR_BTN_DEFAULT);
    tft.drawString("BORRAR", 128 + 48, y + 88 + 18, &fonts::Font2);
}

void ClockApp::on_draw() {
    tft.fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH, CONTENT_HEIGHT, COLOR_BG);
    draw_clock_face();
    draw_stopwatch();
}

bool ClockApp::on_touch(int16_t tx, int16_t ty, bool is_new_press) {
    if (!is_new_press) return false;

    int16_t y = 150;
    // Botón Iniciar/Pausar (16 a 112, y+88 a y+124)
    if (tx >= 16 && tx <= 112 && ty >= (y + 88) && ty <= (y + 124)) {
        buzzer_click();
        if (stopwatch_running) {
            stopwatch_elapsed_ms += (millis() - stopwatch_start_ms);
            stopwatch_running = false;
        } else {
            stopwatch_start_ms = millis();
            stopwatch_running = true;
        }
        draw_stopwatch();
        return true;
    }

    // Botón Borrar / Reset (128 a 224, y+88 a y+124)
    if (tx >= 128 && tx <= 224 && ty >= (y + 88) && ty <= (y + 124)) {
        buzzer_click();
        stopwatch_running = false;
        stopwatch_elapsed_ms = 0;
        draw_stopwatch();
        return true;
    }

    return false;
}

void ClockApp::on_nav_action() {
    buzzer_click();
    if (!wifi_service_is_connected()) {
        buzzer_beep(1000, 200);
    } else {
        bool ok = time_service_sync_ntp();
        if (ok) {
            buzzer_notification();
        } else {
            buzzer_beep(1200, 100);
        }
        draw_clock_face();
    }
}
