#include "ui/keyboard.h"
#include "hal/display.h"
#include "hal/buzzer.h"
#include "ui/widgets.h"

static bool active = false;
static String* current_target = nullptr;
static String initial_copy = "";
static char dialog_title[32] = "Escribir";
static KeyboardDoneCallback done_cb = nullptr;

static bool mode_symbols = false;
static bool shift_active = false;

// Configuración de dimensiones
static const int16_t KB_Y = 160;
static const int16_t KEY_H = 26;
static const int16_t ROW_GAP = 4;

static const char* KEYS_R1_ALPHA = "qwertyuiop";
static const char* KEYS_R2_ALPHA = "asdfghjkl";
static const char* KEYS_R3_ALPHA = "zxcvbnm";

static const char* KEYS_R1_SYM = "1234567890";
static const char* KEYS_R2_SYM = "-/:;()$&@\"";
static const char* KEYS_R3_SYM = ".,?!'#%*+";

void keyboard_init() {
    active = false;
    current_target = nullptr;
    mode_symbols = false;
    shift_active = false;
}

void keyboard_open(String* target, const char* title, KeyboardDoneCallback on_done) {
    if (!target) return;
    current_target = target;
    initial_copy = *target;
    if (title) {
        strncpy(dialog_title, title, sizeof(dialog_title) - 1);
        dialog_title[sizeof(dialog_title) - 1] = '\0';
    } else {
        strcpy(dialog_title, "Escribir");
    }
    done_cb = on_done;
    active = true;
    mode_symbols = false;
    shift_active = false;
    keyboard_draw();
}

void keyboard_close() {
    active = false;
    current_target = nullptr;
}

bool keyboard_is_active() {
    return active;
}

static void draw_preview_box() {
    // Fondo de la zona de edición
    tft.fillRect(4, 30, SCREEN_WIDTH - 8, 122, COLOR_CARD);
    tft.drawRoundRect(4, 30, SCREEN_WIDTH - 8, 122, 6, COLOR_ACCENT);

    // Título y botón cerrar
    tft.setTextColor(COLOR_ACCENT, COLOR_CARD);
    tft.setTextDatum(TL_DATUM);
    tft.drawString(dialog_title, 12, 36, &fonts::Font2);

    // Botón cancelar "X"
    tft.fillRect(SCREEN_WIDTH - 32, 34, 22, 20, COLOR_DANGER);
    tft.setTextColor(COLOR_TEXT, COLOR_DANGER);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("X", SCREEN_WIDTH - 21, 44, &fonts::Font2);

    // Caja de texto
    tft.fillRect(10, 60, SCREEN_WIDTH - 20, 84, COLOR_BG);
    tft.drawRect(10, 60, SCREEN_WIDTH - 20, 84, COLOR_CARD_BORDER);

    // Texto con cursor
    if (current_target) {
        tft.setTextColor(COLOR_TEXT, COLOR_BG);
        tft.setTextDatum(TL_DATUM);
        String display_text = *current_target + "_";
        tft.drawString(display_text.c_str(), 14, 66, &fonts::Font2);
    }
}

