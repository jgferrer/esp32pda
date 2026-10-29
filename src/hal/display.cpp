#include "hal/display.h"

static uint8_t current_brightness = 255;

LGFX::LGFX(void) {
    // 1. Bus SPI Pantalla (HSPI)
    {
        auto cfg = _bus_instance.config();
        cfg.spi_host = HSPI_HOST;
        cfg.spi_mode = 0;
        cfg.freq_write = 40000000;
        cfg.freq_read  = 16000000;
        cfg.spi_3wire  = false;
        cfg.dma_channel = SPI_DMA_CH_AUTO;
        cfg.pin_sclk   = TFT_SCLK_PIN;
        cfg.pin_mosi   = TFT_MOSI_PIN;
        cfg.pin_miso   = TFT_MISO_PIN;
        cfg.pin_dc     = TFT_DC_PIN;
        _bus_instance.config(cfg);
        _panel_instance.setBus(&_bus_instance);
    }

    // 2. Controlador de Panel TFT
    {
        auto cfg = _panel_instance.config();
        cfg.pin_cs           = TFT_CS_PIN;
        cfg.pin_rst          = TFT_RST_PIN;
        cfg.pin_busy         = -1;
        cfg.panel_width      = SCREEN_WIDTH;
        cfg.panel_height     = SCREEN_HEIGHT;
        cfg.offset_x         = 0;
        cfg.offset_y         = 0;
        cfg.offset_rotation  = SCREEN_ROTATION;
        cfg.dummy_read_pixel = 8;
        cfg.dummy_read_bits  = 1;
        cfg.readable         = false;
        cfg.invert           = false;
        cfg.rgb_order        = false;
        cfg.dlen_16bit       = false;
        cfg.bus_shared       = false;
        _panel_instance.config(cfg);
    }

    // 3. Retroiluminación PWM
    {
        auto cfg = _light_instance.config();
        cfg.pin_bl      = TFT_BL_PIN;
        cfg.invert      = false;
        cfg.freq        = 44100;
        cfg.pwm_channel = 7;
        _light_instance.config(cfg);
        _panel_instance.setLight(&_light_instance);
    }

    // 4. Panel Táctil Resistivo XPT2046 (Software SPI)
    {
        auto cfg = _touch_instance.config();
        cfg.pin_cs     = TOUCH_CS_PIN;
        cfg.pin_sclk   = TOUCH_CLK_PIN;
        cfg.pin_mosi   = TOUCH_MOSI_PIN;
        cfg.pin_miso   = TOUCH_MISO_PIN;
        cfg.pin_int    = -1;
        cfg.spi_host   = -1; // Software SPI
        cfg.bus_shared = false;
        cfg.freq       = 1000000;
        cfg.x_min      = 300;
        cfg.x_max      = 3900;
        cfg.y_min      = 3700;
        cfg.y_max      = 200;
        cfg.offset_rotation = 2; // Orientación vertical alineada con panel
        _touch_instance.config(cfg);
        _panel_instance.setTouch(&_touch_instance);
    }

    setPanel(&_panel_instance);
}

LGFX tft;

void display_init() {
    tft.init();
    tft.setRotation(SCREEN_ROTATION);
    display_set_brightness(220); // Brillo por defecto confortable
    tft.fillScreen(COLOR_BG);
    tft.setTextWrap(false);
}

bool display_get_touch(int16_t* x, int16_t* y) {
    uint16_t tx, ty;
    if (tft.getTouch(&tx, &ty)) {
        if (x) *x = (int16_t)tx;
        if (y) *y = (int16_t)ty;
        return true;
    }
    return false;
}

void display_set_brightness(uint8_t brightness) {
    current_brightness = brightness;
    tft.setBrightness(brightness);
}

uint8_t display_get_brightness() {
    return current_brightness;
}
