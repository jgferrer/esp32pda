#include "apps/calendar_app.h"
#include "hal/display.h"
#include "hal/buzzer.h"
#include "hal/storage.h"
#include "services/time_service.h"
#include "ui/nav_bar.h"
#include "ui/keyboard.h"
#include <ArduinoJson.h>

static const char* EVENTS_FILE = "/events.json";
static std::vector<CalendarEvent> all_events;
static String new_event_buffer = "";
static CalendarApp* instance = nullptr;

static const char* MONTH_NAMES[] = {
    "ENERO", "FEBRERO", "MARZO", "ABRIL", "MAYO", "JUNIO",
    "JULIO", "AGOSTO", "SEPTIEMBRE", "OCTUBRE", "NOVIEMBRE", "DICIEMBRE"
};

static const char* DAY_HEADERS[] = { "L", "M", "X", "J", "V", "S", "D" };

static bool is_leap_year(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

static int get_days_in_month(int year, int month) {
    static const int days_per_month[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if (month == 2 && is_leap_year(year)) return 29;
    if (month >= 1 && month <= 12) return days_per_month[month - 1];
    return 30;
}

// 0 = Domingo, 1 = Lunes, ...
static int get_day_of_week(int y, int m, int d) {
    static int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    if (m < 3) y -= 1;
    return (y + y/4 - y/100 + y/400 + t[m-1] + d) % 7;
}

void CalendarApp::load_events() {
    all_events.clear();
    if (!storage_file_exists(EVENTS_FILE)) return;

    String json = storage_read_file(EVENTS_FILE);
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, json);
    if (err) return;

    JsonArray arr = doc.as<JsonArray>();
    for (JsonObject obj : arr) {
        CalendarEvent ev;
        ev.year = obj["y"] | 2026;
        ev.month = obj["m"] | 1;
        ev.day = obj["d"] | 1;
        ev.hour = obj["h"] | 0;
        ev.minute = obj["min"] | 0;
        ev.title = obj["title"] | "";
        if (ev.title.length() > 0) {
            all_events.push_back(ev);
        }
    }
}

void CalendarApp::save_events() {
    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();
    for (const auto& ev : all_events) {
        JsonObject obj = arr.add<JsonObject>();
        obj["y"] = ev.year;
        obj["m"] = ev.month;
        obj["d"] = ev.day;
        obj["h"] = ev.hour;
        obj["min"] = ev.minute;
        obj["title"] = ev.title;
    }
    String out;
    serializeJson(doc, out);
    storage_write_file(EVENTS_FILE, out);
}

const std::vector<CalendarEvent>& CalendarApp::get_events() {
    return all_events;
}

static void on_event_typed(bool saved) {
    if (saved && new_event_buffer.length() > 0 && instance) {
        CalendarEvent ev;
        struct tm t = time_service_get_tm();
        ev.year = 2026;
        ev.month = 10;
        ev.day = 1;
        ev.hour = t.tm_hour;
        ev.minute = t.tm_min;
        ev.title = new_event_buffer;
        all_events.push_back(ev);
        CalendarApp::save_events();
    }
    new_event_buffer = "";
    if (instance) instance->on_draw();
}

void CalendarApp::on_enter() {
    instance = this;
    nav_bar_set_action("+ EVENTO");

    struct tm t = time_service_get_tm();
    view_year = t.tm_year + 1900;
    view_month = t.tm_mon + 1;
    selected_day = t.tm_mday;

    load_events();
}

void CalendarApp::on_exit() {
    instance = nullptr;
}

void CalendarApp::on_update() {}

bool CalendarApp::has_event_on_day(int day) {
    for (const auto& ev : all_events) {
        if (ev.year == view_year && ev.month == view_month && ev.day == day) {
            return true;
        }
    }
    return false;
}

void CalendarApp::draw_calendar_grid() {
    // Cabecera mes y flechas navegación
    tft.fillRect(4, 28, SCREEN_WIDTH - 8, 24, COLOR_CARD);
    tft.drawRoundRect(4, 28, SCREEN_WIDTH - 8, 24, 4, COLOR_CARD_BORDER);

    // Botón <
    tft.fillRect(6, 30, 24, 20, COLOR_BTN_DEFAULT);
    tft.drawRect(6, 30, 24, 20, COLOR_CARD_BORDER);
    tft.setTextColor(COLOR_TEXT, COLOR_BTN_DEFAULT);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("<", 18, 40, &fonts::Font2);

    // Título Mes y Año
    char title_buf[32];
    snprintf(title_buf, sizeof(title_buf), "%s %d", MONTH_NAMES[view_month - 1], view_year);
    tft.setTextColor(COLOR_HIGHLIGHT, COLOR_CARD);
    tft.drawString(title_buf, 120, 40, &fonts::Font2);

    // Botón >
    tft.fillRect(SCREEN_WIDTH - 30, 30, 24, 20, COLOR_BTN_DEFAULT);
    tft.drawRect(SCREEN_WIDTH - 30, 30, 24, 20, COLOR_CARD_BORDER);
    tft.setTextColor(COLOR_TEXT, COLOR_BTN_DEFAULT);
    tft.drawString(">", SCREEN_WIDTH - 18, 40, &fonts::Font2);

    // Días de la semana cabecera (L M X J V S D)
    int16_t col_w = 32;
    int16_t grid_x = 8;
    int16_t y = 56;
    tft.setTextDatum(MC_DATUM);
    for (int i = 0; i < 7; ++i) {
        tft.setTextColor(i >= 5 ? COLOR_WARNING : COLOR_TEXT_MUTED, COLOR_BG);
        tft.drawString(DAY_HEADERS[i], grid_x + i * col_w + col_w / 2, y + 6, &fonts::Font0);
    }

    // Cuadrícula de días
    y = 68;
    int first_wday = (get_day_of_week(view_year, view_month, 1) + 6) % 7; // 0 = Lunes
    int total_days = get_days_in_month(view_year, view_month);
    int current_day = 1;

    struct tm today = time_service_get_tm();
    bool is_current_month = (today.tm_year + 1900 == view_year && today.tm_mon + 1 == view_month);

    for (int row = 0; row < 6; ++row) {
        for (int col = 0; col < 7; ++col) {
            int16_t cell_x = grid_x + col * col_w;
            int16_t cell_y = y + row * 18;

            if (row == 0 && col < first_wday) {
                continue;
            }
            if (current_day > total_days) {
                break;
            }

            bool is_today = (is_current_month && current_day == today.tm_mday);
            bool is_sel = (current_day == selected_day);
            bool has_ev = has_event_on_day(current_day);

            if (is_sel) {
                tft.fillRoundRect(cell_x + 1, cell_y + 1, col_w - 2, 16, 3, COLOR_ACCENT);
                tft.setTextColor(COLOR_BG, COLOR_ACCENT);
            } else if (is_today) {
                tft.drawRoundRect(cell_x + 1, cell_y + 1, col_w - 2, 16, 3, COLOR_WARNING);
                tft.setTextColor(COLOR_WARNING, COLOR_BG);
            } else {
                tft.setTextColor(COLOR_TEXT, COLOR_BG);
            }

            char day_str[4];
            snprintf(day_str, sizeof(day_str), "%d", current_day);
            tft.drawString(day_str, cell_x + col_w / 2, cell_y + 9, &fonts::Font0);

            // Punto de evento
            if (has_ev) {
                uint16_t dot_color = is_sel ? COLOR_BG : COLOR_SUCCESS;
                tft.fillCircle(cell_x + col_w / 2, cell_y + 15, 1, dot_color);
            }

            current_day++;
        }
        if (current_day > total_days) break;
    }
}

void CalendarApp::draw_event_list() {
    int16_t list_y = 184;
    int16_t list_h = 104;

    tft.fillRect(4, list_y, SCREEN_WIDTH - 8, list_h, COLOR_CARD);
    tft.drawRoundRect(4, list_y, SCREEN_WIDTH - 8, list_h, 4, COLOR_CARD_BORDER);

    // Cabecera del día seleccionado
    char hdr[40];
    snprintf(hdr, sizeof(hdr), "EVENTOS DEL DIA %02d/%02d", selected_day, view_month);
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(COLOR_ACCENT, COLOR_CARD);
    tft.drawString(hdr, 10, list_y + 6, &fonts::Font2);

    // Lista de eventos
    int drawn = 0;
    int item_y = list_y + 26;

    for (size_t i = 0; i < all_events.size(); ++i) {
        const auto& ev = all_events[i];
        if (ev.year == view_year && ev.month == view_month && ev.day == selected_day) {
            if (drawn >= 3) break; // Máximo 3 visibles en este espacio compacto

            tft.fillRect(8, item_y, SCREEN_WIDTH - 16, 22, COLOR_BG);
            tft.drawRect(8, item_y, SCREEN_WIDTH - 16, 22, COLOR_CARD_BORDER);

            tft.setTextColor(COLOR_TEXT, COLOR_BG);
            tft.setTextDatum(ML_DATUM);
            tft.drawString(ev.title.c_str(), 14, item_y + 11, &fonts::Font0);

            // Icono papelera para borrar
            tft.setTextColor(COLOR_DANGER, COLOR_BG);
            tft.setTextDatum(MR_DATUM);
            tft.drawString("[X]", SCREEN_WIDTH - 14, item_y + 11, &fonts::Font0);

            item_y += 24;
            drawn++;
        }
    }

    if (drawn == 0) {
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
        tft.drawString("Sin eventos. Toca '+ EVENTO'", 120, list_y + 55, &fonts::Font0);
    }
}

void CalendarApp::on_draw() {
    tft.fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH, CONTENT_HEIGHT, COLOR_BG);
    draw_calendar_grid();
    draw_event_list();
}

