#include "apps/settings_app.h"
#include "hal/display.h"
#include "hal/buzzer.h"
#include "hal/power.h"
#include "hal/storage.h"
#include "ui/nav_bar.h"
#include "ui/keyboard.h"
#include <WiFi.h>

static SettingsApp* instance = nullptr;
static String wifi_password_buffer = "";
static String selected_ssid_for_password = "";

static void on_wifi_password_entered(bool saved) {
    if (saved && instance) {
        wifi_service_connect(selected_ssid_for_password, wifi_password_buffer);
        buzzer_notification();
    }
    wifi_password_buffer = "";
    if (instance) instance->on_draw();
}

void SettingsApp::on_enter() {
    instance = this;
    current_tab = TAB_WIFI;
    is_scanning = false;
    scanned_list.clear();
    nav_bar_set_action(nullptr);
}

void SettingsApp::on_exit() {
    instance = nullptr;
}

void SettingsApp::on_update() {}

void SettingsApp::draw_tab_bar() {
    int16_t y = 28;
    int16_t tab_w = 74;
    int16_t tab_h = 24;

    const char* tab_names[] = { "WiFi", "Pantalla", "Sistema" };

    for (int i = 0; i < 3; ++i) {
        int16_t x = 8 + i * (tab_w + 2);
        bool is_sel = (current_tab == (SettingsTab)i);
        uint16_t bg = is_sel ? COLOR_ACCENT : COLOR_CARD;
        uint16_t txt = is_sel ? COLOR_BG : COLOR_TEXT;

        tft.fillRoundRect(x, y, tab_w, tab_h, 3, bg);
        tft.drawRoundRect(x, y, tab_w, tab_h, 3, COLOR_CARD_BORDER);

        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(txt, bg);
        tft.drawString(tab_names[i], x + tab_w / 2, y + tab_h / 2, &fonts::Font0);
    }
}

void SettingsApp::draw_wifi_tab() {
    int16_t y = 58;

    // Tarjeta estado WiFi
    tft.fillRect(8, y, SCREEN_WIDTH - 16, 68, COLOR_CARD);
    tft.drawRoundRect(8, y, SCREEN_WIDTH - 16, 68, 6, COLOR_CARD_BORDER);

    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(COLOR_ACCENT, COLOR_CARD);
    tft.drawString("ESTADO WIFI", 16, y + 8, &fonts::Font2);

    if (wifi_service_is_connected()) {
        tft.setTextColor(COLOR_SUCCESS, COLOR_CARD);
        tft.drawString("Conectado", 16, y + 26, &fonts::Font0);

        tft.setTextColor(COLOR_TEXT, COLOR_CARD);
        char buf[48];
        snprintf(buf, sizeof(buf), "Red: %s (%d dBm)", wifi_service_get_current_ssid().c_str(), wifi_service_get_rssi());
        tft.drawString(buf, 16, y + 38, &fonts::Font0);

        snprintf(buf, sizeof(buf), "IP: %s", wifi_service_get_ip().c_str());
        tft.drawString(buf, 16, y + 50, &fonts::Font0);

        // Botón desconectar
        tft.fillRoundRect(SCREEN_WIDTH - 84, y + 8, 72, 22, 3, COLOR_DANGER);
        tft.setTextColor(COLOR_TEXT, COLOR_DANGER);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("DESCON.", SCREEN_WIDTH - 48, y + 19, &fonts::Font0);
    } else {
        tft.setTextColor(COLOR_WARNING, COLOR_CARD);
        tft.drawString("Desconectado", 16, y + 28, &fonts::Font0);

        tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
        tft.drawString("Toca 'ESCANEAR REDES' para buscar.", 16, y + 46, &fonts::Font0);
    }

    // Botón Escanear Redes
    y = 132;
    tft.fillRoundRect(8, y, SCREEN_WIDTH - 16, 28, 4, COLOR_BTN_DEFAULT);
    tft.drawRoundRect(8, y, SCREEN_WIDTH - 16, 28, 4, COLOR_ACCENT);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(COLOR_ACCENT, COLOR_BTN_DEFAULT);
    tft.drawString(is_scanning ? "Escaneando..." : "ESCANEAR REDES 2.4GHz", 120, y + 14, &fonts::Font2);

    // Lista de redes escaneadas
    y = 166;
    int max_visible = 4;
    for (size_t i = 0; i < scanned_list.size() && (int)i < max_visible; ++i) {
        const auto& net = scanned_list[i];
        int16_t row_y = y + i * 28;

        tft.fillRoundRect(8, row_y, SCREEN_WIDTH - 16, 24, 3, COLOR_CARD);
        tft.drawRoundRect(8, row_y, SCREEN_WIDTH - 16, 24, 3, COLOR_CARD_BORDER);

        tft.setTextDatum(ML_DATUM);
        tft.setTextColor(COLOR_TEXT, COLOR_CARD);
        String name = net.ssid;
        if (name.length() > 18) name = name.substring(0, 18) + "...";
        tft.drawString(name.c_str(), 14, row_y + 12, &fonts::Font0);

        tft.setTextDatum(MR_DATUM);
        tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
        char info[16];
        snprintf(info, sizeof(info), "%s %ddB", net.is_open ? "[AB]" : "[*]", net.rssi);
        tft.drawString(info, SCREEN_WIDTH - 14, row_y + 12, &fonts::Font0);
    }
}

