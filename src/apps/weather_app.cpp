#include "apps/weather_app.h"
#include "hal/display.h"
#include "hal/buzzer.h"
#include "hal/storage.h"
#include "services/wifi_service.h"
#include "ui/nav_bar.h"
#include "ui/keyboard.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>

static const char* WEATHER_CITY_FILE = "/weather_city.json";

WeatherData WeatherApp::current_weather = { 0.0f, 0.0f, 0, false, "Sin datos" };
String WeatherApp::current_city_name = "Madrid";
float WeatherApp::current_lat = 40.4168f;
float WeatherApp::current_lon = -3.7038f;

static WeatherApp* instance = nullptr;
static String search_city_buffer = "";

static const CityPreset PRESETS[] = {
    { "Madrid",    40.4168f, -3.7038f },
    { "Barcelona", 41.3879f,  2.1699f },
    { "Valencia",  39.4699f, -0.3763f },
    { "Sevilla",   37.3891f, -5.9845f },
    { "Zaragoza",  41.6561f, -0.8773f },
    { "Malaga",    36.7213f, -4.4214f },
    { "Bilbao",    43.2630f, -2.9350f },
    { "Palma",     39.5696f,  2.6502f },
    { "Alicante",  38.3452f, -0.4810f },
    { "Granada",   37.1773f, -3.5986f }
};
static const int PRESET_COUNT = sizeof(PRESETS) / sizeof(PRESETS[0]);

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

void WeatherApp::load_city_config() {
    if (!storage_file_exists(WEATHER_CITY_FILE)) return;
    String json = storage_read_file(WEATHER_CITY_FILE);
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, json);
    if (!err) {
        const char* name = doc["name"];
        if (name && strlen(name) > 0) current_city_name = name;
        current_lat = doc["lat"] | 40.4168f;
        current_lon = doc["lon"] | -3.7038f;
    }
}

void WeatherApp::save_city_config() {
    JsonDocument doc;
    doc["name"] = current_city_name;
    doc["lat"] = current_lat;
    doc["lon"] = current_lon;
    String out;
    serializeJson(doc, out);
    storage_write_file(WEATHER_CITY_FILE, out);
}

void WeatherApp::set_city(const String& name, float lat, float lon) {
    current_city_name = name;
    current_lat = lat;
    current_lon = lon;
    save_city_config();
    current_weather.valid = false;
    fetch_weather();
}

String WeatherApp::get_city_name() {
    return current_city_name;
}

bool WeatherApp::geocode_and_set_city(const String& city_query) {
    if (!wifi_service_is_connected()) return false;

    // Codificar espacios si los hay
    String clean_query = city_query;
    clean_query.trim();
    clean_query.replace(" ", "%20");

    HTTPClient http;
    http.setTimeout(6000);
    String url = "http://geocoding-api.open-meteo.com/v1/search?name=" + clean_query + "&count=1&language=es&format=json";
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, payload);
        if (!err && doc["results"].is<JsonArray>()) {
            JsonObject first = doc["results"][0];
            const char* found_name = first["name"] | city_query.c_str();
            float lat = first["latitude"] | 0.0f;
            float lon = first["longitude"] | 0.0f;

            if (lat != 0.0f || lon != 0.0f) {
                set_city(found_name, lat, lon);
                http.end();
                return true;
            }
        }
    }
    http.end();
    return false;
}

