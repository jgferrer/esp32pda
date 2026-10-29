#include "apps/launcher_app.h"
#include "hal/display.h"
#include "hal/buzzer.h"
#include "kernel/app_manager.h"
#include "services/time_service.h"
#include "ui/nav_bar.h"

struct LauncherItem {
    AppID id;
    int16_t x, y, w, h;
    const char* title;
    const char* subtitle;
    uint16_t accent_color;
};

static LauncherItem items[] = {
    { APP_CALENDAR,   8,  72, 108, 64, "AGENDA",     "Eventos",   0x3E18 }, // Verde
    { APP_NOTES,     124, 72, 108, 64, "NOTAS",      "Memos",     0xFD20 }, // Naranja
    { APP_CLOCK,      8, 144, 108, 64, "RELOJ",      "Alarmas",   0x05BF }, // Cyan
    { APP_CALCULATOR,124, 144, 108, 64, "CALC",       "Bolsillo",  0xAF1E }, // Púrpura
    { APP_WEATHER,    8, 216, 108, 64, "TIEMPO",     "Prevision", 0x3D7E }, // Azul cielo
    { APP_SETTINGS,  124, 216, 108, 64, "AJUSTES",    "WiFi/Sys",  0x9CD3 }  // Gris
};

void LauncherApp::on_enter() {
    nav_bar_set_action(nullptr);
}

void LauncherApp::on_exit() {}

void LauncherApp::on_update() {}

void LauncherApp::on_draw() {
    // Banner de bienvenida y fecha
    tft.fillRect(8, 28, 224, 38, COLOR_CARD);
    tft.drawRoundRect(8, 28, 224, 38, 4, COLOR_CARD_BORDER);

    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(COLOR_HIGHLIGHT, COLOR_CARD);
    String date_str = time_service_get_full_date_str();
    tft.drawString(date_str.c_str(), 120, 40, &fonts::Font2);

    tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
    tft.drawString("Selecciona una aplicacion", 120, 56, &fonts::Font0);

    // Dibujar tarjetas de las apps
    for (size_t i = 0; i < sizeof(items) / sizeof(items[0]); ++i) {
        const auto& it = items[i];

        // Fondo de tarjeta
        tft.fillRoundRect(it.x, it.y, it.w, it.h, 6, COLOR_CARD);
        tft.drawRoundRect(it.x, it.y, it.w, it.h, 6, it.accent_color);

        // Barra decorativa superior
        tft.fillRoundRect(it.x + 2, it.y + 2, it.w - 4, 6, 2, it.accent_color);

        // Título de la app
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(COLOR_TEXT, COLOR_CARD);
        tft.drawString(it.title, it.x + it.w / 2, it.y + 26, &fonts::Font2);

        // Subtítulo
        tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
        tft.drawString(it.subtitle, it.x + it.w / 2, it.y + 46, &fonts::Font0);
    }
}

bool LauncherApp::on_touch(int16_t tx, int16_t ty, bool is_new_press) {
    if (!is_new_press) return false;

    for (size_t i = 0; i < sizeof(items) / sizeof(items[0]); ++i) {
        const auto& it = items[i];
        if (tx >= it.x && tx <= (it.x + it.w) && ty >= it.y && ty <= (it.y + it.h)) {
            buzzer_click();
            // Feedback táctil: parpadeo rápido
            tft.drawRoundRect(it.x, it.y, it.w, it.h, 6, COLOR_HIGHLIGHT);
            delay(50);
            app_manager_switch_to(it.id);
            return true;
        }
    }
    return false;
}
