#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ==========================================
// PANTALLA TFT 2.8" (ESP32-2432S028 CYD)
// ==========================================
#define SCREEN_WIDTH    240
#define SCREEN_HEIGHT   320
#define SCREEN_ROTATION 0 // Vertical (Retrato)

// Pines TFT (Bus HSPI)
#define TFT_MISO_PIN   12
#define TFT_MOSI_PIN   13
#define TFT_SCLK_PIN   14
#define TFT_CS_PIN     15
#define TFT_DC_PIN      2
#define TFT_RST_PIN    -1
#define TFT_BL_PIN     21

// Pines Panel Táctil Resistivo XPT2046 (Software SPI)
#define TOUCH_MOSI_PIN 32
#define TOUCH_MISO_PIN 39
#define TOUCH_CLK_PIN  25
#define TOUCH_CS_PIN   33
#define TOUCH_IRQ_PIN  36

// ==========================================
// PERIFÉRICOS
// ==========================================
// LED RGB Integrado (Active-LOW: 0=ON, 1=OFF)
#define LED_RED_PIN    4
#define LED_GREEN_PIN  16
#define LED_BLUE_PIN   17

// Audio Buzzer (PWM)
#define BUZZER_PIN     26
#define BUZZER_CHANNEL  0

// Sensor LDR (Luz ambiental)
#define LDR_PIN        34

// ==========================================
// GEOMETRÍA DE LA INTERFAZ (UI LAYOUT)
// ==========================================
#define STATUS_BAR_HEIGHT   24
#define NAV_BAR_HEIGHT      28
#define CONTENT_Y_START     STATUS_BAR_HEIGHT
#define CONTENT_HEIGHT      (SCREEN_HEIGHT - STATUS_BAR_HEIGHT - NAV_BAR_HEIGHT) // 268px

// ==========================================
// PALETA DE COLORES (RGB565) - ESTILO RETRO-PDA MODERNO
// ==========================================
#define COLOR_BG            0x10A2  // Fondo oscuro elegante
#define COLOR_CARD          0x18E3  // Superficie de tarjetas / contenedores
#define COLOR_CARD_BORDER   0x3186  // Bordes sutiles
#define COLOR_HEADER        0x0861  // Fondo de cabeceras
#define COLOR_NAV           0x0841  // Barra inferior
#define COLOR_TEXT          0xFFFF  // Texto principal (blanco)
#define COLOR_TEXT_MUTED    0x9CD3  // Texto secundario / gris suave
#define COLOR_ACCENT        0x05BF  // Acento cyan vibrante
#define COLOR_WARNING       0xFD00  // Acento naranja / aviso
#define COLOR_SUCCESS       0x3666  // Verde esmeralda
#define COLOR_DANGER        0xF986  // Rojo suave
#define COLOR_BTN_DEFAULT   0x2124  // Botón estado normal
#define COLOR_BTN_PRESSED   0x4A69  // Botón estado pulsado
#define COLOR_HIGHLIGHT     0xFDE0  // Amarillo dorado para selecciones

// ==========================================
// IDENTIFICADORES DE APLICACIONES (APP ID)
// ==========================================
enum AppID : uint8_t {
    APP_LAUNCHER = 0,
    APP_CALENDAR,
    APP_NOTES,
    APP_CLOCK,
    APP_CALCULATOR,
    APP_WEATHER,
    APP_SETTINGS,
    APP_COUNT
};

#endif // CONFIG_H