bool WeatherApp::fetch_weather() {
    if (!wifi_service_is_connected()) return false;

    HTTPClient http;
    http.setTimeout(5000);
    String url = "http://api.open-meteo.com/v1/forecast?latitude=" + 
                 String(current_lat, 4) + "&longitude=" + String(current_lon, 4) + 
                 "&current_weather=true";
    http.begin(url);
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

static void on_city_search_done(bool saved) {
    if (saved && search_city_buffer.length() > 0 && instance) {
        tft.fillRect(10, 100, SCREEN_WIDTH - 20, 60, COLOR_CARD);
        tft.drawRoundRect(10, 100, SCREEN_WIDTH - 20, 60, 4, COLOR_HIGHLIGHT);
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(COLOR_HIGHLIGHT, COLOR_CARD);
        tft.drawString("Buscando coordenadas...", 120, 130, &fonts::Font2);

        bool ok = WeatherApp::geocode_and_set_city(search_city_buffer);
        if (ok) {
            buzzer_notification();
        } else {
            buzzer_beep(800, 200);
        }
    }
    search_city_buffer = "";
    if (instance) {
        instance->on_draw();
    }
}

void WeatherApp::on_enter() {
    instance = this;
    is_selecting_city = false;
    nav_bar_set_action("ACTUALIZAR");
    load_city_config();
    if (!current_weather.valid && wifi_service_is_connected()) {
        fetch_weather();
    }
}

void WeatherApp::on_exit() {
    instance = nullptr;
}

void WeatherApp::on_update() {}

void WeatherApp::draw_city_selector() {
    tft.fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH, CONTENT_HEIGHT, COLOR_BG);

    // Cabecera selector
    tft.fillRect(6, 28, SCREEN_WIDTH - 12, 28, COLOR_CARD);
    tft.drawRoundRect(6, 28, SCREEN_WIDTH - 12, 28, 4, COLOR_CARD_BORDER);

    tft.setTextDatum(ML_DATUM);
    tft.setTextColor(COLOR_ACCENT, COLOR_CARD);
    tft.drawString("SELECCIONAR CIUDAD", 14, 42, &fonts::Font2);

    // Botón Cerrar [X]
    tft.fillRoundRect(SCREEN_WIDTH - 38, 30, 28, 24, 3, COLOR_DANGER);
    tft.setTextColor(COLOR_TEXT, COLOR_DANGER);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("X", SCREEN_WIDTH - 24, 42, &fonts::Font2);

    // Botón BUSCAR CON TECLADO
    tft.fillRoundRect(8, 60, SCREEN_WIDTH - 16, 30, 4, COLOR_CARD);
    tft.drawRoundRect(8, 60, SCREEN_WIDTH - 16, 30, 4, COLOR_ACCENT);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(COLOR_HIGHLIGHT, COLOR_CARD);
    tft.drawString("BUSCAR CON TECLADO...", 120, 75, &fonts::Font2);

    // Parrilla de ciudades frecuentes (2 columnas x 5 filas)
    int16_t start_y = 96;
    int16_t card_w = 108;
    int16_t card_h = 32;
    int16_t gap_y = 6;

    for (int i = 0; i < PRESET_COUNT; ++i) {
        int col = i % 2;
        int row = i / 2;
        int16_t x = 8 + col * (card_w + 8);
        int16_t y = start_y + row * (card_h + gap_y);

        bool is_current = (current_city_name.equalsIgnoreCase(PRESETS[i].name));
        uint16_t bg = is_current ? COLOR_ACCENT : COLOR_CARD;
        uint16_t txt = is_current ? COLOR_BG : COLOR_TEXT;

        tft.fillRoundRect(x, y, card_w, card_h, 4, bg);
        tft.drawRoundRect(x, y, card_w, card_h, 4, is_current ? COLOR_HIGHLIGHT : COLOR_CARD_BORDER);

        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(txt, bg);
        tft.drawString(PRESETS[i].name, x + card_w / 2, y + card_h / 2, &fonts::Font2);
    }
}