void keyboard_draw() {
    if (!active) return;

    draw_preview_box();

    // Fondo del teclado
    tft.fillRect(0, KB_Y - 4, SCREEN_WIDTH, SCREEN_HEIGHT - KB_Y + 4, COLOR_NAV);
    tft.drawFastHLine(0, KB_Y - 4, SCREEN_WIDTH, COLOR_CARD_BORDER);

    tft.setTextDatum(MC_DATUM);

    // FILA 1 (10 teclas)
    int16_t y = KB_Y;
    const char* r1 = mode_symbols ? KEYS_R1_SYM : KEYS_R1_ALPHA;
    int16_t w_r1 = 22;
    int16_t start_x_r1 = 4;
    for (int i = 0; i < 10; ++i) {
        char ch = r1[i];
        if (!mode_symbols && shift_active) ch = toupper(ch);
        char str[2] = { ch, '\0' };
        int16_t x = start_x_r1 + i * (w_r1 + 2);
        tft.fillRoundRect(x, y, w_r1, KEY_H, 3, COLOR_BTN_DEFAULT);
        tft.drawRoundRect(x, y, w_r1, KEY_H, 3, COLOR_CARD_BORDER);
        tft.setTextColor(COLOR_TEXT, COLOR_BTN_DEFAULT);
        tft.drawString(str, x + w_r1 / 2, y + KEY_H / 2, &fonts::Font2);
    }

    // FILA 2 (9-10 teclas)
    y += KEY_H + ROW_GAP;
    const char* r2 = mode_symbols ? KEYS_R2_SYM : KEYS_R2_ALPHA;
    int len_r2 = strlen(r2);
    int16_t w_r2 = 22;
    int16_t start_x_r2 = (SCREEN_WIDTH - (len_r2 * (w_r2 + 2))) / 2;
    for (int i = 0; i < len_r2; ++i) {
        char ch = r2[i];
        if (!mode_symbols && shift_active) ch = toupper(ch);
        char str[2] = { ch, '\0' };
        int16_t x = start_x_r2 + i * (w_r2 + 2);
        tft.fillRoundRect(x, y, w_r2, KEY_H, 3, COLOR_BTN_DEFAULT);
        tft.drawRoundRect(x, y, w_r2, KEY_H, 3, COLOR_CARD_BORDER);
        tft.setTextColor(COLOR_TEXT, COLOR_BTN_DEFAULT);
        tft.drawString(str, x + w_r2 / 2, y + KEY_H / 2, &fonts::Font2);
    }

    // FILA 3 (Shift/Caps + letras + Backspace)
    y += KEY_H + ROW_GAP;
    // Tecla Shift / Mayus
    int16_t shift_w = 28;
    tft.fillRoundRect(4, y, shift_w, KEY_H, 3, shift_active ? COLOR_ACCENT : COLOR_BTN_DEFAULT);
    tft.drawRoundRect(4, y, shift_w, KEY_H, 3, COLOR_CARD_BORDER);
    tft.setTextColor(shift_active ? COLOR_BG : COLOR_TEXT);
    tft.drawString(shift_active ? "^" : "^", 4 + shift_w / 2, y + KEY_H / 2, &fonts::Font2);

    const char* r3 = mode_symbols ? KEYS_R3_SYM : KEYS_R3_ALPHA;
    int len_r3 = strlen(r3);
    int16_t w_r3 = 21;
    int16_t start_x_r3 = 36;
    for (int i = 0; i < len_r3; ++i) {
        char ch = r3[i];
        if (!mode_symbols && shift_active) ch = toupper(ch);
        char str[2] = { ch, '\0' };
        int16_t x = start_x_r3 + i * (w_r3 + 2);
        tft.fillRoundRect(x, y, w_r3, KEY_H, 3, COLOR_BTN_DEFAULT);
        tft.drawRoundRect(x, y, w_r3, KEY_H, 3, COLOR_CARD_BORDER);
        tft.setTextColor(COLOR_TEXT, COLOR_BTN_DEFAULT);
        tft.drawString(str, x + w_r3 / 2, y + KEY_H / 2, &fonts::Font2);
    }

    // Tecla Backspace (Borrar)
    int16_t bksp_w = 34;
    int16_t bksp_x = SCREEN_WIDTH - bksp_w - 4;
    tft.fillRoundRect(bksp_x, y, bksp_w, KEY_H, 3, COLOR_BTN_DEFAULT);
    tft.drawRoundRect(bksp_x, y, bksp_w, KEY_H, 3, COLOR_CARD_BORDER);
    tft.setTextColor(COLOR_DANGER, COLOR_BTN_DEFAULT);
    tft.drawString("<-", bksp_x + bksp_w / 2, y + KEY_H / 2, &fonts::Font2);

    // FILA 4 (Modo 123/ABC + Espacio + OK)
    y += KEY_H + ROW_GAP;
    // Botón 123/ABC
    int16_t mode_w = 46;
    tft.fillRoundRect(4, y, mode_w, KEY_H, 3, COLOR_BTN_DEFAULT);
    tft.drawRoundRect(4, y, mode_w, KEY_H, 3, COLOR_CARD_BORDER);
    tft.setTextColor(COLOR_ACCENT, COLOR_BTN_DEFAULT);
    tft.drawString(mode_symbols ? "ABC" : "?123", 4 + mode_w / 2, y + KEY_H / 2, &fonts::Font2);

    // Espacio
    int16_t spc_x = 54;
    int16_t spc_w = 120;
    tft.fillRoundRect(spc_x, y, spc_w, KEY_H, 3, COLOR_BTN_DEFAULT);
    tft.drawRoundRect(spc_x, y, spc_w, KEY_H, 3, COLOR_CARD_BORDER);
    tft.setTextColor(COLOR_TEXT_MUTED, COLOR_BTN_DEFAULT);
    tft.drawString("ESPACIO", spc_x + spc_w / 2, y + KEY_H / 2, &fonts::Font0);

    // Botón OK / Guardar
    int16_t ok_x = SCREEN_WIDTH - 58;
    int16_t ok_w = 54;
    tft.fillRoundRect(ok_x, y, ok_w, KEY_H, 3, COLOR_SUCCESS);
    tft.drawRoundRect(ok_x, y, ok_w, KEY_H, 3, COLOR_CARD_BORDER);
    tft.setTextColor(COLOR_TEXT, COLOR_SUCCESS);
    tft.drawString("OK", ok_x + ok_w / 2, y + KEY_H / 2, &fonts::Font2);
}

