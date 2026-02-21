#include <LovyanGFX.hpp>
#include <math.h>

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
    { 
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

    { 
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

/* ================= ICON SCALE ================= */
float SCALE = 1.2f;  

/* ================= HELPERS ================= */
uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) { return tft.color565(r, g, b); }

void clearCenterIconArea(int cx, int cy) {
  const int w = 220;
  const int h = 220;
  tft.fillRect(cx - w/2, cy - h/2, w, h, TFT_BLACK);
}

void drawThickCircle(int x, int y, int r, int thickness, uint16_t color) {
  for (int i = 0; i < thickness; i++) tft.drawCircle(x, y, r - i, color);
}

void drawThickLine(int x0, int y0, int x1, int y1, int thickness, uint16_t color) {
  int half = thickness / 2;
  for (int dx = -half; dx <= half; dx++) {
    for (int dy = -half; dy <= half; dy++) {
      tft.drawLine(x0 + dx, y0 + dy, x1 + dx, y1 + dy, color);
    }
  }
}

/* ================= HAPPY / SAD / BUSY ================= */
void drawHappyFace(int x, int y) {
  int r = (int)(45 * SCALE);
  int eyeOffsetX = (int)(15 * SCALE);
  int eyeOffsetY = (int)(10 * SCALE);
  int eyeSize = (int)(5 * SCALE);

  tft.fillCircle(x, y, r, TFT_YELLOW);
  drawThickCircle(x, y, r, 2, TFT_BLACK);

  tft.fillCircle(x - eyeOffsetX, y - eyeOffsetY, eyeSize + 1, TFT_BLACK);
  tft.fillCircle(x + eyeOffsetX, y - eyeOffsetY, eyeSize + 1, TFT_BLACK);

  int mouthWidth  = (int)(28 * SCALE);
  int mouthHeight = (int)(12 * SCALE);
  int mouthY      = (int)(y + (15 * SCALE));

  int prevX = x - mouthWidth;
  int prevY = mouthY;

  for (int i = -mouthWidth; i <= mouthWidth; i++) {
    float t = (float)i / mouthWidth;
    int curveY = mouthY + (int)(mouthHeight * (1 - t * t));
    int drawX = x + i;
    drawThickLine(prevX, prevY, drawX, curveY, 2, TFT_BLACK);
    prevX = drawX; prevY = curveY;
  }
}

void drawSadFace(int x, int y) {
  int r = (int)(45 * SCALE);
  int eyeOffsetX = (int)(15 * SCALE);
  int eyeOffsetY = (int)(10 * SCALE);
  int eyeSize = (int)(5 * SCALE);

  tft.fillCircle(x, y, r, TFT_ORANGE);
  drawThickCircle(x, y, r, 2, TFT_BLACK);

  int leftEyeX  = x - eyeOffsetX;
  int rightEyeX = x + eyeOffsetX;
  int eyeY      = y - eyeOffsetY;

  tft.fillCircle(leftEyeX,  eyeY, eyeSize + 1, TFT_BLACK);
  tft.fillCircle(rightEyeX, eyeY, eyeSize + 1, TFT_BLACK);

  int mouthWidth  = (int)(28 * SCALE);
  int mouthHeight = (int)(12 * SCALE);
  int mouthY      = (int)(y + (32 * SCALE));

  int prevX = x - mouthWidth;
  int prevY = mouthY;

  for (int i = -mouthWidth; i <= mouthWidth; i++) {
    float t = (float)i / mouthWidth;
    int curveY = mouthY - (int)(mouthHeight * (1 - t * t));
    int drawX = x + i;
    drawThickLine(prevX, prevY, drawX, curveY, 2, TFT_BLACK);
    prevX = drawX; prevY = curveY;
  }

  // Tear
  uint16_t tearFill = tft.color565(80, 180, 255);
  uint16_t tearOut  = tft.color565(40, 120, 220);

  int tearX = rightEyeX + (int)(4 * SCALE);
  int tearY = eyeY + (int)(12 * SCALE);
  int tearR = (int)(6 * SCALE);

  tft.fillCircle(tearX, tearY, tearR, tearFill);
  tft.fillTriangle(tearX - tearR, tearY, tearX + tearR, tearY, tearX, tearY + (tearR * 2), tearFill);

  drawThickCircle(tearX, tearY, tearR, 2, tearOut);
  drawThickLine(tearX - tearR, tearY, tearX, tearY + tearR * 2, 2, tearOut);
  drawThickLine(tearX + tearR, tearY, tearX, tearY + tearR * 2, 2, tearOut);
}

void drawBusyIcon(int x, int y) {
  int r = (int)(45 * SCALE);
  int lineOffset = (int)(32 * SCALE);
  drawThickCircle(x, y, r, 2, TFT_RED);
  drawThickLine(x - lineOffset, y + lineOffset, x + lineOffset, y - lineOffset, 2, TFT_RED);
}

/* =================  HEART ================= */
void drawHeartShape(int x, int y, float s, uint16_t color) {
  int circleOffset = (int)(15 * s);
  int circleR      = (int)(15 * s);
  int triWidth     = (int)(30 * s);
  int triHeight    = (int)(40 * s);

  tft.fillCircle(x - circleOffset, y, circleR, color);
  tft.fillCircle(x + circleOffset, y, circleR, color);
  tft.fillTriangle(x - triWidth, y, x + triWidth, y, x, y + triHeight, color);
}

void drawHeart(int x, int y) {
  uint16_t shadow   = tft.color565(130,  0, 25);
  uint16_t base     = tft.color565(230, 20, 60);
  uint16_t inner    = tft.color565(255, 70, 110);
  uint16_t gloss1   = tft.color565(255, 180, 200);
  uint16_t gloss2   = tft.color565(255, 230, 235);

  float s = SCALE;

  drawHeartShape(x + (int)(2 * s), y + (int)(3 * s), s, shadow);
  drawHeartShape(x, y, s, base);
  drawHeartShape(x - (int)(2 * s), y - (int)(2 * s), s * 0.86f, inner);

  int shineX = x - (int)(10 * s);
  int shineY = y - (int)(6 * s);

  tft.fillCircle(shineX, shineY, (int)(6 * s),  gloss1);
  tft.fillCircle(shineX - (int)(2 * s), shineY - (int)(2 * s), (int)(3 * s), gloss2);
  tft.fillCircle(x - (int)(2 * s), y - (int)(14 * s), (int)(2 * s), gloss2);
}

/* ================= SLEEP: STATIC SCENE + ZZZ  ================= */
static uint32_t zLastMs = 0;
static uint8_t  zFrame  = 0;
static bool     sleepSceneDrawn = false;

struct Rect { int x, y, w, h; };
static Rect zPrev[3] = { {0,0,0,0}, {0,0,0,0}, {0,0,0,0} };

void drawZ(float s, int x, int y, int size, uint16_t col) {
  int t = (int)(2 * s);
  if (t < 1) t = 1;
  for (int i = 0; i < t; i++) {
    tft.drawLine(x, y + i, x + size, y + i, col);
    tft.drawLine(x + size, y + i, x, y + size + i, col);
    tft.drawLine(x, y + size + i, x + size, y + size + i, col);
  }
}

void drawSleepingBearStatic(int cx, int cy) {
  float s = 0.72f;

  auto U = [&](float v)->int {
    int out = (int)lroundf(v * s);
    return (out < 1) ? 1 : out;
  };

  // Colors
  uint16_t bedBase    = rgb565(70, 120, 255);
  uint16_t bedHi      = rgb565(140, 185, 255);
  uint16_t bedShadow  = rgb565(18, 20, 28);

  uint16_t pillowBase = rgb565(245, 245, 255);
  uint16_t pillowHi   = rgb565(255, 255, 255);

  uint16_t blanketBase= rgb565(55, 165, 175);   // teal
  uint16_t blanketHi  = rgb565(135, 220, 220);  // highlight
  uint16_t blanketSh  = rgb565(30, 115, 125);   // shadow
  uint16_t blanketEdge= rgb565(20, 90, 100);    // subtle outline

  uint16_t furBase    = rgb565(180, 130, 90);
  uint16_t furShadow  = rgb565(140, 100, 70);
  uint16_t furHi      = rgb565(225, 185, 145);

  uint16_t lineCol    = rgb565(70, 45, 38);

  // ---------------- BED ----------------
  int bedY = cy + U(70);
  int bedW = U(230);
  int bedH = U(62);

  tft.fillRoundRect(cx - bedW/2 + U(3), bedY - bedH/2 + U(4), bedW, bedH, U(22), bedShadow);
  tft.fillRoundRect(cx - bedW/2,        bedY - bedH/2,        bedW, bedH, U(22), bedBase);
  tft.fillRoundRect(cx - bedW/2 + U(10), bedY - bedH/2 + U(10), bedW - U(20), U(14), U(16), bedHi);

  // ---------------- PILLOW ----------------
  int pillowX = cx - U(70);
  int pillowY = cy + U(18);
  int pillowW = U(130);
  int pillowH = U(62);

  tft.fillRoundRect(pillowX - pillowW/2 + U(3), pillowY - pillowH/2 + U(3),
                    pillowW, pillowH, U(22), bedShadow);
  tft.fillRoundRect(pillowX - pillowW/2, pillowY - pillowH/2,
                    pillowW, pillowH, U(22), pillowBase);
  tft.fillRoundRect(pillowX - pillowW/2 + U(10), pillowY - pillowH/2 + U(10),
                    pillowW - U(20), U(16), U(14), pillowHi);

  // ---------------- BEAR HEAD ----------------
  int hx = pillowX - U(4);
  int hy = pillowY - U(4);
  int headR = U(28);

  tft.fillCircle(hx + U(3), hy + U(2), headR, furShadow);
  tft.fillCircle(hx,        hy,        headR, furBase);

  // ear
  tft.fillCircle(hx - U(18), hy - U(16), U(10), furShadow);
  tft.fillCircle(hx - U(20), hy - U(18), U(10), furBase);
  tft.fillCircle(hx - U(22), hy - U(20), U(5),  furHi);

  // snout highlight
  tft.fillEllipse(hx + U(10), hy + U(14), U(30), U(18), furHi);

  // eyes closed
  tft.drawLine(hx - U(10), hy - U(6), hx - U(2),  hy - U(6), lineCol);
  tft.drawLine(hx + U(2),  hy - U(6), hx + U(10), hy - U(6), lineCol);

  // nose + small mouth 
  tft.fillRoundRect(hx + U(7), hy + U(3), U(11), U(7), U(3), lineCol);
  tft.drawCircle(hx + U(12), hy + U(16), U(4), lineCol);

  // ---------------- BLANKET  ----------------
  int bx = cx + U(25);
  int by = cy + U(52);

  int blankW = U(195);
  int blankH = U(90);

  int blankX = bx - blankW/2;
  int blankY = by - U(38);

  // Shadow under blanket 
  tft.fillRoundRect(blankX + U(4), blankY + U(7), blankW, blankH, U(28), blanketSh);

  // Main blanket
  tft.fillRoundRect(blankX, blankY, blankW, blankH, U(28), blanketBase);

  // Subtle outline (only bottom + right feel)
  tft.drawRoundRect(blankX, blankY, blankW, blankH, U(28), blanketEdge);

  // Top fold highlight strip
  tft.fillRoundRect(blankX + U(10), blankY + U(10), blankW - U(20), U(18), U(14), blanketHi);

  // Crease arcs (like fabric folds)
  tft.drawArc(blankX + U(70),  blankY + U(58), U(32), U(18), 210, 330, blanketHi);
  tft.drawArc(blankX + U(128), blankY + U(68), U(38), U(22), 210, 330, blanketHi);

  // Blanket tuck near head (small cut illusion)
 
  tft.drawArc(blankX + U(30), blankY + U(30), U(22), U(16), 40, 140, blanketSh);

  // ---------------- PAW (FIXED POSITION) ----------------

  int pawX = blankX + U(125);
  int pawY = blankY + U(24);  
  int pawR = U(10);

  // paw shadow + paw
  tft.fillCircle(pawX + U(2), pawY + U(2), pawR, furShadow);
  tft.fillCircle(pawX,        pawY,        pawR, furBase);

  // toe beans
  uint16_t bean = rgb565(235, 175, 175);
  tft.fillCircle(pawX - U(5), pawY - U(2), U(2), bean);
  tft.fillCircle(pawX - U(1), pawY - U(4), U(2), bean);
  tft.fillCircle(pawX + U(3), pawY - U(2), U(2), bean);
}

void eraseOldZzz() {
  for (int i = 0; i < 3; i++) {
    if (zPrev[i].w > 0 && zPrev[i].h > 0) {
      tft.fillRect(zPrev[i].x, zPrev[i].y, zPrev[i].w, zPrev[i].h, TFT_BLACK);
    }
  }
}

void drawZzzOnly(int cx, int cy) {
  float s = 1.0f;
  uint16_t zColMid = rgb565(180, 210, 255);

  for (int i = 0; i < 3; i++) {
    float phase = (zFrame + i * 18) / 255.0f; 
    int zx = cx + (int)(50*s) + (int)(i * (14*s));                
    int zy = cy - (int)(58*s) - (int)(phase * (26*s)) + (int)(sin((zFrame + i*20) * 0.05f) * (2*s));
    int zs = (int)((10 + i*3) * s);

    uint16_t zc = (i == 0) ? rgb565(210, 230, 255)
                 : (i == 1) ? zColMid
                 : rgb565(140, 180, 240);

    int pad = 5;
    zPrev[i] = { zx - pad, zy - pad, zs + pad*2, zs + pad*2 + 8 };
    drawZ(s, zx, zy, zs, zc);
  }
}

void startSleepScene(int cx, int cy) {
  clearCenterIconArea(cx, cy);
  drawSleepingBearStatic(cx, cy);

  for (int i = 0; i < 3; i++) zPrev[i] = {0,0,0,0};
  eraseOldZzz();
  drawZzzOnly(cx, cy);

  sleepSceneDrawn = true;
}

void updateZzzAnimation(int cx, int cy) {
  uint32_t now = millis();
  if (now - zLastMs < 90) return;

  zLastMs = now;
  zFrame += 6;

  eraseOldZzz();
  drawZzzOnly(cx, cy);
}

/* ================= MAIN UI ================= */
void drawPartnerIconTopRight() {
  int px = tft.width() - 25 - 45;  
  int py = 25;

  float oldScale = SCALE;
  SCALE = 0.9f;

  if (partnerStatus == ST_MISS_YOU) {
    drawHeart(px, py);
  } else {
    tft.fillCircle(px, py, 10, TFT_GREEN);
  }

  SCALE = oldScale;
}

void drawStatusScreen() {
  tft.fillScreen(TFT_BLACK);
  sleepSceneDrawn = false;

  drawPartnerIconTopRight();

  int cx = tft.width() / 2;
  int cy = tft.height() / 2 - 20;

  clearCenterIconArea(cx, cy);

  switch (myStatus) {
    case ST_FREE:     drawHappyFace(cx, cy); break;
    case ST_BUSY:     drawBusyIcon(cx, cy); break;
    case ST_SLEEPING: startSleepScene(cx, cy); break;
    case ST_MISS_YOU: drawHeart(cx, cy); break;
    case ST_BAD_DAY:  drawSadFace(cx, cy); break;
    default: break;
  }

  tft.setTextSize(3);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(lgfx::textdatum_t::middle_center);
  tft.drawString(statusText[(int)myStatus], tft.width() / 2, tft.height() - 40);
}

/* ================= BUTTON ================= */
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

  if (now - lastChangeMs >= 40) {
    if (lastStable != lastRead) {
      lastStable = lastRead;
      if (lastStable == LOW) return true;
    }
  }
  return false;
}

/* ================= SETUP & LOOP ================= */
void setup() {
  tft.init();
  tft.setRotation(3);
  pinMode(ENC_SW, INPUT_PULLUP);
  drawStatusScreen();
}

void loop() {
  if (buttonPressedEvent()) {
    myStatus = (MyStatus)((myStatus + 1) % ST_COUNT);
    drawStatusScreen();
  }

  // Animate Zzz while sleeping 
  if (myStatus == ST_SLEEPING && sleepSceneDrawn) {
    int cx = tft.width() / 2;
    int cy = tft.height() / 2 - 20;
    updateZzzAnimation(cx, cy);
  }
}