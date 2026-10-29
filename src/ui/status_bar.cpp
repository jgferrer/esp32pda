#include "ui/status_bar.h"
#include "hal/display.h"
#include "services/time_service.h"
#include "services/wifi_service.h"

static char current_title[32] = "ESP32-PDA";
static String last_rendered_time = "";
static bool last_rendered_wifi = false;

void status_bar_init() {
    snprintf(current_title, sizeof(current_title), "ESP32-PDA");
    last_rendered_time = "";
}

void status_bar_set_title(const char* title) {
    if (title) {
        strncpy(current_title, title, sizeof(current_title) - 1);
        current_title[sizeof(current_title) - 1] = '\0';
        status_bar_draw(true);
    }
}

void status_bar_draw(bool force) {
    String now_str = time_service_get_time_str(false);
    bool wifi_ok = wifi_service_is_connected();

    if (!force && now_str == last_rendered_time && wifi_ok == last_rendered_wifi) {
        return; // No hace falta redibujar
    }

    last_rendered_time = now_str;
    last_rendered_wifi = wifi_ok;

    // Fondo barra superior
    tft.fillRect(0, 0, SCREEN_WIDTH, STATUS_BAR_HEIGHT, COLOR_HEADER);
    tft.drawFastHLine(0, STATUS_BAR_HEIGHT - 1, SCREEN_WIDTH, COLOR_CARD_BORDER);

    // Título de la app a la izquierda
    tft.setTextDatum(ML_DATUM);
    tft.setTextColor(COLOR_ACCENT, COLOR_HEADER);
    tft.drawString(current_title, 8, STATUS_BAR_HEIGHT / 2, &fonts::Font2);

    // Indicador WiFi (separado limpiamente a la izquierda de la hora)
    tft.setTextDatum(MR_DATUM);
    if (wifi_ok) {
        tft.setTextColor(COLOR_SUCCESS, COLOR_HEADER);
        tft.drawString("WiFi", SCREEN_WIDTH - 64, STATUS_BAR_HEIGHT / 2, &fonts::Font0);
    } else {
        tft.setTextColor(COLOR_TEXT_MUTED, COLOR_HEADER);
        tft.drawString("----", SCREEN_WIDTH - 64, STATUS_BAR_HEIGHT / 2, &fonts::Font0);
    }

    // Hora actual a la derecha
    tft.setTextDatum(MR_DATUM);
    tft.setTextColor(COLOR_TEXT, COLOR_HEADER);
    tft.drawString(now_str.c_str(), SCREEN_WIDTH - 8, STATUS_BAR_HEIGHT / 2, &fonts::Font2);
}

void status_bar_update() {
    status_bar_draw(false);
}
