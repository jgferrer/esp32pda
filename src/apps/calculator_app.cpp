#include "apps/calculator_app.h"
#include "hal/display.h"
#include "hal/buzzer.h"
#include "ui/nav_bar.h"

static const char* KEY_LABELS[5][4] = {
    { "C", "+/-", "%", "/" },
    { "7", "8",   "9", "*" },
    { "4", "5",   "6", "-" },
    { "1", "2",   "3", "+" },
    { "0", ".",   "<-","=" }
};

void CalculatorApp::on_enter() {
    current_value = 0;
    operand_a = 0;
    pending_op = '\0';
    start_new_number = true;
    display_buffer = "0";
    nav_bar_set_action(nullptr);
}

void CalculatorApp::on_exit() {}

void CalculatorApp::on_update() {}

void CalculatorApp::update_display() {
    // Pantalla LCD superior
    tft.fillRect(10, 30, SCREEN_WIDTH - 20, 52, COLOR_CARD);
    tft.drawRoundRect(10, 30, SCREEN_WIDTH - 20, 52, 6, COLOR_ACCENT);

    // Operación pendiente en pequeño arriba a la izquierda
    if (pending_op != '\0') {
        char op_str[4] = { pending_op, '\0' };
        tft.setTextColor(COLOR_HIGHLIGHT, COLOR_CARD);
        tft.setTextDatum(TL_DATUM);
        tft.drawString(op_str, 18, 34, &fonts::Font2);
    }

    // Número actual en grande alineado a la derecha
    tft.setTextDatum(MR_DATUM);
    tft.setTextColor(COLOR_TEXT, COLOR_CARD);
    tft.drawString(display_buffer.c_str(), SCREEN_WIDTH - 20, 58, &fonts::Font4);
}

void CalculatorApp::on_draw() {
    tft.fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH, CONTENT_HEIGHT, COLOR_BG);
    update_display();

    // Dibujar botonera 4x5
    int16_t start_x = 10;
    int16_t start_y = 88;
    int16_t btn_w = 52;
    int16_t btn_h = 36;
    int16_t gap = 4;

    for (int row = 0; row < 5; ++row) {
        for (int col = 0; col < 4; ++col) {
            int16_t x = start_x + col * (btn_w + gap);
            int16_t y = start_y + row * (btn_h + gap);
            const char* label = KEY_LABELS[row][col];

            uint16_t bg = COLOR_BTN_DEFAULT;
            uint16_t txt = COLOR_TEXT;

            if (col == 3) {
                // Columna de operadores principales
                bg = COLOR_ACCENT;
                txt = COLOR_BG;
            } else if (row == 0) {
                // Fila de control (C, +/-, %)
                bg = COLOR_CARD;
                txt = COLOR_WARNING;
            } else if (row == 4 && col == 3) {
                // Igual
                bg = COLOR_SUCCESS;
                txt = COLOR_TEXT;
            }

            tft.fillRoundRect(x, y, btn_w, btn_h, 4, bg);
            tft.drawRoundRect(x, y, btn_w, btn_h, 4, COLOR_CARD_BORDER);

            tft.setTextDatum(MC_DATUM);
            tft.setTextColor(txt, bg);
            tft.drawString(label, x + btn_w / 2, y + btn_h / 2, &fonts::Font2);
        }
    }
}

void CalculatorApp::calculate_result() {
    double b = display_buffer.toDouble();
    double res = 0;
    switch (pending_op) {
        case '+': res = operand_a + b; break;
        case '-': res = operand_a - b; break;
        case '*': res = operand_a * b; break;
        case '/': res = (b != 0) ? (operand_a / b) : 0; break;
        default:  res = b; break;
    }

    // Formatear resultado
    char buf[32];
    if (res == (long)res) {
        snprintf(buf, sizeof(buf), "%ld", (long)res);
    } else {
        snprintf(buf, sizeof(buf), "%.4f", res);
        // Quitar ceros a la derecha
        int len = strlen(buf);
        while (len > 1 && buf[len - 1] == '0') {
            buf[len - 1] = '\0';
            len--;
        }
        if (len > 1 && buf[len - 1] == '.') {
            buf[len - 1] = '\0';
        }
    }
    display_buffer = String(buf);
    pending_op = '\0';
    start_new_number = true;
}

bool CalculatorApp::on_touch(int16_t tx, int16_t ty, bool is_new_press) {
    if (!is_new_press) return false;

    int16_t start_x = 10;
    int16_t start_y = 88;
    int16_t btn_w = 52;
    int16_t btn_h = 36;
    int16_t gap = 4;

    for (int row = 0; row < 5; ++row) {
        for (int col = 0; col < 4; ++col) {
            int16_t x = start_x + col * (btn_w + gap);
            int16_t y = start_y + row * (btn_h + gap);

            if (tx >= x && tx <= (x + btn_w) && ty >= y && ty <= (y + btn_h)) {
                buzzer_click();
                const char* label = KEY_LABELS[row][col];

                if (strcmp(label, "C") == 0) {
                    display_buffer = "0";
                    operand_a = 0;
                    pending_op = '\0';
                    start_new_number = true;
                } else if (strcmp(label, "<-") == 0) {
                    if (display_buffer.length() > 1) {
                        display_buffer.remove(display_buffer.length() - 1);
                    } else {
                        display_buffer = "0";
                        start_new_number = true;
                    }
                } else if (strcmp(label, "+/-") == 0) {
                    if (display_buffer.startsWith("-")) {
                        display_buffer = display_buffer.substring(1);
                    } else if (display_buffer != "0") {
                        display_buffer = "-" + display_buffer;
                    }
                } else if (strcmp(label, "%") == 0) {
                    double val = display_buffer.toDouble() / 100.0;
                    display_buffer = String(val, 4);
                } else if (strcmp(label, "=") == 0) {
                    calculate_result();
                } else if (strcmp(label, "+") == 0 || strcmp(label, "-") == 0 ||
                           strcmp(label, "*") == 0 || strcmp(label, "/") == 0) {
                    if (pending_op != '\0' && !start_new_number) {
                        calculate_result();
                    }
                    operand_a = display_buffer.toDouble();
                    pending_op = label[0];
                    start_new_number = true;
                } else if (strcmp(label, ".") == 0) {
                    if (display_buffer.indexOf('.') == -1) {
                        display_buffer += ".";
                        start_new_number = false;
                    }
                } else {
                    // Dígito 0-9
                    if (start_new_number || display_buffer == "0") {
                        display_buffer = String(label);
                        start_new_number = false;
                    } else if (display_buffer.length() < 10) {
                        display_buffer += label;
                    }
                }

                update_display();
                return true;
            }
        }
    }

    return false;
}
