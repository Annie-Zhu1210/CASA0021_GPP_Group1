#include <LovyanGFX.hpp>

/* ================= TFT PIN CONFIG ================= */
static constexpr int TFT_D0  = 1;
static constexpr int TFT_D1  = 2;
static constexpr int TFT_D2  = 3;
static constexpr int TFT_D3  = 4;
static constexpr int TFT_D4  = 5;
static constexpr int TFT_D5  = 6;
static constexpr int TFT_D6  = 7;
static constexpr int TFT_D7  = 15;

static constexpr int TFT_CS  = 10;
static constexpr int TFT_DC  = 9;
static constexpr int TFT_WR  = 8;
static constexpr int TFT_RST = 14;

/* ================= KY-040 BUTTON ================= */

static constexpr int ENC_SW = 17;

/* ================= TFT CLASS ================= */
class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ILI9488  _panel;
  lgfx::Bus_Parallel8  _bus;

public:
  LGFX() {
    { // Bus
      auto cfg = _bus.config();
      cfg.port = 0;
      cfg.freq_write = 2000000;
      cfg.pin_wr = TFT_WR;
      cfg.pin_rd = -1;
      cfg.pin_rs = TFT_DC;

      cfg.pin_d0 = TFT_D0; cfg.pin_d1 = TFT_D1;
      cfg.pin_d2 = TFT_D2; cfg.pin_d3 = TFT_D3;
      cfg.pin_d4 = TFT_D4; cfg.pin_d5 = TFT_D5;
      cfg.pin_d6 = TFT_D6; cfg.pin_d7 = TFT_D7;

      _bus.config(cfg);
      _panel.setBus(&_bus);
    }

    { // Panel
      auto cfg = _panel.config();
      cfg.pin_cs = TFT_CS;
      cfg.pin_rst = TFT_RST;
      cfg.pin_busy = -1;

      cfg.memory_width  = 320;
      cfg.memory_height = 480;
      cfg.panel_width   = 320;
      cfg.panel_height  = 480;

      cfg.offset_x = 0;
      cfg.offset_y = 0;
      cfg.invert = false;
      cfg.rgb_order = false;

      _panel.config(cfg);
    }
    setPanel(&_panel);
  }
};

LGFX tft;

/* ================= STATUS SYSTEM ================= */
enum MyStatus {
  ST_FREE = 0,
  ST_BUSY,
  ST_SLEEPING,
  ST_MISS_YOU,
  ST_BAD_DAY,
  ST_COUNT
};

MyStatus myStatus = ST_FREE;
MyStatus partnerStatus = ST_MISS_YOU;

const char* statusText[ST_COUNT] = {
  "FREE",
  "BUSY",
  "SLEEPING",
  "MISS YOU",
  "BAD DAY"
};

/* ================= ICON DRAWING (20% BIGGER) ================= */
static constexpr float SCALE = 1.2;   // 20% bigger

void drawHappyFace(int x, int y) {
  int r = 45 * SCALE;
  int eyeOffsetX = 15 * SCALE;
  int eyeOffsetY = 10 * SCALE;
  int eyeSize = 5 * SCALE;
  int smileR = 20 * SCALE;

  tft.fillCircle(x, y, r, TFT_YELLOW);
  tft.fillCircle(x - eyeOffsetX, y - eyeOffsetY, eyeSize, TFT_BLACK);
  tft.fillCircle(x + eyeOffsetX, y - eyeOffsetY, eyeSize, TFT_BLACK);
  tft.drawArc(x, y + 10 * SCALE, smileR, smileR, 200, 340, TFT_BLACK);
}

void drawSadFace(int x, int y) {
  int r = 45 * SCALE;
  int eyeOffsetX = 15 * SCALE;
  int eyeOffsetY = 10 * SCALE;
  int eyeSize = 5 * SCALE;
  int sadR = 20 * SCALE;

  tft.fillCircle(x, y, r, TFT_ORANGE);
  tft.fillCircle(x - eyeOffsetX, y - eyeOffsetY, eyeSize, TFT_BLACK);
  tft.fillCircle(x + eyeOffsetX, y - eyeOffsetY, eyeSize, TFT_BLACK);
  tft.drawArc(x, y + 25 * SCALE, sadR, sadR, 20, 160, TFT_BLACK);
}

void drawBusyIcon(int x, int y) {
  int r = 45 * SCALE;
  int lineOffset = 32 * SCALE;

  tft.drawCircle(x, y, r, TFT_RED);
  tft.drawLine(x - lineOffset, y + lineOffset,
               x + lineOffset, y - lineOffset, TFT_RED);
}

void drawHeart(int x, int y) {
  int circleOffset = 15 * SCALE;
  int circleR = 15 * SCALE;
  int triWidth = 30 * SCALE;
  int triHeight = 40 * SCALE;

  tft.fillCircle(x - circleOffset, y, circleR, TFT_RED);
  tft.fillCircle(x + circleOffset, y, circleR, TFT_RED);
  tft.fillTriangle(x - triWidth, y,
                   x + triWidth, y,
                   x, y + triHeight, TFT_RED);
}

void drawSleep(int x, int y) {
  // Increase text size by ~20%
  tft.setTextSize(5);   // was 4 → approx +25% (closest integer step)
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextDatum(lgfx::textdatum_t::middle_center);
  tft.drawString("Zzz", x, y);
}
/* ================= MAIN UI ================= */
void drawPartnerIconTopRight() {
  if (partnerStatus == ST_MISS_YOU) {
    drawHeart(tft.width() - 20, 25);
  } else {
    tft.fillCircle(tft.width() - 20, 25, 10, TFT_GREEN);
  }
}

void drawStatusScreen() {
  tft.fillScreen(TFT_BLACK);

  drawPartnerIconTopRight();

  int cx = tft.width() / 2;
  int cy = tft.height() / 2 - 20;

  switch (myStatus) {
    case ST_FREE:     drawHappyFace(cx, cy); break;
    case ST_BUSY:     drawBusyIcon(cx, cy); break;
    case ST_SLEEPING: drawSleep(cx, cy); break;
    case ST_MISS_YOU: drawHeart(cx, cy); break;
    case ST_BAD_DAY:  drawSadFace(cx, cy); break;
    default: break;
  }

  tft.setTextSize(3);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(lgfx::textdatum_t::middle_center);
  tft.drawString(statusText[(int)myStatus], tft.width() / 2, tft.height() - 40);
}

/* ================= BUTTON (FIXED) ================= */
// Proper edge-detect: triggers once when button goes HIGH -> LOW
bool buttonPressedEvent() {
  static bool lastStable = HIGH;
  static bool lastRead   = HIGH;
  static uint32_t lastChangeMs = 0;

  bool r = digitalRead(ENC_SW);
  uint32_t now = millis();

  if (r != lastRead) {
    lastRead = r;
    lastChangeMs = now;
  }

  // stable for 40ms
  if (now - lastChangeMs >= 40) {
    if (lastStable != lastRead) {
      lastStable = lastRead;

      // detect press edge (active LOW)
      if (lastStable == LOW) return true;
    }
  }

  return false;
}

/* ================= SETUP & LOOP ================= */
void setup() {
  tft.init();

  // ✅ You said rotation(3) is correct
  tft.setRotation(3);

  pinMode(ENC_SW, INPUT_PULLUP);

  drawStatusScreen();
}

void loop() {
  if (buttonPressedEvent()) {
    myStatus = (MyStatus)((myStatus + 1) % ST_COUNT);
    drawStatusScreen();
  }
}