void WeatherApp::draw_weather_card() {
    tft.fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH, CONTENT_HEIGHT, COLOR_BG);

    if (!wifi_service_is_connected()) {
        tft.fillRect(10, 40, SCREEN_WIDTH - 20, 160, COLOR_CARD);
        tft.drawRoundRect(10, 40, SCREEN_WIDTH - 20, 160, 6, COLOR_WARNING);

        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(COLOR_WARNING, COLOR_CARD);
        tft.drawString("WiFi Desconectado", 120, 80, &fonts::Font2);

        tft.setTextColor(COLOR_TEXT, COLOR_CARD);
        tft.drawString("Conecta el WiFi en 'AJUSTES'", 120, 110, &fonts::Font2);
        tft.drawString("para ver el pronostico.", 120, 130, &fonts::Font0);

        tft.fillRoundRect(30, 150, SCREEN_WIDTH - 60, 32, 4, COLOR_BTN_DEFAULT);
        tft.drawRoundRect(30, 150, SCREEN_WIDTH - 60, 32, 4, COLOR_CARD_BORDER);
        tft.setTextColor(COLOR_ACCENT, COLOR_BTN_DEFAULT);
        tft.drawString("Abrir Ajustes", 120, 166, &fonts::Font2);
        return;
    }

    const int16_t card_x = 8;
    const int16_t card_y = 27;
    const int16_t card_w = SCREEN_WIDTH - 16; // 224
    const int16_t card_h = 260;

    // 1. Fondo de la tarjeta
    tft.fillRoundRect(card_x, card_y, card_w, card_h, 6, COLOR_CARD);

    // 2. Fila superior: etiqueta "UBICACIÓN" y botón [CIUDAD]
    tft.setTextDatum(ML_DATUM);
    tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
    tft.drawString("UBICACION:", card_x + 10, card_y + 15, &fonts::Font0);

    // Botón [CIUDAD]
    const int16_t btn_city_x = card_x + card_w - 76;
    const int16_t btn_city_y = card_y + 6;
    const int16_t btn_city_w = 68;
    const int16_t btn_city_h = 20;
    tft.fillRoundRect(btn_city_x, btn_city_y, btn_city_w, btn_city_h, 3, COLOR_BTN_DEFAULT);
    tft.drawRoundRect(btn_city_x, btn_city_y, btn_city_w, btn_city_h, 3, COLOR_ACCENT);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(COLOR_ACCENT, COLOR_BTN_DEFAULT);
    tft.drawString("CIUDAD", btn_city_x + btn_city_w / 2, btn_city_y + btn_city_h / 2, &fonts::Font0);

    // 3. Fila de la ciudad (centrada, ancho completo para nombres largos)
    String city_upper = current_city_name;
    city_upper.toUpperCase();
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(COLOR_HIGHLIGHT, COLOR_CARD);
    if (city_upper.length() > 10) {
        tft.drawString(city_upper.c_str(), 120, card_y + 36, &fonts::Font2);
    } else {
        tft.drawString(city_upper.c_str(), 120, card_y + 36, &fonts::Font4);
    }

    tft.drawFastHLine(card_x + 14, card_y + 54, card_w - 28, COLOR_CARD_BORDER);

    // 4. Icono climático gráfico y Temperatura
    int16_t icon_x = card_x + 46;
    int16_t icon_y = card_y + 95;

    if (current_weather.weather_code <= 1) {
        // Sol
        tft.fillCircle(icon_x, icon_y, 20, COLOR_HIGHLIGHT);
        tft.drawCircle(icon_x, icon_y, 24, COLOR_WARNING);
    } else if (current_weather.weather_code <= 3) {
        // Sol con nube
        tft.fillCircle(icon_x - 8, icon_y - 8, 14, COLOR_HIGHLIGHT);
        tft.fillRoundRect(icon_x - 14, icon_y - 2, 38, 20, 10, COLOR_TEXT_MUTED);
    } else {
        // Nube con lluvia
        tft.fillRoundRect(icon_x - 14, icon_y - 6, 38, 20, 10, COLOR_TEXT_MUTED);
        tft.drawLine(icon_x - 8, icon_y + 18, icon_x - 12, icon_y + 26, COLOR_ACCENT);
        tft.drawLine(icon_x, icon_y + 18, icon_x - 4, icon_y + 26, COLOR_ACCENT);
        tft.drawLine(icon_x + 8, icon_y + 18, icon_x + 4, icon_y + 26, COLOR_ACCENT);
    }

    // Gran temperatura
    char temp_str[16];
    snprintf(temp_str, sizeof(temp_str), "%.1f", current_weather.temp);
    tft.setTextDatum(ML_DATUM);
    tft.setTextColor(COLOR_TEXT, COLOR_CARD);
    tft.drawString(temp_str, icon_x + 36, icon_y, &fonts::Font6);

    // Símbolo de grados (°) y C
    int16_t temp_text_w = tft.textWidth(temp_str, &fonts::Font6);
    int16_t deg_x = icon_x + 36 + temp_text_w + 4;
    int16_t deg_y = icon_y - 14;
    tft.drawCircle(deg_x, deg_y, 3, COLOR_TEXT);
    tft.drawCircle(deg_x, deg_y, 2, COLOR_TEXT);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("C", deg_x + 6, deg_y - 3, &fonts::Font4);

    // 5. Condición meteorológica
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(COLOR_HIGHLIGHT, COLOR_CARD);
    tft.drawString(current_weather.condition_text.c_str(), 120, card_y + 142, &fonts::Font4);

    tft.drawFastHLine(card_x + 16, card_y + 166, card_w - 32, COLOR_CARD_BORDER);

    // 6. Viento y estado
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
    char wind_buf[32];
    snprintf(wind_buf, sizeof(wind_buf), "Viento: %.1f km/h", current_weather.windspeed);
    tft.drawString(wind_buf, 120, card_y + 184, &fonts::Font2);

    tft.setTextColor(COLOR_SUCCESS, COLOR_CARD);
    tft.drawString("Pronostico en tiempo real", 120, card_y + 208, &fonts::Font2);

    tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
    tft.drawString("Open-Meteo API Service", 120, card_y + 232, &fonts::Font0);

    // 7. Borde exterior azul: SE DIBUJA AL FINAL PARA CERRAR PERFECTAMENTE
    tft.drawRoundRect(card_x, card_y, card_w, card_h, 6, COLOR_ACCENT);
    tft.drawRoundRect(card_x + 1, card_y + 1, card_w - 2, card_h - 2, 5, COLOR_ACCENT);
}

