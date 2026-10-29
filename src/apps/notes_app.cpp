#include "apps/notes_app.h"
#include "hal/display.h"
#include "hal/buzzer.h"
#include "hal/storage.h"
#include "services/time_service.h"
#include "ui/nav_bar.h"
#include "ui/keyboard.h"
#include <ArduinoJson.h>

static const char* NOTES_FILE = "/notes.json";
static std::vector<NoteItem> notes_list;
static NotesApp* instance = nullptr;
static String edit_buffer = "";
static bool is_editing_new = false;

void NotesApp::load_notes() {
    notes_list.clear();
    if (!storage_file_exists(NOTES_FILE)) return;

    String json = storage_read_file(NOTES_FILE);
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, json);
    if (err) return;

    JsonArray arr = doc.as<JsonArray>();
    for (JsonObject obj : arr) {
        NoteItem item;
        item.id = obj["id"] | (uint32_t)random(100000);
        item.title = obj["title"] | "";
        item.content = obj["content"] | "";
        item.date_str = obj["date"] | "";
        if (item.content.length() > 0 || item.title.length() > 0) {
            notes_list.push_back(item);
        }
    }
}

void NotesApp::save_notes() {
    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();
    for (const auto& item : notes_list) {
        JsonObject obj = arr.add<JsonObject>();
        obj["id"] = item.id;
        obj["title"] = item.title;
        obj["content"] = item.content;
        obj["date"] = item.date_str;
    }
    String out;
    serializeJson(doc, out);
    storage_write_file(NOTES_FILE, out);
}

static void on_note_saved(bool saved) {
    if (saved && edit_buffer.length() > 0 && instance) {
        if (is_editing_new) {
            NoteItem item;
            item.id = millis();
            item.content = edit_buffer;
            int nl = edit_buffer.indexOf(' ');
            if (nl > 0 && nl < 16) {
                item.title = edit_buffer.substring(0, nl);
            } else {
                item.title = edit_buffer.substring(0, min(14, (int)edit_buffer.length()));
            }
            item.date_str = time_service_get_date_str();
            notes_list.insert(notes_list.begin(), item);
        }
        NotesApp::save_notes();
    }
    edit_buffer = "";
    if (instance) instance->on_draw();
}

void NotesApp::on_enter() {
    instance = this;
    scroll_offset = 0;
    viewing_note_idx = -1;
    nav_bar_set_action("+ NOTA");
    load_notes();
}

void NotesApp::on_exit() {
    instance = nullptr;
}

void NotesApp::on_update() {}

void NotesApp::draw_list_view() {
    tft.fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH, CONTENT_HEIGHT, COLOR_BG);

    // Cabecera resumen
    tft.fillRect(6, 28, SCREEN_WIDTH - 12, 22, COLOR_CARD);
    tft.drawRoundRect(6, 28, SCREEN_WIDTH - 12, 22, 3, COLOR_CARD_BORDER);
    tft.setTextColor(COLOR_ACCENT, COLOR_CARD);
    tft.setTextDatum(ML_DATUM);
    char hdr[32];
    snprintf(hdr, sizeof(hdr), "MIS NOTAS (%d)", (int)notes_list.size());
    tft.drawString(hdr, 12, 39, &fonts::Font2);

    if (notes_list.empty()) {
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(COLOR_TEXT_MUTED, COLOR_BG);
        tft.drawString("No hay notas guardadas.", 120, 130, &fonts::Font2);
        tft.drawString("Pulsa '+ NOTA' para crear una.", 120, 150, &fonts::Font0);
        return;
    }

    // Listado de notas
    int16_t item_y = 54;
    int16_t item_h = 42;
    int max_items = 5;

    for (size_t i = scroll_offset; i < notes_list.size() && (int)i < scroll_offset + max_items; ++i) {
        const auto& note = notes_list[i];

        tft.fillRoundRect(6, item_y, SCREEN_WIDTH - 12, item_h, 4, COLOR_CARD);
        tft.drawRoundRect(6, item_y, SCREEN_WIDTH - 12, item_h, 4, COLOR_CARD_BORDER);

        // Barra decorativa
        tft.fillRoundRect(8, item_y + 4, 3, item_h - 8, 1, COLOR_WARNING);

        // Título y fecha
        tft.setTextColor(COLOR_TEXT, COLOR_CARD);
        tft.setTextDatum(TL_DATUM);
        tft.drawString(note.title.c_str(), 16, item_y + 6, &fonts::Font2);

        tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
        tft.setTextDatum(TR_DATUM);
        tft.drawString(note.date_str.c_str(), SCREEN_WIDTH - 14, item_y + 6, &fonts::Font0);

        // Extracto contenido
        tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
        tft.setTextDatum(TL_DATUM);
        String snippet = note.content;
        if (snippet.length() > 32) snippet = snippet.substring(0, 32) + "...";
        tft.drawString(snippet.c_str(), 16, item_y + 24, &fonts::Font0);

        item_y += item_h + 4;
    }
}

