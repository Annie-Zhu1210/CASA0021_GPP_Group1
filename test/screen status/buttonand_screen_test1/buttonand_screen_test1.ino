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

// ---------- KY-040 pins ----------
static constexpr int ENC_CLK = 16;
static constexpr int ENC_DT  = 17;
static constexpr int ENC_SW  = 18;

// RD must be tied to 3.3V (write-only), so we set pin_rd = -1.

class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ILI9488   _panel;   
  lgfx::Bus_Parallel8   _bus;

public:
  LGFX() {
    // Bus config
    {
      auto cfg = _bus.config();
      cfg.port = 0;
      cfg.freq_write = 6000000;   
      cfg.pin_wr = TFT_WR;
      cfg.pin_rd = -1;           
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

    // Panel config
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

// ---------- Encoder handling ----------
volatile int32_t encDelta = 0;
volatile uint32_t lastEdgeUs = 0;

void IRAM_ATTR onEncClkChange() {
  // debounce edges (mechanical encoder)
  uint32_t now = micros();
  if (now - lastEdgeUs < 800) return;  // 0.8ms
  lastEdgeUs = now;

  // direction from DT at CLK edge
  int dt = digitalRead(ENC_DT);
  encDelta += (dt == LOW) ? +1 : -1;
}

bool buttonPressedEvent() {
  // debounced "pressed" event (active low)
  static uint32_t lastMs = 0;
  static bool lastStable = HIGH;
  static bool lastRead = HIGH;

  bool r = digitalRead(ENC_SW);
  uint32_t now = millis();

  if (r != lastRead) { lastMs = now; lastRead = r; }
  if (now - lastMs > 25) {
    if (r != lastStable) {
      lastStable = r;
      if (lastStable == LOW) return true; // pressed
    }
  }
  return false;
}

// ---------- Demo behavior ----------
uint16_t palette[] = {
  TFT_BLACK, TFT_RED, TFT_GREEN, TFT_BLUE, TFT_YELLOW, TFT_CYAN, TFT_MAGENTA, TFT_WHITE
};
int colorIndex = 1;
bool autoCycle = false;
uint32_t lastAutoMs = 0;

void showColor() {
  tft.fillScreen(palette[colorIndex]);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, palette[colorIndex]);
  tft.setCursor(10, 10);
  tft.printf("Color index: %d\n", colorIndex);
  tft.printf("Auto cycle: %s", autoCycle ? "ON" : "OFF");
}

void setup() {
  // TFT
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);

  // Encoder pins
  pinMode(ENC_CLK, INPUT_PULLUP);
  pinMode(ENC_DT,  INPUT_PULLUP);
  pinMode(ENC_SW,  INPUT_PULLUP);

  // Interrupt on CLK changes (works well on ESP32)
  attachInterrupt(digitalPinToInterrupt(ENC_CLK), onEncClkChange, CHANGE);

  showColor();
}

void loop() {
  // consume encoder delta safely
  int32_t d;
  noInterrupts();
  d = encDelta;
  encDelta = 0;
  interrupts();

  if (d != 0) {
    colorIndex += (d > 0) ? 1 : -1;
    int n = (int)(sizeof(palette) / sizeof(palette[0]));
    if (colorIndex < 0) colorIndex = n - 1;
    if (colorIndex >= n) colorIndex = 0;
    showColor();
  }

  if (buttonPressedEvent()) {
    autoCycle = !autoCycle;
    showColor();
  }

  if (autoCycle && millis() - lastAutoMs > 700) {
    lastAutoMs = millis();
    colorIndex = (colorIndex + 1) % (int)(sizeof(palette) / sizeof(palette[0]));
    showColor();
  }

  delay(5);
}