void SettingsApp::draw_display_tab() {
    int16_t y = 58;

    // Brillo de pantalla
    tft.fillRect(8, y, SCREEN_WIDTH - 16, 70, COLOR_CARD);
    tft.drawRoundRect(8, y, SCREEN_WIDTH - 16, 70, 6, COLOR_CARD_BORDER);

    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(COLOR_ACCENT, COLOR_CARD);
    tft.drawString("BRILLO DE PANTALLA", 16, y + 8, &fonts::Font2);

    int pct = (display_get_brightness() * 100) / 255;
    char pct_str[16];
    snprintf(pct_str, sizeof(pct_str), "%d %%", pct);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(COLOR_TEXT, COLOR_CARD);
    tft.drawString(pct_str, 120, y + 42, &fonts::Font4);

    // Botones [-] y [+]
    tft.fillRoundRect(20, y + 28, 44, 30, 4, COLOR_BTN_DEFAULT);
    tft.drawRoundRect(20, y + 28, 44, 30, 4, COLOR_CARD_BORDER);
    tft.drawString("-", 42, y + 43, &fonts::Font4);

    tft.fillRoundRect(SCREEN_WIDTH - 64, y + 28, 44, 30, 4, COLOR_BTN_DEFAULT);
    tft.drawRoundRect(SCREEN_WIDTH - 64, y + 28, 44, 30, 4, COLOR_CARD_BORDER);
    tft.drawString("+", SCREEN_WIDTH - 42, y + 43, &fonts::Font4);

    // Sonido y feedback
    y = 136;
    tft.fillRect(8, y, SCREEN_WIDTH - 16, 60, COLOR_CARD);
    tft.drawRoundRect(8, y, SCREEN_WIDTH - 16, 60, 6, COLOR_CARD_BORDER);

    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(COLOR_ACCENT, COLOR_CARD);
    tft.drawString("SONIDO / CLICKS", 16, y + 8, &fonts::Font2);

    bool snd_on = buzzer_is_enabled();
    tft.setTextDatum(ML_DATUM);
    tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
    tft.drawString(snd_on ? "Sonidos tactiles activados" : "Silenciado", 16, y + 36, &fonts::Font0);

    tft.fillRoundRect(SCREEN_WIDTH - 84, y + 24, 72, 26, 4, snd_on ? COLOR_SUCCESS : COLOR_DANGER);
    tft.setTextColor(COLOR_TEXT, snd_on ? COLOR_SUCCESS : COLOR_DANGER);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(snd_on ? "ACTIVO" : "MUTE", SCREEN_WIDTH - 48, y + 37, &fonts::Font2);

    // Reposo automático
    y = 204;
    tft.fillRect(8, y, SCREEN_WIDTH - 16, 76, COLOR_CARD);
    tft.drawRoundRect(8, y, SCREEN_WIDTH - 16, 76, 6, COLOR_CARD_BORDER);

    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(COLOR_ACCENT, COLOR_CARD);
    tft.drawString("APAGADO AUTOMATICO", 16, y + 8, &fonts::Font2);

    uint16_t to = power_get_timeout_seconds();
    const char* to_opts[] = { "1m", "3m", "5m", "OFF" };
    uint16_t to_vals[] = { 60, 180, 300, 0 };

    for (int i = 0; i < 4; ++i) {
        int16_t bx = 16 + i * 54;
        bool sel = (to == to_vals[i]);
        uint16_t bcol = sel ? COLOR_HIGHLIGHT : COLOR_BTN_DEFAULT;
        uint16_t tcol = sel ? COLOR_BG : COLOR_TEXT;

        tft.fillRoundRect(bx, y + 34, 46, 26, 3, bcol);
        tft.drawRoundRect(bx, y + 34, 46, 26, 3, COLOR_CARD_BORDER);
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(tcol, bcol);
        tft.drawString(to_opts[i], bx + 23, y + 47, &fonts::Font0);
    }
}

