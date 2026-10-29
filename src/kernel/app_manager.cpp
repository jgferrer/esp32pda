#include "kernel/app_manager.h"
#include "hal/display.h"
#include "hal/power.h"
#include "hal/buzzer.h"
#include "ui/status_bar.h"
#include "ui/nav_bar.h"
#include "ui/keyboard.h"

static AppBase* apps[APP_COUNT] = { nullptr };
static AppID current_app_id = APP_LAUNCHER;
static AppID previous_app_id = APP_LAUNCHER;
static bool needs_redraw = true;

void app_manager_init() {
    for (int i = 0; i < APP_COUNT; ++i) {
        apps[i] = nullptr;
    }
    current_app_id = APP_LAUNCHER;
    previous_app_id = APP_LAUNCHER;
    needs_redraw = true;
}

void app_manager_register_app(AppID id, AppBase* app) {
    if (id < APP_COUNT) {
        apps[id] = app;
    }
}

void app_manager_switch_to(AppID id) {
    if (id >= APP_COUNT || apps[id] == nullptr) return;

    if (keyboard_is_active()) {
        keyboard_close();
    }

    if (apps[current_app_id]) {
        apps[current_app_id]->on_exit();
    }

    previous_app_id = current_app_id;
    current_app_id = id;

    // Actualizar barras
    status_bar_set_title(apps[current_app_id]->get_title());
    nav_bar_show_back(current_app_id != APP_LAUNCHER);

    // Limpiar pantalla
    tft.fillScreen(COLOR_BG);
    status_bar_draw(true);
    nav_bar_draw();

    // Iniciar app
    apps[current_app_id]->on_enter();
    apps[current_app_id]->on_draw();
    needs_redraw = false;
}

AppID app_manager_get_current_id() {
    return current_app_id;
}

void app_manager_request_redraw() {
    needs_redraw = true;
}

void app_manager_update() {
    status_bar_update();

    if (apps[current_app_id]) {
        apps[current_app_id]->on_update();
    }

    if (needs_redraw) {
        needs_redraw = false;
        if (apps[current_app_id]) {
            apps[current_app_id]->on_draw();
        }
    }
}

void app_manager_handle_touch(int16_t tx, int16_t ty, bool is_new_press) {
    power_notify_activity();

    // 1. Si el teclado en pantalla está activo, consume todo el toque
    if (keyboard_is_active()) {
        if (is_new_press) {
            keyboard_handle_touch(tx, ty);
        }
        return;
    }

    // 2. Si toca en la barra de navegación inferior
    if (ty >= (SCREEN_HEIGHT - NAV_BAR_HEIGHT)) {
        if (is_new_press) {
            NavEvent ev = nav_bar_handle_touch(tx, ty);
            if (ev == NAV_HOME) {
                app_manager_switch_to(APP_LAUNCHER);
            } else if (ev == NAV_BACK) {
                if (current_app_id != APP_LAUNCHER) {
                    app_manager_switch_to(APP_LAUNCHER);
                }
            } else if (ev == NAV_ACTION) {
                if (apps[current_app_id]) {
                    apps[current_app_id]->on_nav_action();
                }
            }
        }
        return;
    }

    // 3. Si toca en la barra superior (status bar) -> atajo rápido a Ajustes si se pulsa el icono WiFi
    if (ty < STATUS_BAR_HEIGHT) {
        if (is_new_press && tx >= SCREEN_WIDTH - 70 && tx <= SCREEN_WIDTH - 45) {
            buzzer_click();
            app_manager_switch_to(APP_SETTINGS);
        }
        return;
    }

    // 4. Delegar en la aplicación activa
    if (apps[current_app_id]) {
        apps[current_app_id]->on_touch(tx, ty, is_new_press);
    }
}