void NotesApp::draw_detail_view() {
    if (viewing_note_idx < 0 || viewing_note_idx >= (int)notes_list.size()) {
        viewing_note_idx = -1;
        draw_list_view();
        return;
    }

    const auto& note = notes_list[viewing_note_idx];
    tft.fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH, CONTENT_HEIGHT, COLOR_BG);

    // Caja de detalle
    tft.fillRoundRect(6, 28, SCREEN_WIDTH - 12, CONTENT_HEIGHT - 6, 6, COLOR_CARD);
    tft.drawRoundRect(6, 28, SCREEN_WIDTH - 12, CONTENT_HEIGHT - 6, 6, COLOR_CARD_BORDER);

    // Cabecera detalle
    tft.setTextColor(COLOR_WARNING, COLOR_CARD);
    tft.setTextDatum(TL_DATUM);
    tft.drawString(note.title.c_str(), 12, 34, &fonts::Font2);

    tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
    tft.drawString(note.date_str.c_str(), 12, 52, &fonts::Font0);

    tft.drawFastHLine(12, 66, SCREEN_WIDTH - 24, COLOR_CARD_BORDER);

    // Contenido
    tft.setTextColor(COLOR_TEXT, COLOR_CARD);
    tft.setTextDatum(TL_DATUM);
    tft.drawString(note.content.c_str(), 12, 74, &fonts::Font2);

    // Botón borrar dentro del visor
    tft.fillRoundRect(SCREEN_WIDTH - 76, 32, 64, 22, 3, COLOR_DANGER);
    tft.setTextColor(COLOR_TEXT, COLOR_DANGER);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("BORRAR", SCREEN_WIDTH - 44, 43, &fonts::Font0);
}

void NotesApp::on_draw() {
    if (viewing_note_idx >= 0) {
        nav_bar_set_action("CERRAR");
        draw_detail_view();
    } else {
        nav_bar_set_action("+ NOTA");
        draw_list_view();
    }
    nav_bar_draw();
}

bool NotesApp::on_touch(int16_t tx, int16_t ty, bool is_new_press) {
    if (!is_new_press) return false;

    // Si estamos en vista detalle
    if (viewing_note_idx >= 0) {
        // Botón Borrar
        if (tx >= SCREEN_WIDTH - 76 && tx <= SCREEN_WIDTH - 12 && ty >= 32 && ty <= 54) {
            buzzer_notification();
            notes_list.erase(notes_list.begin() + viewing_note_idx);
            save_notes();
            viewing_note_idx = -1;
            on_draw();
            return true;
        }
        return true;
    }

    // Si estamos en listado de notas
    int16_t item_y = 54;
    int16_t item_h = 42;
    int max_items = 5;

    for (size_t i = scroll_offset; i < notes_list.size() && (int)i < scroll_offset + max_items; ++i) {
        if (tx >= 6 && tx <= SCREEN_WIDTH - 6 && ty >= item_y && ty <= item_y + item_h) {
            buzzer_click();
            viewing_note_idx = i;
            on_draw();
            return true;
        }
        item_y += item_h + 4;
    }

    return false;
}

void NotesApp::on_nav_action() {
    if (viewing_note_idx >= 0) {
        // Botón Cerrar detalle
        buzzer_click();
        viewing_note_idx = -1;
        on_draw();
    } else {
        // Crear nueva nota
        is_editing_new = true;
        edit_buffer = "";
        keyboard_open(&edit_buffer, "Nueva Nota", on_note_saved);
    }
}