void SettingsApp::draw_system_tab() {
    int16_t y = 58;

    tft.fillRect(8, y, SCREEN_WIDTH - 16, 222, COLOR_CARD);
    tft.drawRoundRect(8, y, SCREEN_WIDTH - 16, 222, 6, COLOR_CARD_BORDER);

    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(COLOR_ACCENT, COLOR_CARD);
    tft.drawString("INFORMACION DEL SISTEMA", 16, y + 10, &fonts::Font2);

    tft.drawFastHLine(16, y + 30, SCREEN_WIDTH - 32, COLOR_CARD_BORDER);

    int16_t line_y = y + 40;
    char buf[64];

    snprintf(buf, sizeof(buf), "S.O.: ESP32 PocketOS v1.0");
    tft.setTextColor(COLOR_TEXT, COLOR_CARD);
    tft.drawString(buf, 16, line_y, &fonts::Font0); line_y += 18;

    snprintf(buf, sizeof(buf), "Placa: ESP32-2432S028 (CYD)");
    tft.drawString(buf, 16, line_y, &fonts::Font0); line_y += 18;

    snprintf(buf, sizeof(buf), "Pantalla: TFT 2.8\" (240x320)");
    tft.drawString(buf, 16, line_y, &fonts::Font0); line_y += 18;

    snprintf(buf, sizeof(buf), "RAM Libre: %u KB", ESP.getFreeHeap() / 1024);
    tft.drawString(buf, 16, line_y, &fonts::Font0); line_y += 18;

    snprintf(buf, sizeof(buf), "CPU: %u MHz (Dual Core)", ESP.getCpuFreqMHz());
    tft.drawString(buf, 16, line_y, &fonts::Font0); line_y += 18;

    snprintf(buf, sizeof(buf), "Flash LittleFS: %u / %u KB", (unsigned int)(storage_used_bytes() / 1024), (unsigned int)(storage_total_bytes() / 1024));
    tft.drawString(buf, 16, line_y, &fonts::Font0); line_y += 18;

    snprintf(buf, sizeof(buf), "MAC: %s", WiFi.macAddress().c_str());
    tft.drawString(buf, 16, line_y, &fonts::Font0); line_y += 18;

    unsigned long up_sec = millis() / 1000;
    snprintf(buf, sizeof(buf), "Tiempo encendido: %lum %lus", up_sec / 60, up_sec % 60);
    tft.drawString(buf, 16, line_y, &fonts::Font0);
}

void SettingsApp::on_draw() {
    tft.fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH, CONTENT_HEIGHT, COLOR_BG);
    draw_tab_bar();

    switch (current_tab) {
        case TAB_WIFI:    draw_wifi_tab(); break;
        case TAB_DISPLAY: draw_display_tab(); break;
        case TAB_SYSTEM:  draw_system_tab(); break;
    }
}