bool keyboard_handle_touch(int16_t tx, int16_t ty) {
    if (!active) return false;

    // Botón Cancelar (X)
    if (tx >= SCREEN_WIDTH - 36 && tx <= SCREEN_WIDTH - 10 && ty >= 30 && ty <= 56) {
        buzzer_click();
        if (current_target) *current_target = initial_copy;
        keyboard_close();
        if (done_cb) done_cb(false);
        return true;
    }

    if (ty < KB_Y) return true; // Toco la zona superior pero no la X

    // FILA 1
    int16_t y = KB_Y;
    if (ty >= y && ty <= y + KEY_H) {
        int16_t w_r1 = 22;
        int16_t start_x_r1 = 4;
        for (int i = 0; i < 10; ++i) {
            int16_t x = start_x_r1 + i * (w_r1 + 2);
            if (tx >= x && tx <= x + w_r1) {
                buzzer_click();
                const char* r1 = mode_symbols ? KEYS_R1_SYM : KEYS_R1_ALPHA;
                char ch = r1[i];
                if (!mode_symbols && shift_active) ch = toupper(ch);
                if (current_target && current_target->length() < 120) {
                    *current_target += ch;
                    draw_preview_box();
                }
                return true;
            }
        }
    }

    // FILA 2
    y += KEY_H + ROW_GAP;
    if (ty >= y && ty <= y + KEY_H) {
        const char* r2 = mode_symbols ? KEYS_R2_SYM : KEYS_R2_ALPHA;
        int len_r2 = strlen(r2);
        int16_t w_r2 = 22;
        int16_t start_x_r2 = (SCREEN_WIDTH - (len_r2 * (w_r2 + 2))) / 2;
        for (int i = 0; i < len_r2; ++i) {
            int16_t x = start_x_r2 + i * (w_r2 + 2);
            if (tx >= x && tx <= x + w_r2) {
                buzzer_click();
                char ch = r2[i];
                if (!mode_symbols && shift_active) ch = toupper(ch);
                if (current_target && current_target->length() < 120) {
                    *current_target += ch;
                    draw_preview_box();
                }
                return true;
            }
        }
    }

    // FILA 3
    y += KEY_H + ROW_GAP;
    if (ty >= y && ty <= y + KEY_H) {
        // Shift
        if (tx >= 4 && tx <= 32) {
            buzzer_click();
            shift_active = !shift_active;
            keyboard_draw();
            return true;
        }

        // Backspace
        int16_t bksp_w = 34;
        int16_t bksp_x = SCREEN_WIDTH - bksp_w - 4;
        if (tx >= bksp_x && tx <= SCREEN_WIDTH - 4) {
            buzzer_click();
            if (current_target && current_target->length() > 0) {
                current_target->remove(current_target->length() - 1);
                draw_preview_box();
            }
            return true;
        }

        // Letras Fila 3
        const char* r3 = mode_symbols ? KEYS_R3_SYM : KEYS_R3_ALPHA;
        int len_r3 = strlen(r3);
        int16_t w_r3 = 21;
        int16_t start_x_r3 = 36;
        for (int i = 0; i < len_r3; ++i) {
            int16_t x = start_x_r3 + i * (w_r3 + 2);
            if (tx >= x && tx <= x + w_r3) {
                buzzer_click();
                char ch = r3[i];
                if (!mode_symbols && shift_active) ch = toupper(ch);
                if (current_target && current_target->length() < 120) {
                    *current_target += ch;
                    draw_preview_box();
                }
                return true;
            }
        }
    }

    // FILA 4
    y += KEY_H + ROW_GAP;
    if (ty >= y && ty <= y + KEY_H) {
        // Modo 123/ABC
        if (tx >= 4 && tx <= 50) {
            buzzer_click();
            mode_symbols = !mode_symbols;
            keyboard_draw();
            return true;
        }

        // Espacio
        int16_t spc_x = 54;
        int16_t spc_w = 120;
        if (tx >= spc_x && tx <= spc_x + spc_w) {
            buzzer_click();
            if (current_target && current_target->length() < 120) {
                *current_target += ' ';
                draw_preview_box();
            }
            return true;
        }

        // OK
        int16_t ok_x = SCREEN_WIDTH - 58;
        if (tx >= ok_x && tx <= SCREEN_WIDTH - 4) {
            buzzer_notification();
            keyboard_close();
            if (done_cb) done_cb(true);
            return true;
        }
    }

    return true;
}