void WeatherApp::on_draw() {
    if (is_selecting_city) {
        nav_bar_set_action("VOLVER");
        draw_city_selector();
    } else {
        nav_bar_set_action("ACTUALIZAR");
        draw_weather_card();
    }
    nav_bar_draw();
}

bool WeatherApp::on_touch(int16_t tx, int16_t ty, bool is_new_press) {
    if (!is_new_press) return false;

    // Si estamos en el selector de ciudades
    if (is_selecting_city) {
        // Botón Cerrar [X] (SCREEN_WIDTH - 38 a -10, Y: 30 a 54)
        if (tx >= SCREEN_WIDTH - 38 && tx <= SCREEN_WIDTH - 10 && ty >= 30 && ty <= 54) {
            buzzer_click();
            is_selecting_city = false;
            on_draw();
            return true;
        }

        // Botón BUSCAR CON TECLADO (Y: 60 a 90)
        if (tx >= 8 && tx <= SCREEN_WIDTH - 8 && ty >= 60 && ty <= 90) {
            buzzer_click();
            search_city_buffer = "";
            keyboard_open(&search_city_buffer, "Buscar Ciudad", on_city_search_done);
            is_selecting_city = false;
            return true;
        }

        // Toque en parrilla de ciudades predefinidas
        int16_t start_y = 96;
        int16_t card_w = 108;
        int16_t card_h = 32;
        int16_t gap_y = 6;

        for (int i = 0; i < PRESET_COUNT; ++i) {
            int col = i % 2;
            int row = i / 2;
            int16_t x = 8 + col * (card_w + 8);
            int16_t y = start_y + row * (card_h + gap_y);

            if (tx >= x && tx <= x + card_w && ty >= y && ty <= y + card_h) {
                buzzer_notification();
                set_city(PRESETS[i].name, PRESETS[i].lat, PRESETS[i].lon);
                is_selecting_city = false;
                on_draw();
                return true;
            }
        }
        return true;
    }

    // Si estamos en la pantalla principal del tiempo:
    // Botón [CIUDAD] (x: 148 a 230, y: 30 a 58) O toque en el nombre de la ciudad (x: 16 a 224, y: 56 a 86)
    if ((tx >= 148 && tx <= 230 && ty >= 30 && ty <= 58) ||
        (tx >= 16 && tx <= 224 && ty >= 56 && ty <= 86)) {
        buzzer_click();
        is_selecting_city = true;
        on_draw();
        return true;
    }

    // Si no hay WiFi y pulsa "Abrir Ajustes"
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
    if (is_selecting_city) {
        is_selecting_city = false;
        on_draw();
        return;
    }

    if (wifi_service_is_connected()) {
        tft.fillRect(16, 230, SCREEN_WIDTH - 32, 20, COLOR_CARD);
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(COLOR_WARNING, COLOR_CARD);
        tft.drawString("Actualizando...", 120, 240, &fonts::Font0);

        fetch_weather();
        buzzer_notification();
        draw_weather_card();
    } else {
        buzzer_beep(800, 150);
    }
}