bool SettingsApp::on_touch(int16_t tx, int16_t ty, bool is_new_press) {
    if (!is_new_press) return false;

    // Toque en barra de pestañas (Y=28 a 52)
    if (ty >= 28 && ty <= 52) {
        int16_t tab_w = 74;
        for (int i = 0; i < 3; ++i) {
            int16_t x = 8 + i * (tab_w + 2);
            if (tx >= x && tx <= x + tab_w) {
                buzzer_click();
                current_tab = (SettingsTab)i;
                on_draw();
                return true;
            }
        }
    }

    // Pestaña WiFi
    if (current_tab == TAB_WIFI) {
        // Botón desconectar (SCREEN_WIDTH-84 a -12, Y:66 a 88)
        if (wifi_service_is_connected() && tx >= SCREEN_WIDTH - 84 && tx <= SCREEN_WIDTH - 12 && ty >= 66 && ty <= 88) {
            buzzer_notification();
            wifi_service_disconnect();
            on_draw();
            return true;
        }

        // Botón Escanear Redes (Y: 132 a 160)
        if (tx >= 8 && tx <= SCREEN_WIDTH - 8 && ty >= 132 && ty <= 160) {
            buzzer_click();
            is_scanning = true;
            tft.fillRoundRect(8, 132, SCREEN_WIDTH - 16, 28, 4, COLOR_BTN_DEFAULT);
            tft.setTextDatum(MC_DATUM);
            tft.setTextColor(COLOR_HIGHLIGHT, COLOR_BTN_DEFAULT);
            tft.drawString("Escaneando redes...", 120, 146, &fonts::Font2);

            scanned_list = wifi_service_scan_networks();
            is_scanning = false;
            buzzer_notification();
            on_draw();
            return true;
        }

        // Toque en una red escaneada para conectar (Y >= 166)
        if (ty >= 166) {
            int idx = (ty - 166) / 28;
            if (idx >= 0 && idx < (int)scanned_list.size() && idx < 4) {
                buzzer_click();
                selected_ssid_for_password = scanned_list[idx].ssid;
                wifi_password_buffer = "";
                char prompt[48];
                snprintf(prompt, sizeof(prompt), "Clave: %s", selected_ssid_for_password.c_str());
                keyboard_open(&wifi_password_buffer, prompt, on_wifi_password_entered);
                return true;
            }
        }
    }

    // Pestaña Pantalla
    if (current_tab == TAB_DISPLAY) {
        // Brillo [-] (20 a 64, Y: 86 a 116)
        if (tx >= 20 && tx <= 64 && ty >= 86 && ty <= 116) {
            buzzer_click();
            uint8_t br = display_get_brightness();
            if (br > 35) br -= 30; else br = 15;
            display_set_brightness(br);
            on_draw();
            return true;
        }

        // Brillo [+] (SCREEN_WIDTH - 64 a SCREEN_WIDTH - 20, Y: 86 a 116)
        if (tx >= SCREEN_WIDTH - 64 && tx <= SCREEN_WIDTH - 20 && ty >= 86 && ty <= 116) {
            buzzer_click();
            uint8_t br = display_get_brightness();
            if (br < 225) br += 30; else br = 255;
            display_set_brightness(br);
            on_draw();
            return true;
        }

        // Toggle Sonido (SCREEN_WIDTH - 84 a -12, Y: 160 a 186)
        if (tx >= SCREEN_WIDTH - 84 && tx <= SCREEN_WIDTH - 12 && ty >= 160 && ty <= 186) {
            bool current = buzzer_is_enabled();
            buzzer_set_enabled(!current);
            if (!current) buzzer_notification();
            on_draw();
            return true;
        }

        // Timeout de apagado automático
        if (ty >= 238 && ty <= 264) {
            uint16_t to_vals[] = { 60, 180, 300, 0 };
            for (int i = 0; i < 4; ++i) {
                int16_t bx = 16 + i * 54;
                if (tx >= bx && tx <= bx + 46) {
                    buzzer_click();
                    power_set_timeout_seconds(to_vals[i]);
                    on_draw();
                    return true;
                }
            }
        }
    }

    return false;
}

void SettingsApp::on_nav_action() {}