bool CalendarApp::on_touch(int16_t tx, int16_t ty, bool is_new_press) {
    if (!is_new_press) return false;

    // Flecha anterior mes (<)
    if (tx >= 6 && tx <= 30 && ty >= 28 && ty <= 52) {
        buzzer_click();
        view_month--;
        if (view_month < 1) {
            view_month = 12;
            view_year--;
        }
        selected_day = 1;
        on_draw();
        return true;
    }

    // Flecha siguiente mes (>)
    if (tx >= SCREEN_WIDTH - 30 && tx <= SCREEN_WIDTH - 6 && ty >= 28 && ty <= 52) {
        buzzer_click();
        view_month++;
        if (view_month > 12) {
            view_month = 1;
            view_year++;
        }
        selected_day = 1;
        on_draw();
        return true;
    }

    // Toque en cuadrícula de días
    if (ty >= 68 && ty <= 178) {
        int16_t col_w = 32;
        int16_t grid_x = 8;
        int col = (tx - grid_x) / col_w;
        int row = (ty - 68) / 18;

        if (col >= 0 && col < 7 && row >= 0 && row < 6) {
            int first_wday = (get_day_of_week(view_year, view_month, 1) + 6) % 7;
            int day_clicked = (row * 7 + col) - first_wday + 1;
            int total_days = get_days_in_month(view_year, view_month);

            if (day_clicked >= 1 && day_clicked <= total_days) {
                buzzer_click();
                selected_day = day_clicked;
                on_draw();
                return true;
            }
        }
    }

    // Toque en lista de eventos para borrar [X]
    if (ty >= 210 && ty <= 280) {
        int item_idx = (ty - 210) / 24;
        if (tx >= SCREEN_WIDTH - 40 && tx <= SCREEN_WIDTH - 8) {
            int match_count = 0;
            for (auto it = all_events.begin(); it != all_events.end(); ++it) {
                if (it->year == view_year && it->month == view_month && it->day == selected_day) {
                    if (match_count == item_idx) {
                        buzzer_notification();
                        all_events.erase(it);
                        save_events();
                        on_draw();
                        return true;
                    }
                    match_count++;
                }
            }
        }
    }

    return false;
}

void CalendarApp::on_nav_action() {
    new_event_buffer = "";
    keyboard_open(&new_event_buffer, "Nuevo Evento", on_event_typed);
}
