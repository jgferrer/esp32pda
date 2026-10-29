#include "apps/weather_app.h"
#include "hal/display.h"
#include "hal/buzzer.h"
#include "services/wifi_service.h"
#include "ui/nav_bar.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>

WeatherData WeatherApp::current_weather = { 0.0f, 0.0f, 0, false, "Sin datos" };

static String get_wmo_description(int code) {
    switch (code) {
        case 0:  return "Cielo Despejado";
        case 1:
        case 2:  return "Poco Nuboso";
        case 3:  return "Nublado";
        case 45:
        case 48: return "Niebla";
        case 51:
        case 53:
        case 55: return "Llovizna";
        case 61:
        case 63:
        case 65: return "Lluvia";
        case 71:
        case 73:
        case 75: return "Nieve";
        case 80:
        case 81:
        case 82: return "Chubascos";
        case 95:
        case 96: return "Tormenta";
        default: return "Variable";
    }
}

bool WeatherApp::fetch_weather() {
    if (!wifi_service_is_connected()) return false;

    HTTPClient http;
    http.setTimeout(5000);
    // API de Open-Meteo (Madrid por defecto; libre sin API key)
    http.begin("http://api.open-meteo.com/v1/forecast?latitude=40.4168&longitude=-3.7038&current_weather=true");
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, payload);
        if (!err) {
            JsonObject cw = doc["current_weather"];
            current_weather.temp = cw["temperature"] | 0.0f;
            current_weather.windspeed = cw["windspeed"] | 0.0f;
            current_weather.weather_code = cw["weathercode"] | 0;
            current_weather.condition_text = get_wmo_description(current_weather.weather_code);
            current_weather.valid = true;
            http.end();
            return true;
        }
    }
    http.end();
    return false;
}

void WeatherApp::on_enter() {
    nav_bar_set_action("ACTUALIZAR");
    if (!current_weather.valid && wifi_service_is_connected()) {
        fetch_weather();
    }
}

void WeatherApp::on_exit() {}

void WeatherApp::on_update() {}

void WeatherApp::draw_weather_card() {
    tft.fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH, CONTENT_HEIGHT, COLOR_BG);

    if (!wifi_service_is_connected()) {
        tft.fillRect(10, 40, SCREEN_WIDTH - 20, 160, COLOR_CARD);
        tft.drawRoundRect(10, 40, SCREEN_WIDTH - 20, 160, 6, COLOR_WARNING);

        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(COLOR_WARNING, COLOR_CARD);
        tft.drawString("WiFi Desconectado", 120, 80, &fonts::Font2);

        tft.setTextColor(COLOR_TEXT, COLOR_CARD);
        tft.drawString("Conecta el WiFi en 'AJUSTES'", 120, 110, &fonts::Font0);
        tft.drawString("para ver el pronostico del tiempo.", 120, 126, &fonts::Font0);

        tft.fillRoundRect(30, 150, SCREEN_WIDTH - 60, 32, 4, COLOR_BTN_DEFAULT);
        tft.drawRoundRect(30, 150, SCREEN_WIDTH - 60, 32, 4, COLOR_CARD_BORDER);
        tft.setTextColor(COLOR_ACCENT, COLOR_BTN_DEFAULT);
        tft.drawString("Abrir Ajustes", 120, 166, &fonts::Font2);
        return;
    }

    // Tarjeta del tiempo
    tft.fillRect(8, 30, SCREEN_WIDTH - 16, 250, COLOR_CARD);
    tft.drawRoundRect(8, 30, SCREEN_WIDTH - 16, 250, 6, COLOR_ACCENT);

    // Ciudad
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(COLOR_ACCENT, COLOR_CARD);
    tft.drawString("MADRID / LOCAL", 16, 40, &fonts::Font2);

    tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
    tft.drawString("Open-Meteo Service", 16, 58, &fonts::Font0);

    // Dibujar icono climático gráfico
    int16_t icon_x = 70;
    int16_t icon_y = 115;
    if (current_weather.weather_code <= 1) {
        // Sol
        tft.fillCircle(icon_x, icon_y, 22, COLOR_HIGHLIGHT);
        tft.drawCircle(icon_x, icon_y, 26, COLOR_WARNING);
    } else if (current_weather.weather_code <= 3) {
        // Sol con nube
        tft.fillCircle(icon_x - 10, icon_y - 10, 14, COLOR_HIGHLIGHT);
        tft.fillRoundRect(icon_x - 14, icon_y, 36, 18, 9, COLOR_TEXT_MUTED);
    } else {
        // Nube con lluvia
        tft.fillRoundRect(icon_x - 14, icon_y - 6, 36, 18, 9, COLOR_TEXT_MUTED);
        tft.drawLine(icon_x - 8, icon_y + 16, icon_x - 12, icon_y + 24, COLOR_ACCENT);
        tft.drawLine(icon_x, icon_y + 16, icon_x - 4, icon_y + 24, COLOR_ACCENT);
        tft.drawLine(icon_x + 8, icon_y + 16, icon_x + 4, icon_y + 24, COLOR_ACCENT);
    }

    // Gran temperatura
    tft.setTextDatum(ML_DATUM);
    tft.setTextColor(COLOR_TEXT, COLOR_CARD);
    char temp_buf[16];
    snprintf(temp_buf, sizeof(temp_buf), "%.1f C", current_weather.temp);
    tft.drawString(temp_buf, 120, 115, &fonts::Font6);

    // Condición meteorológica
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(COLOR_HIGHLIGHT, COLOR_CARD);
    tft.drawString(current_weather.condition_text.c_str(), 120, 168, &fonts::Font4);

    tft.drawFastHLine(20, 192, SCREEN_WIDTH - 40, COLOR_CARD_BORDER);

    // Viento y detalles
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
    char wind_buf[32];
    snprintf(wind_buf, sizeof(wind_buf), "Viento: %.1f km/h", current_weather.windspeed);
    tft.drawString(wind_buf, 120, 210, &fonts::Font2);

    tft.setTextColor(COLOR_SUCCESS, COLOR_CARD);
    tft.drawString("Pronostico actualizado en vivo", 120, 235, &fonts::Font0);
}

void WeatherApp::on_draw() {
    draw_weather_card();
}

bool WeatherApp::on_touch(int16_t tx, int16_t ty, bool is_new_press) {
    if (!is_new_press) return false;

    // Si no hay wifi y toca el botón "Abrir Ajustes"
    if (!wifi_service_is_connected()) {
        if (tx >= 30 && tx <= (SCREEN_WIDTH - 30) && ty >= 150 && ty <= 182) {
            buzzer_click();
            extern void app_manager_switch_to(AppID id);
            app_manager_switch_to(APP_SETTINGS);
            return true;
        }
    }
    return false;
}

void WeatherApp::on_nav_action() {
    buzzer_click();
    if (wifi_service_is_connected()) {
        tft.fillRect(16, 230, SCREEN_WIDTH - 32, 20, COLOR_CARD);
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(COLOR_WARNING, COLOR_CARD);
        tft.drawString("Descargando...", 120, 240, &fonts::Font0);

        fetch_weather();
        buzzer_notification();
        draw_weather_card();
    } else {
        buzzer_beep(800, 150);
    }
}
