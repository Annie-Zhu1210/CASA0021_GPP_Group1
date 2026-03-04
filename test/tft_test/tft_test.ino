#include <LovyanGFX.hpp>

// ---------- Your 8-bit parallel bus pins ----------
static constexpr int TFT_D0  = 1;
static constexpr int TFT_D1  = 2;
static constexpr int TFT_D2  = 3;
static constexpr int TFT_D3  = 4;
static constexpr int TFT_D4  = 5;
static constexpr int TFT_D5  = 6;
static constexpr int TFT_D6  = 7;
static constexpr int TFT_D7  = 15;

static constexpr int TFT_CS  = 10;
static constexpr int TFT_DC  = 9;   // RS/DC
static constexpr int TFT_WR  = 8;
static constexpr int TFT_RST = 14;

// RD must be tied to 3.3V (write-only), so we set pin_rd = -1.

class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ILI9488   _panel;   // try ILI9488 first
  lgfx::Bus_Parallel8   _bus;

public:
  LGFX() {
    // Bus config
    {
      auto cfg = _bus.config();
      cfg.port = 0;
      cfg.freq_write = 6000000;   // safe on breadboard
      cfg.pin_wr = TFT_WR;
      cfg.pin_rd = -1;            // write-only
      cfg.pin_rs = TFT_DC;

      cfg.pin_d0 = TFT_D0;
      cfg.pin_d1 = TFT_D1;
      cfg.pin_d2 = TFT_D2;
      cfg.pin_d3 = TFT_D3;
      cfg.pin_d4 = TFT_D4;
      cfg.pin_d5 = TFT_D5;
      cfg.pin_d6 = TFT_D6;
      cfg.pin_d7 = TFT_D7;

      _bus.config(cfg);
      _panel.setBus(&_bus);
    }

    // Panel config (no cfg.rotation here)
    {
      auto cfg = _panel.config();
      cfg.pin_cs  = TFT_CS;
      cfg.pin_rst = TFT_RST;
      cfg.pin_busy = -1;

      cfg.memory_width  = 320;
      cfg.memory_height = 480;
      cfg.panel_width   = 320;
      cfg.panel_height  = 480;

      cfg.offset_x = 0;
      cfg.offset_y = 0;

      cfg.invert = false;
      cfg.rgb_order = false; // flip to true if colors swapped

      _panel.config(cfg);
    }

    setPanel(&_panel);
  }
};

LGFX tft;

void setup() {
  tft.init();
  tft.setRotation(0);     // 0-3 later
  tft.fillScreen(TFT_BLACK);
  delay(600);
}

void loop() {
  tft.fillScreen(TFT_BLACK); delay(600);
  tft.fillScreen(TFT_RED);   delay(600);
  tft.fillScreen(TFT_GREEN); delay(600);
  tft.fillScreen(TFT_BLUE);  delay(600);
}

