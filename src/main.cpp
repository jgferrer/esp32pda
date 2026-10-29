#include <Arduino.h>
#include "config.h"
#include "hal/display.h"
#include "hal/buzzer.h"
#include "hal/led.h"
#include "hal/storage.h"
#include "hal/power.h"
#include "services/wifi_service.h"
#include "services/time_service.h"
#include "ui/status_bar.h"
#include "ui/nav_bar.h"
#include "ui/keyboard.h"
#include "kernel/app_manager.h"

// Aplicaciones
#include "apps/launcher_app.h"
#include "apps/calendar_app.h"
#include "apps/notes_app.h"
#include "apps/clock_app.h"
#include "apps/calculator_app.h"
#include "apps/weather_app.h"
#include "apps/settings_app.h"

static LauncherApp app_launcher;
static CalendarApp app_calendar;
static NotesApp app_notes;
static ClockApp app_clock;
static CalculatorApp app_calculator;
static WeatherApp app_weather;
static SettingsApp app_settings;

static bool was_touching = false;

static void show_splash_screen() {
    tft.fillScreen(COLOR_BG);

    // Caja central de bienvenida
    tft.fillRoundRect(20, 90, SCREEN_WIDTH - 40, 140, 8, COLOR_CARD);
    tft.drawRoundRect(20, 90, SCREEN_WIDTH - 40, 140, 8, COLOR_ACCENT);

    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(COLOR_HIGHLIGHT, COLOR_CARD);
    tft.drawString("ESP32 - PDA", 120, 125, &fonts::Font4);

    tft.setTextColor(COLOR_TEXT, COLOR_CARD);
    tft.drawString("PocketOS Personal Assistant", 120, 155, &fonts::Font2);

    tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
    tft.drawString("Iniciando servicios...", 120, 195, &fonts::Font0);

    buzzer_notification();
    delay(1000);
}

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println("\n[ESP32-PDA] Iniciando PocketOS...");

    // Inicialización del Hardware (HAL)
    led_init();
    buzzer_init();
    storage_init();
    display_init();
    power_init();

    // Servicios del Sistema
    wifi_service_init();
    time_service_init();

    // Pantalla de arranque retro
    show_splash_screen();

    // Interfaz de Usuario
    keyboard_init();
    status_bar_init();
    nav_bar_init();

    // Gestor de Aplicaciones (Kernel)
    app_manager_init();
    app_manager_register_app(APP_LAUNCHER,   &app_launcher);
    app_manager_register_app(APP_CALENDAR,   &app_calendar);
    app_manager_register_app(APP_NOTES,      &app_notes);
    app_manager_register_app(APP_CLOCK,      &app_clock);
    app_manager_register_app(APP_CALCULATOR, &app_calculator);
    app_manager_register_app(APP_WEATHER,    &app_weather);
    app_manager_register_app(APP_SETTINGS,   &app_settings);

    // Cargar menú principal
    app_manager_switch_to(APP_LAUNCHER);
    Serial.println("[ESP32-PDA] Sistema listo y operativo.");
}

void loop() {
    // 1. Tareas de fondo y servicios
    power_update();
    wifi_service_update();
    time_service_update();

    // 2. Gestión táctil
    int16_t tx = 0, ty = 0;
    bool is_touching = display_get_touch(&tx, &ty);

    if (is_touching) {
        if (power_is_screen_dimmed()) {
            power_wake_up();
            was_touching = true;
            delay(100);
        } else {
            bool is_new_press = !was_touching;
            app_manager_handle_touch(tx, ty, is_new_press);
            was_touching = true;
        }
    } else {
        was_touching = false;
    }

    // 3. Actualización de pantalla y ciclo de vida de apps
    app_manager_update();

    delay(5);
}
