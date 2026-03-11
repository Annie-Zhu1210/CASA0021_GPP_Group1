/*
 * All TFT drawing functions for the device.
 * Home screen layout:
 *   Partner's status and time in the centre with a bigger size.
 *   Self status and time with a smaller size one the right panel.
 *   Wi-Fi indicator on the top left.
 */

#ifndef SCREEN_DRAW_H
#define SCREEN_DRAW_H

#include "globals.h"

bool getLocalTimeSafe(struct tm* out);
bool getTimeInZone(int idx, time_t baseNow, struct tm* out);
void resetWorldClockCache();
void drawPartnerTimeOnly();
void drawSelfPanelTimeOnly();
int getUsedScheduleCount();
int getUsedScheduleSlotByRow(int row);
void formatScheduleSummary(int slot, char* line1, size_t n1, char* line2, size_t n2);
void formatRepeatLabelFromDate(uint8_t repeat, int year, int month, int day, char* out, size_t n);
void formatScheduleItemSummary(const ScheduleItem& s, char* line1, size_t n1, char* line2, size_t n2);

uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
  return tft.color565(r, g, b);
}

// Layout constants
static constexpr int DIVIDER_X = 330;
static constexpr int TOP_STRIP_H = 48;
static constexpr int SELF_PX_BODY = DIVIDER_X + 1;

// Wi-Fi icon
static constexpr int WIFI_IX = 4;
static constexpr int WIFI_IY = 8;
static constexpr int WIFI_IW = 38;
static constexpr int WIFI_IH = 26;

// Partner information strip
static constexpr int PINFO_X = WIFI_IX + WIFI_IW + 4;
static constexpr int PINFO_W = DIVIDER_X - PINFO_X - 2;
static constexpr int PINFO_UTC_Y = 4;
static constexpr int PINFO_TIME_Y = 18;

// Self information strip
static constexpr int SINFO_X = DIVIDER_X + 2;
static constexpr int SINFO_W = 480 - SINFO_X - 2;
static constexpr int SINFO_UTC_Y = 4;
static constexpr int SINFO_TIME_Y = 18;

// Partner status emoji body
static constexpr int PARTNER_CX = DIVIDER_X / 2;
static constexpr int PARTNER_CY = TOP_STRIP_H + (320 - TOP_STRIP_H - 36) / 2;
static constexpr int PARTNER_LABEL_Y = 284;

// Self status emoji body
static constexpr int SELF_EMO_CX = DIVIDER_X + (480 - DIVIDER_X) / 2;
static constexpr int SELF_EMO_CY = TOP_STRIP_H + (320 - TOP_STRIP_H) / 2 - 14;


// Self panel
inline uint16_t warmBg() {
  return tft.color565(31, 30, 31);
}

// Wi-Fi indicator

void drawHomeWiFiIndicator(bool force = false) {
  bool connected = (WiFi.status() == WL_CONNECTED);
  int state = connected ? 1 : 0;
  if (!force && homeWifiIconState == state) return;
  homeWifiIconState = state;
  tft.fillRect(WIFI_IX, WIFI_IY, WIFI_IW, WIFI_IH, TFT_BLACK);
  uint16_t col = connected ? TFT_GREEN : tft.color565(80, 80, 80);
  struct {
    int x, y, w, h;
  } bars[3] = {
    { WIFI_IX + 0, WIFI_IY + 16, 7, 8 },
    { WIFI_IX + 10, WIFI_IY + 10, 7, 14 },
    { WIFI_IX + 20, WIFI_IY + 4, 7, 20 },
  };
  for (int i = 0; i < 3; i++) {
    if (connected) tft.fillRect(bars[i].x, bars[i].y, bars[i].w, bars[i].h, col);
    else tft.drawRect(bars[i].x, bars[i].y, bars[i].w, bars[i].h, col);
  }
}

void drawWiFiIndicator(bool force = false) {
  const int iw = 30, ih = 22, ox = tft.width() - iw - 8, oy = 8;
  bool connected = (WiFi.status() == WL_CONNECTED);
  int state = connected ? 1 : 0;
  if (!force && headerWifiIconState == state) return;
  headerWifiIconState = state;
  tft.fillRect(ox, oy, iw, ih, TFT_BLACK);
  uint16_t col = connected ? TFT_GREEN : tft.color565(80, 80, 80);
  struct {
    int x, y, w, h;
  } bars[3] = {
    { ox + 0, oy + 14, 7, 8 },
    { ox + 10, oy + 8, 7, 14 },
    { ox + 20, oy + 2, 7, 20 },
  };
  for (int i = 0; i < 3; i++) {
    if (connected) tft.fillRect(bars[i].x, bars[i].y, bars[i].w, bars[i].h, col);
    else tft.drawRect(bars[i].x, bars[i].y, bars[i].w, bars[i].h, col);
  }
}

// Shared header for screens other than the home screen
void drawHeader(const char* title) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(lgfx::top_left);
  tft.drawString(title, 10, 10);
  tft.drawFastHLine(10, 36, tft.width() - 20, TFT_DARKGREY);
  drawWiFiIndicator(true);
}

// Button
void drawButton(int x, int y, int w, int h,
                const char* label, bool selected,
                uint16_t accentCol = 0) {
  uint16_t bg = selected ? ((accentCol != 0) ? accentCol : TFT_DARKCYAN) : TFT_BLACK;
  tft.fillRoundRect(x, y, w, h, 6, bg);
  tft.drawRoundRect(x, y, w, h, 6, selected ? TFT_WHITE : TFT_DARKGREY);
  tft.setTextColor(selected ? TFT_WHITE : TFT_LIGHTGREY, bg);
  tft.setTextDatum(lgfx::middle_center);
  tft.setTextSize(2);
  tft.drawString(label, x + w / 2, y + h / 2);
  tft.setTextDatum(lgfx::top_left);
}

// Emoji drawing
void drawThickCircle(int x, int y, int r, int thickness, uint16_t color) {
  for (int i = 0; i < thickness; i++) tft.drawCircle(x, y, r - i, color);
}

void drawThickLine(int x0, int y0, int x1, int y1, int thickness, uint16_t color) {
  int half = thickness / 2;
  for (int dx = -half; dx <= half; dx++)
    for (int dy = -half; dy <= half; dy++)
      tft.drawLine(x0 + dx, y0 + dy, x1 + dx, y1 + dy, color);
}

void drawHappyFace(int x, int y) {
  int r = (int)(45 * SCALE), ex = (int)(15 * SCALE), ey = (int)(10 * SCALE), es = (int)(5 * SCALE);
  tft.fillCircle(x, y, r, TFT_YELLOW);
  drawThickCircle(x, y, r, 2, TFT_BLACK);
  tft.fillCircle(x - ex, y - ey, es + 1, TFT_BLACK);
  tft.fillCircle(x + ex, y - ey, es + 1, TFT_BLACK);
  int mw = (int)(28 * SCALE), mh = (int)(12 * SCALE), my2 = (int)(y + 15 * SCALE);
  int px = x - mw, py = my2;
  for (int i = -mw; i <= mw; i++) {
    float t = (float)i / mw;
    int cy2 = my2 + (int)(mh * (1 - t * t));
    drawThickLine(px, py, x + i, cy2, 2, TFT_BLACK);
    px = x + i;
    py = cy2;
  }
}

void drawSadFace(int x, int y) {
  int r = (int)(45 * SCALE), ex = (int)(15 * SCALE), ey = (int)(10 * SCALE), es = (int)(5 * SCALE);
  tft.fillCircle(x, y, r, TFT_ORANGE);
  drawThickCircle(x, y, r, 2, TFT_BLACK);
  tft.fillCircle(x - ex, y - ey, es + 1, TFT_BLACK);
  tft.fillCircle(x + ex, y - ey, es + 1, TFT_BLACK);
  int mw = (int)(28 * SCALE), mh = (int)(12 * SCALE), my2 = (int)(y + 32 * SCALE);
  int px = x - mw, py = my2;
  for (int i = -mw; i <= mw; i++) {
    float t = (float)i / mw;
    int cy2 = my2 - (int)(mh * (1 - t * t));
    drawThickLine(px, py, x + i, cy2, 2, TFT_BLACK);
    px = x + i;
    py = cy2;
  }
  uint16_t tf2 = tft.color565(80, 180, 255), to = tft.color565(40, 120, 220);
  int tx = x + ex + (int)(4 * SCALE), ty = y - ey + (int)(12 * SCALE), tr = (int)(6 * SCALE);
  tft.fillCircle(tx, ty, tr, tf2);
  tft.fillTriangle(tx - tr, ty, tx + tr, ty, tx, ty + tr * 2, tf2);
  drawThickCircle(tx, ty, tr, 2, to);
  drawThickLine(tx - tr, ty, tx, ty + tr * 2, 2, to);
  drawThickLine(tx + tr, ty, tx, ty + tr * 2, 2, to);
}

void drawBusyIcon(int x, int y) {
  int r = (int)(45 * SCALE), lo = (int)(32 * SCALE);
  drawThickCircle(x, y, r, 2, TFT_RED);
  drawThickLine(x - lo, y + lo, x + lo, y - lo, 2, TFT_RED);
}

void drawHeartShape(int x, int y, float s, uint16_t color) {
  tft.fillCircle(x - (int)(15 * s), y, (int)(15 * s), color);
  tft.fillCircle(x + (int)(15 * s), y, (int)(15 * s), color);
  tft.fillTriangle(x - (int)(30 * s), y, x + (int)(30 * s), y, x, y + (int)(40 * s), color);
}

void drawHeart(int x, int y) {
  float s = SCALE;
  drawHeartShape(x + (int)(2 * s), y + (int)(3 * s), s, tft.color565(130, 0, 25));
  drawHeartShape(x, y, s, tft.color565(230, 20, 60));
  drawHeartShape(x - (int)(2 * s), y - (int)(2 * s), s * 0.86f, tft.color565(255, 70, 110));
  int sx = x - (int)(10 * s), sy = y - (int)(6 * s);
  tft.fillCircle(sx, sy, (int)(6 * s), tft.color565(255, 180, 200));
  tft.fillCircle(sx - (int)(2 * s), sy - (int)(2 * s), (int)(3 * s), tft.color565(255, 230, 235));
  tft.fillCircle(x - (int)(2 * s), y - (int)(14 * s), (int)(2 * s), tft.color565(255, 230, 235));
}

void drawOfflineFace(int x, int y) {
  int r = (int)(45 * SCALE);
  uint16_t face = tft.color565(125, 130, 140);
  uint16_t edge = tft.color565(70, 74, 82);
  uint16_t dark = tft.color565(35, 38, 44);
  uint16_t accent = tft.color565(220, 90, 90);
  tft.fillCircle(x, y, r, face);
  drawThickCircle(x, y, r, 2, edge);

  int ex = (int)(16 * SCALE), ey = (int)(10 * SCALE), d = (int)(5 * SCALE);
  drawThickLine(x - ex - d, y - ey - d, x - ex + d, y - ey + d, 2, dark);
  drawThickLine(x - ex - d, y - ey + d, x - ex + d, y - ey - d, 2, dark);
  drawThickLine(x + ex - d, y - ey - d, x + ex + d, y - ey + d, 2, dark);
  drawThickLine(x + ex - d, y - ey + d, x + ex + d, y - ey - d, 2, dark);
  drawThickLine(x - (int)(16 * SCALE), y + (int)(18 * SCALE),
                x + (int)(16 * SCALE), y + (int)(18 * SCALE), 2, dark);

  int px = x + (int)(24 * SCALE), py = y + (int)(18 * SCALE);
  tft.fillRoundRect(px - (int)(10 * SCALE), py - (int)(8 * SCALE), (int)(20 * SCALE), (int)(16 * SCALE), 3, edge);
  tft.fillRect(px + (int)(10 * SCALE), py - (int)(5 * SCALE), (int)(4 * SCALE), (int)(3 * SCALE), edge);
  tft.fillRect(px + (int)(10 * SCALE), py + (int)(2 * SCALE), (int)(4 * SCALE), (int)(3 * SCALE), edge);
  drawThickLine(px - (int)(13 * SCALE), py + (int)(10 * SCALE),
                px + (int)(17 * SCALE), py - (int)(10 * SCALE), 2, accent);
}

void drawCheckingFace(int x, int y) {
  int r = (int)(45 * SCALE);
  uint16_t face = tft.color565(95, 105, 130);
  uint16_t edge = tft.color565(60, 75, 105);
  uint16_t dark = tft.color565(20, 25, 35);
  uint16_t accent = tft.color565(120, 180, 255);
  tft.fillCircle(x, y, r, face);
  drawThickCircle(x, y, r, 2, edge);
  tft.fillCircle(x - (int)(14 * SCALE), y - (int)(8 * SCALE), (int)(4 * SCALE), dark);
  tft.fillCircle(x + (int)(14 * SCALE), y - (int)(8 * SCALE), (int)(4 * SCALE), dark);
  tft.fillCircle(x - (int)(8 * SCALE), y + (int)(16 * SCALE), (int)(2 * SCALE), accent);
  tft.fillCircle(x, y + (int)(16 * SCALE), (int)(2 * SCALE), accent);
  tft.fillCircle(x + (int)(8 * SCALE), y + (int)(16 * SCALE), (int)(2 * SCALE), accent);
}

// Updated simpler sleeping emoji (sleep bear icon was too big to fit in the right panel)
void drawSleepingZs(int cx, int cy, uint16_t bgCol) {
  int clearR = (int)(50 * SCALE);
  tft.fillRect(cx - clearR, cy - clearR, clearR * 2, clearR * 2, bgCol);

  struct ZDef {
    int offX, offY, legPx;
    uint16_t col;
  };
  ZDef zs[3] = {
    { (int)(-20 * SCALE), (int)(14 * SCALE), max(4, (int)(10 * SCALE)), rgb565(180, 210, 255) },
    { (int)(0 * SCALE), (int)(0 * SCALE), max(5, (int)(14 * SCALE)), rgb565(100, 160, 255) },
    { (int)(22 * SCALE), (int)(-16 * SCALE), max(6, (int)(18 * SCALE)), rgb565(50, 110, 230) },
  };
  for (int i = 0; i < 3; i++) {
    int zx = cx + zs[i].offX, zy = cy + zs[i].offY, s = zs[i].legPx;
    uint16_t c = zs[i].col;
    int t = max(1, s / 5);
    drawThickLine(zx - s / 2, zy - s / 2, zx + s / 2, zy - s / 2, t, c);
    drawThickLine(zx + s / 2, zy - s / 2, zx - s / 2, zy + s / 2, t, c);
    drawThickLine(zx - s / 2, zy + s / 2, zx + s / 2, zy + s / 2, t, c);
  }
}

// Sleeping bear for partner's status (enough space to show the shole emoji)
void drawZ_bear(float s, int x, int y, int size, uint16_t col) {
  int t = max(1, (int)(2 * s));
  for (int i = 0; i < t; i++) {
    tft.drawLine(x, y + i, x + size, y + i, col);
    tft.drawLine(x + size, y + i, x, y + size + i, col);
    tft.drawLine(x, y + size + i, x + size, y + size + i, col);
  }
}
void eraseOldZzz() {
  for (int i = 0; i < 3; i++)
    if (zPrev[i].w > 0) tft.fillRect(zPrev[i].x, zPrev[i].y, zPrev[i].w, zPrev[i].h, TFT_BLACK);
}
void drawZzzOnly(int cx, int cy) {
  uint16_t zm = rgb565(180, 210, 255);
  for (int i = 0; i < 3; i++) {
    float ph = (zFrame + i * 18) / 255.0f;
    int zx = cx + 50 + i * 14, zy = cy - 58 - (int)(ph * 26) + (int)(sin((zFrame + i * 20) * 0.05f) * 2), zs = 10 + i * 3;
    uint16_t zc = (i == 0) ? rgb565(210, 230, 255) : (i == 1) ? zm
                                                              : rgb565(140, 180, 240);
    int pad = 5;
    zPrev[i] = { zx - pad, zy - pad, zs + pad * 2, zs + pad * 2 + 8 };
    drawZ_bear(1.0f, zx, zy, zs, zc);
  }
}
void drawSleepingBearStatic(int cx, int cy) {
  float s = 0.72f;
  auto U = [&](float v) -> int {
    return max(1, (int)lroundf(v * s));
  };
  uint16_t bedBase = rgb565(70, 120, 255), bedHi = rgb565(140, 185, 255), bedShadow = rgb565(18, 20, 28);
  uint16_t pillowBase = rgb565(245, 245, 255), pillowHi = rgb565(255, 255, 255);
  uint16_t blanketBase = rgb565(55, 165, 175), blanketHi = rgb565(135, 220, 220);
  uint16_t blanketSh = rgb565(30, 115, 125), blanketEdge = rgb565(20, 90, 100);
  uint16_t furBase = rgb565(180, 130, 90), furShadow = rgb565(140, 100, 70), furHi = rgb565(225, 185, 145);
  uint16_t lineCol = rgb565(70, 45, 38);
  int bedY = cy + U(70), bedW = U(230), bedH = U(62);
  tft.fillRoundRect(cx - bedW / 2 + U(3), bedY - bedH / 2 + U(4), bedW, bedH, U(22), bedShadow);
  tft.fillRoundRect(cx - bedW / 2, bedY - bedH / 2, bedW, bedH, U(22), bedBase);
  tft.fillRoundRect(cx - bedW / 2 + U(10), bedY - bedH / 2 + U(10), bedW - U(20), U(14), U(16), bedHi);
  int pillowX = cx - U(70), pillowY = cy + U(18), pillowW = U(130), pillowH = U(62);
  tft.fillRoundRect(pillowX - pillowW / 2 + U(3), pillowY - pillowH / 2 + U(3), pillowW, pillowH, U(22), bedShadow);
  tft.fillRoundRect(pillowX - pillowW / 2, pillowY - pillowH / 2, pillowW, pillowH, U(22), pillowBase);
  tft.fillRoundRect(pillowX - pillowW / 2 + U(10), pillowY - pillowH / 2 + U(10), pillowW - U(20), U(16), U(14), pillowHi);
  int hx = pillowX - U(4), hy = pillowY - U(4), headR = U(28);
  tft.fillCircle(hx + U(3), hy + U(2), headR, furShadow);
  tft.fillCircle(hx, hy, headR, furBase);
  tft.fillCircle(hx - U(18), hy - U(16), U(10), furShadow);
  tft.fillCircle(hx - U(20), hy - U(18), U(10), furBase);
  tft.fillCircle(hx - U(22), hy - U(20), U(5), furHi);
  tft.fillEllipse(hx + U(10), hy + U(14), U(30), U(18), furHi);
  tft.drawLine(hx - U(10), hy - U(6), hx - U(2), hy - U(6), lineCol);
  tft.drawLine(hx + U(2), hy - U(6), hx + U(10), hy - U(6), lineCol);
  tft.fillRoundRect(hx + U(7), hy + U(3), U(11), U(7), U(3), lineCol);
  tft.drawCircle(hx + U(12), hy + U(16), U(4), lineCol);
  int bx = cx + U(25), by = cy + U(52), blankW = U(195), blankH = U(90), blankX = bx - blankW / 2, blankY = by - U(38);
  tft.fillRoundRect(blankX + U(4), blankY + U(7), blankW, blankH, U(28), blanketSh);
  tft.fillRoundRect(blankX, blankY, blankW, blankH, U(28), blanketBase);
  tft.drawRoundRect(blankX, blankY, blankW, blankH, U(28), blanketEdge);
  tft.fillRoundRect(blankX + U(10), blankY + U(10), blankW - U(20), U(18), U(14), blanketHi);
  tft.drawArc(blankX + U(70), blankY + U(58), U(32), U(18), 210, 330, blanketHi);
  tft.drawArc(blankX + U(128), blankY + U(68), U(38), U(22), 210, 330, blanketHi);
  tft.drawArc(blankX + U(30), blankY + U(30), U(22), U(16), 40, 140, blanketSh);
  int pawX = blankX + U(125), pawY = blankY + U(24), pawR = U(10);
  tft.fillCircle(pawX + U(2), pawY + U(2), pawR, furShadow);
  tft.fillCircle(pawX, pawY, pawR, furBase);
  uint16_t bean = rgb565(235, 175, 175);
  tft.fillCircle(pawX - U(5), pawY - U(2), U(2), bean);
  tft.fillCircle(pawX - U(1), pawY - U(4), U(2), bean);
  tft.fillCircle(pawX + U(3), pawY - U(2), U(2), bean);
}
void startSleepScene(int cx, int cy) {
  drawSleepingBearStatic(cx, cy);
  for (int i = 0; i < 3; i++) zPrev[i] = { 0, 0, 0, 0 };
  eraseOldZzz();
  drawZzzOnly(cx, cy);
  sleepSceneDrawn = true;
}
void updateZzzAnimation(int cx, int cy) {
  if (millis() - zLastMs < 90) return;
  zLastMs = millis();
  zFrame += 6;
  eraseOldZzz();
  drawZzzOnly(cx, cy);
}

// Partner information strip
void drawClockField2(int x, int y, int value, uint16_t fg, uint16_t bg, int digitW, int charH) {
  char b[3];
  snprintf(b, sizeof(b), "%02d", value);
  tft.fillRect(x, y, digitW * 2, charH, bg);
  tft.setTextColor(fg, bg);
  tft.drawString(b, x, y);
}

void drawClockDigit1(int x, int y, int digit, uint16_t fg, uint16_t bg, int digitW, int charH) {
  char c[2] = { (char)('0' + digit), '\0' };
  tft.fillRect(x, y, digitW, charH, bg);
  tft.setTextColor(fg, bg);
  tft.drawString(c, x, y);
}

void drawClockHmsIncremental(int x, int y, int textSize, uint16_t fg, uint16_t bg,
                             int h, int m, int s,
                             int& prevH, int& prevM, int& prevS,
                             bool& prevValid, bool validNow) {
  tft.setTextDatum(lgfx::top_left);
  tft.setTextSize(textSize);
  int digitW = tft.textWidth("0");
  int colonW = tft.textWidth(":");
  int charH = tft.fontHeight();
  int hX = x;
  int c1X = hX + digitW * 2;
  int mX = c1X + colonW;
  int c2X = mX + digitW * 2;
  int sX = c2X + colonW;

  if (!validNow) {
    if (prevValid || prevH != -2 || prevM != -2 || prevS != -2) {
      tft.fillRect(x, y, digitW * 6 + colonW * 2, charH, bg);
      tft.setTextColor(TFT_ORANGE, bg);
      tft.drawString("00:00:00", x, y);
    }
    prevValid = false;
    prevH = prevM = prevS = -2;
    return;
  }

  bool full = !prevValid || prevH < 0 || prevM < 0 || prevS < 0;
  prevValid = true;

  if (full) {
    drawClockField2(hX, y, h, fg, bg, digitW, charH);
    tft.setTextColor(fg, bg);
    tft.drawString(":", c1X, y);
    drawClockField2(mX, y, m, fg, bg, digitW, charH);
    tft.drawString(":", c2X, y);
    drawClockField2(sX, y, s, fg, bg, digitW, charH);
    prevH = h;
    prevM = m;
    prevS = s;
    return;
  }

  if (h != prevH) drawClockField2(hX, y, h, fg, bg, digitW, charH);
  if (m != prevM) drawClockField2(mX, y, m, fg, bg, digitW, charH);
  if (s != prevS) {
    int prevTens = prevS / 10;
    int newTens = s / 10;
    if (newTens != prevTens) drawClockDigit1(sX, y, newTens, fg, bg, digitW, charH);
    drawClockDigit1(sX + digitW, y, s % 10, fg, bg, digitW, charH);
  }

  prevH = h;
  prevM = m;
  prevS = s;
}

void drawClockHmIncremental(int x, int y, int textSize, uint16_t fg, uint16_t bg,
                            int h, int m,
                            int& prevH, int& prevM,
                            bool& prevValid, bool validNow) {
  tft.setTextDatum(lgfx::top_left);
  tft.setTextSize(textSize);
  int digitW = tft.textWidth("0");
  int colonW = tft.textWidth(":");
  int charH = tft.fontHeight();
  int hX = x;
  int c1X = hX + digitW * 2;
  int mX = c1X + colonW;

  if (!validNow) {
    if (prevValid || prevH != -2 || prevM != -2) {
      tft.fillRect(x, y, digitW * 4 + colonW, charH, bg);
      tft.setTextColor(TFT_ORANGE, bg);
      tft.drawString("--:--", x, y);
    }
    prevValid = false;
    prevH = prevM = -2;
    return;
  }

  bool full = !prevValid || prevH < 0 || prevM < 0;
  prevValid = true;

  if (full) {
    drawClockField2(hX, y, h, fg, bg, digitW, charH);
    tft.setTextColor(fg, bg);
    tft.drawString(":", c1X, y);
    drawClockField2(mX, y, m, fg, bg, digitW, charH);
    prevH = h;
    prevM = m;
    return;
  }

  if (h != prevH) drawClockField2(hX, y, h, fg, bg, digitW, charH);
  if (m != prevM) drawClockField2(mX, y, m, fg, bg, digitW, charH);
  prevH = h;
  prevM = m;
}

void drawPartnerInfoStrip() {
  tft.fillRect(PINFO_X, 0, PINFO_W, TOP_STRIP_H, TFT_BLACK);
  char label[24];
  if (!partnerPresenceKnown) snprintf(label, sizeof(label), "P: CHECKING");
  else if (partnerOnline) snprintf(label, sizeof(label), "P: %s", kTimezones[partnerTzIndex].label);
  else snprintf(label, sizeof(label), "P: OFFLINE");
  tft.setTextDatum(lgfx::top_left);
  tft.setTextSize(1);
  uint16_t col = !partnerPresenceKnown ? tft.color565(120, 180, 255)
                                       : (partnerOnline ? TFT_CYAN : TFT_ORANGE);
  tft.setTextColor(col, TFT_BLACK);
  tft.drawString(label, PINFO_X, PINFO_UTC_Y);
  prevPartnerHour = prevPartnerMinute = prevPartnerSecond = -1;
  prevPartnerTimeValid = false;
  drawPartnerTimeOnly();
}

void drawPartnerTimeOnly() {
  if (!partnerPresenceKnown) {
    tft.setTextDatum(lgfx::top_left);
    tft.setTextSize(1);
    int h = tft.fontHeight();
    tft.fillRect(PINFO_X, PINFO_TIME_Y, PINFO_W, h * 2 + 2, TFT_BLACK);
    tft.setTextColor(tft.color565(120, 180, 255), TFT_BLACK);
    tft.drawString("Checking partner...", PINFO_X, PINFO_TIME_Y);
    tft.drawString("Waiting for heartbeat", PINFO_X, PINFO_TIME_Y + h + 1);
    prevPartnerTimeValid = false;
    prevPartnerHour = prevPartnerMinute = prevPartnerSecond = -2;
    return;
  }

  if (!partnerOnline) {
    tft.setTextDatum(lgfx::top_left);
    tft.setTextSize(1);
    int h = tft.fontHeight();
    tft.fillRect(PINFO_X, PINFO_TIME_Y, PINFO_W, h * 2 + 2, TFT_BLACK);
    tft.setTextColor(TFT_ORANGE, TFT_BLACK);
    tft.drawString("Offline since:", PINFO_X, PINFO_TIME_Y);
    char buf[24];
    time_t stamp = (partnerOfflineSinceEpoch > 100000) ? partnerOfflineSinceEpoch : partnerLastSeenEpoch;
    if (stamp > 100000) {
      struct tm ti;
      localtime_r(&stamp, &ti);
      strftime(buf, sizeof(buf), "%m/%d %H:%M", &ti);
      tft.drawString(buf, PINFO_X, PINFO_TIME_Y + h + 1);
    } else {
      tft.drawString("--/-- --:--", PINFO_X, PINFO_TIME_Y + h + 1);
    }
    prevPartnerTimeValid = false;
    prevPartnerHour = prevPartnerMinute = prevPartnerSecond = -2;
    return;
  }

  bool valid = false;
  int h = 0, m = 0, s = 0;
  if (partnerTimeValid && partnerEpoch > 100000) {
    struct tm ti;
    const char* restore = kTimezones[tzIndex].posix;
    setenv("TZ", kTimezones[partnerTzIndex].posix, 1);
    tzset();
    time_t ep = partnerEpoch;
    if (partnerTimeRxMs > 0) ep += (time_t)((millis() - partnerTimeRxMs) / 1000UL);
    localtime_r(&ep, &ti);
    setenv("TZ", restore, 1);
    tzset();
    h = ti.tm_hour;
    m = ti.tm_min;
    s = ti.tm_sec;
    valid = true;
  }
  drawClockHmIncremental(PINFO_X, PINFO_TIME_Y, 2, TFT_WHITE, TFT_BLACK,
                         h, m,
                         prevPartnerHour, prevPartnerMinute,
                         prevPartnerTimeValid, valid);
  prevPartnerSecond = s;
}

// Self information strip
void drawSelfInfoStrip() {
  tft.fillRect(SINFO_X, 0, SINFO_W, TOP_STRIP_H, warmBg());
  uint16_t bg = warmBg();
  char label[16];
  snprintf(label, sizeof(label), "Me:%s", kTimezones[tzIndex].label);
  tft.setTextDatum(lgfx::top_left);
  tft.setTextSize(1);
  tft.setTextColor(TFT_CYAN, bg);
  tft.drawString(label, SINFO_X, SINFO_UTC_Y);
  prevSelfHour = prevSelfMinute = prevSelfSecond = -1;
  prevSelfTimeValid = false;
  drawSelfPanelTimeOnly();
}

void drawSelfPanelTimeOnly() {
  uint16_t bg = warmBg();
  struct tm ti;
  bool valid = getLocalTimeSafe(&ti);
  int h = valid ? ti.tm_hour : 0;
  int m = valid ? ti.tm_min : 0;
  int s = valid ? ti.tm_sec : 0;
  drawClockHmIncremental(SINFO_X, SINFO_TIME_Y, 1, TFT_WHITE, bg,
                         h, m,
                         prevSelfHour, prevSelfMinute,
                         prevSelfTimeValid, valid);
  prevSelfSecond = s;
}

// Self emoji
void drawSelfEmoji() {
  tft.fillRect(SELF_PX_BODY, TOP_STRIP_H, 480 - SELF_PX_BODY, 320 - TOP_STRIP_H, warmBg());
  uint16_t emoBg = warmBg();
  float saved = SCALE;
  SCALE = 0.55f;
  switch (myStatus) {
    case ST_FREE: drawHappyFace(SELF_EMO_CX, SELF_EMO_CY); break;
    case ST_BUSY: drawBusyIcon(SELF_EMO_CX, SELF_EMO_CY); break;
    case ST_SLEEPING: drawSleepingZs(SELF_EMO_CX, SELF_EMO_CY, emoBg); break;
    case ST_MISS_YOU: drawHeart(SELF_EMO_CX, SELF_EMO_CY); break;
    case ST_BAD_DAY: drawSadFace(SELF_EMO_CX, SELF_EMO_CY); break;
    default: break;
  }
  SCALE = saved;
  tft.setTextDatum(lgfx::top_center);
  tft.setTextSize(1);
  tft.setTextColor(TFT_LIGHTGREY, emoBg);
  int labelY = SELF_EMO_CY + (int)(52 * 0.55f) + 6;
  tft.drawString(statusText[(int)myStatus], SELF_EMO_CX, labelY);
  tft.setTextDatum(lgfx::top_left);
}

// Partner emoji
void drawPartnerEmoji() {
  tft.fillRect(0, TOP_STRIP_H, DIVIDER_X, 320 - TOP_STRIP_H - 36, TFT_BLACK);
  sleepSceneDrawn = false;
  float saved = SCALE;
  SCALE = 1.2f;
  if (!partnerPresenceKnown) {
    drawCheckingFace(PARTNER_CX, PARTNER_CY);
  } else if (!partnerOnline) {
    drawOfflineFace(PARTNER_CX, PARTNER_CY);
  } else {
    switch (partnerStatus) {
      case ST_FREE: drawHappyFace(PARTNER_CX, PARTNER_CY); break;
      case ST_BUSY: drawBusyIcon(PARTNER_CX, PARTNER_CY); break;
      case ST_SLEEPING: startSleepScene(PARTNER_CX, PARTNER_CY); break;
      case ST_MISS_YOU: drawHeart(PARTNER_CX, PARTNER_CY); break;
      case ST_BAD_DAY: drawSadFace(PARTNER_CX, PARTNER_CY); break;
      default: break;
    }
  }
  SCALE = saved;
}

void drawPartnerLabel() {
  tft.fillRect(0, PARTNER_LABEL_Y - 2, DIVIDER_X, 36, TFT_BLACK);
  tft.setTextSize(3);
  uint16_t col = !partnerPresenceKnown ? tft.color565(120, 180, 255)
                                       : (partnerOnline ? TFT_WHITE : TFT_ORANGE);
  tft.setTextColor(col, TFT_BLACK);
  tft.setTextDatum(lgfx::middle_center);
  const char* text = !partnerPresenceKnown ? "CHECKING"
                                           : (partnerOnline ? statusText[(int)partnerStatus] : "OFFLINE");
  tft.drawString(text, PARTNER_CX, PARTNER_LABEL_Y + 10);
  tft.setTextDatum(lgfx::top_left);
}

// Full home screen draw
void drawEmojiHome() {
  tzListStaticDrawn = false;
  timeEditStaticDrawn = false;
  sleepSceneDrawn = false;

  tft.fillScreen(TFT_BLACK);
  tft.fillRect(DIVIDER_X, 0, 480 - DIVIDER_X, 320, warmBg());

  // Dividers
  tft.drawFastVLine(DIVIDER_X, 0, 320, tft.color565(60, 35, 38));
  tft.drawFastHLine(0, TOP_STRIP_H - 1, DIVIDER_X, TFT_DARKGREY);

  homeWifiIconState = -1;
  drawHomeWiFiIndicator(true);
  drawPartnerInfoStrip();
  drawSelfInfoStrip();
  drawPartnerEmoji();
  drawPartnerLabel();
  drawSelfEmoji();

  homeOverlayDrawn = true;
  partnerStatusDirty = false;
  partnerInfoDirty = false;
  lastHomeSecond = -1;
}

// Home screen tick
void tickEmojiHome() {
  if (partnerStatusDirty) {
    drawPartnerEmoji();
    drawPartnerLabel();
    partnerStatusDirty = false;
  }
  if (partnerInfoDirty) {
    drawPartnerInfoStrip();
    partnerInfoDirty = false;
  }
  time_t now = time(nullptr);
  if (now > 100000 && now != lastHomeSecond) {
    lastHomeSecond = now;
    drawSelfPanelTimeOnly();
    drawPartnerTimeOnly();
    drawHomeWiFiIndicator();
  }
  if (partnerOnline && partnerStatus == ST_SLEEPING && sleepSceneDrawn) {
    updateZzzAnimation(PARTNER_CX, PARTNER_CY);
  }
}


// Times and timezones
void drawTopLeftInfoStatic() {}
void drawTopLeftTimeOnly() {
  drawSelfPanelTimeOnly();
}
void drawSelfPanelText() {
  drawSelfInfoStrip();
}
void drawSelfPanelStatic() {}

void drawTimezoneList() {
  tzListStaticDrawn = true;
  timeEditStaticDrawn = false;
  drawHeader(startupFlow ? "Startup 1/2: TZ" : "Settings: TZ");
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString("Rotate: choose   Press: confirm", 10, 266);
  tft.fillRect(8, 52, tft.width() - 16, 210, TFT_BLACK);
}

void drawTimezoneListRowsOnly() {
  const int yTop = 55, itemH = 52;
  tft.fillRect(8, 52, tft.width() - 16, 210, TFT_BLACK);
  for (int row = 0; row < 4; row++) {
    int idx = tzListIndex - 1 + row;
    if (idx < 0 || idx >= TZ_COUNT) continue;
    int y = yTop + row * itemH;
    bool sel = (idx == tzListIndex);
    uint16_t bg = sel ? TFT_NAVY : TFT_BLACK;
    tft.fillRoundRect(8, y - 2, tft.width() - 16, itemH - 6, 5, bg);
    tft.setTextColor(sel ? TFT_WHITE : TFT_CYAN, bg);
    tft.setTextSize(2);
    tft.drawString(kTimezones[idx].label, 14, y + 2);
    tft.setTextColor(sel ? TFT_YELLOW : TFT_GREEN, bg);
    tft.setTextSize(1);
    tft.drawString(String("Countries: ") + kTimezones[idx].countries, 14, y + 24);
  }
}

void drawTimeEditor() {
  timeEditStaticDrawn = true;
  tzListStaticDrawn = false;
  drawHeader(startupFlow ? "Startup 2/2: Time" : "Settings: Time");
  tft.setTextSize(2);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString(String("TZ: ") + kTimezones[tzIndex].label, 10, 48);
  tft.drawString("Rotate: change", 10, 72);
  tft.drawString("Press: next/save", 10, 96);
  tft.fillRect(10, 112, tft.width() - 20, 60, TFT_BLACK);
  tft.setTextDatum(lgfx::top_left);
}

void drawTimeEditorFieldsOnly() {
  const int x = 16, y = 118, w = 52, h = 40, gap = 8;
  int values[5] = { editYear, editMonth, editDay, editHour, editMinute };
  const char* labels[5] = { "YEAR", "MON", "DAY", "HOUR", "MIN" };
  tft.fillRect(10, 112, tft.width() - 20, 60, TFT_BLACK);
  for (int i = 0; i < 5; i++) {
    int bx = x + i * (w + gap);
    bool sel = (i == editField);
    uint16_t bg = sel ? TFT_DARKCYAN : TFT_BLACK;
    tft.fillRoundRect(bx, y, w, h, 5, bg);
    tft.drawRoundRect(bx, y, w, h, 5, TFT_DARKGREY);
    tft.setTextColor(sel ? TFT_WHITE : TFT_LIGHTGREY, bg);
    tft.setTextDatum(lgfx::top_center);
    tft.setTextSize(1);
    tft.drawString(labels[i], bx + w / 2, y + 3);
    char buf[8];
    if (i == 0) snprintf(buf, sizeof(buf), "%04d", values[i]);
    else snprintf(buf, sizeof(buf), "%02d", values[i]);
    tft.setTextSize(2);
    tft.drawString(buf, bx + w / 2, y + 16);
  }
  tft.setTextDatum(lgfx::top_left);
}

void drawTimeEditorFieldOnly(int i) {
  const int x = 16, y = 118, w = 52, h = 40, gap = 8;
  int values[5] = { editYear, editMonth, editDay, editHour, editMinute };
  const char* labels[5] = { "YEAR", "MON", "DAY", "HOUR", "MIN" };
  if (i < 0 || i > 4) return;
  int bx = x + i * (w + gap);
  bool sel = (i == editField);
  uint16_t bg = sel ? TFT_DARKCYAN : TFT_BLACK;
  tft.fillRoundRect(bx, y, w, h, 5, bg);
  tft.drawRoundRect(bx, y, w, h, 5, TFT_DARKGREY);
  tft.setTextColor(sel ? TFT_WHITE : TFT_LIGHTGREY, bg);
  tft.setTextDatum(lgfx::top_center);
  tft.setTextSize(1);
  tft.drawString(labels[i], bx + w / 2, y + 3);
  char buf[8];
  if (i == 0) snprintf(buf, sizeof(buf), "%04d", values[i]);
  else snprintf(buf, sizeof(buf), "%02d", values[i]);
  tft.setTextSize(2);
  tft.drawString(buf, bx + w / 2, y + 16);
  tft.setTextDatum(lgfx::top_left);
}

// Startup screens
void drawWelcomePage() {
  tzListStaticDrawn = timeEditStaticDrawn = false;
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(lgfx::middle_center);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(3);
  tft.drawString("Welcome to MoodLink", tft.width() / 2, 58);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString("Smart Desktop Companion", tft.width() / 2, 96);

  float saved = SCALE;
  SCALE = 0.7f;
  drawHappyFace(tft.width() / 2 - 56, 175);
  drawHeart(tft.width() / 2 + 56, 175);
  SCALE = saved;

  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString("Press to continue", tft.width() / 2, 280);
  tft.setTextDatum(lgfx::top_left);
}

void drawBootNetworkStatusOnly() {
  tft.fillRect(10, 86, tft.width() - 20, 56, TFT_BLACK);
  bool connected = (WiFi.status() == WL_CONNECTED);
  tft.setTextSize(2);
  if (connected) {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawString("Connected Wi-Fi:", 10, 88);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    String ssid = WiFi.SSID();
    tft.drawString(ssid.length() ? ssid : "(hidden)", 10, 112);
  } else {
    tft.setTextColor(TFT_ORANGE, TFT_BLACK);
    tft.drawString("Wi-Fi not connected", 10, 88);
    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    tft.drawString("You can setup now or skip", 10, 112);
  }
}

void drawBootNetworkButtonsOnly() {
  tft.fillRect(10, 160, tft.width() - 20, 108, TFT_BLACK);
  int bw = tft.width() - 20;
  drawButton(10, 164, bw, 44, "Setup Wi-Fi", bootMenuIndex == 0);
  drawButton(10, 220, bw, 44, "Skip to Home", bootMenuIndex == 1, TFT_DARKGREEN);
}

void drawBootNetworkPage() {
  tzListStaticDrawn = timeEditStaticDrawn = false;
  drawHeader("Startup: Network");
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString("Check current network before entering home.", 10, 48);
  drawBootNetworkStatusOnly();
  drawBootNetworkButtonsOnly();
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString("Rotate: move   Press: confirm", 10, 272);
}

void drawDateTimeMenuDynamicOnly() {
  tft.fillRect(10, 48, tft.width() - 20, 232, TFT_BLACK);

  tft.setTextSize(2);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString("Auto mode:", 10, 50);
  tft.setTextColor(autoTimeEnabled ? TFT_GREEN : TFT_ORANGE, TFT_BLACK);
  tft.drawString(autoTimeEnabled ? "ON" : "OFF", 150, 50);

  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString("Auto mode uses network time + timezone detection.", 10, 74);

  if (autoTimeEnabled) {
    drawButton(10, 108, tft.width() - 20, 44, "Toggle Auto Time", dateTimeMenuIndex == 0);
    drawButton(10, 164, tft.width() - 20, 44, "Back", dateTimeMenuIndex == 1, TFT_DARKGREEN);
  } else {
    drawButton(10, 108, tft.width() - 20, 36, "Toggle Auto Time", dateTimeMenuIndex == 0);
    drawButton(10, 150, tft.width() - 20, 36, "Set Timezone", dateTimeMenuIndex == 1);
    drawButton(10, 192, tft.width() - 20, 36, "Set Date & Time", dateTimeMenuIndex == 2);
    drawButton(10, 234, tft.width() - 20, 36, "Back", dateTimeMenuIndex == 3, TFT_DARKGREEN);
  }
}

void drawDateTimeMenuButtonsOnly(int prevIndex = -1, bool forceFull = false) {
  auto drawOne = [&](int idx) {
    if (autoTimeEnabled) {
      if (idx == 0) drawButton(10, 108, tft.width() - 20, 44, "Toggle Auto Time", dateTimeMenuIndex == 0);
      else if (idx == 1) drawButton(10, 164, tft.width() - 20, 44, "Back", dateTimeMenuIndex == 1, TFT_DARKGREEN);
    } else {
      if (idx == 0) drawButton(10, 108, tft.width() - 20, 36, "Toggle Auto Time", dateTimeMenuIndex == 0);
      else if (idx == 1) drawButton(10, 150, tft.width() - 20, 36, "Set Timezone", dateTimeMenuIndex == 1);
      else if (idx == 2) drawButton(10, 192, tft.width() - 20, 36, "Set Date & Time", dateTimeMenuIndex == 2);
      else if (idx == 3) drawButton(10, 234, tft.width() - 20, 36, "Back", dateTimeMenuIndex == 3, TFT_DARKGREEN);
    }
  };

  if (forceFull || prevIndex < 0) {
    drawDateTimeMenuDynamicOnly();
    return;
  }
  drawOne(prevIndex);
  if (prevIndex != dateTimeMenuIndex) drawOne(dateTimeMenuIndex);
}

void drawDateTimeMenu() {
  tzListStaticDrawn = timeEditStaticDrawn = false;
  drawHeader("Settings: Date & Time");
  drawDateTimeMenuDynamicOnly();

  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString("Rotate: move   Press: confirm", 10, 284);
}

void drawMenuOptionsOnly() {
  tft.fillRect(10, 44, tft.width() - 20, 220, TFT_BLACK);
  const char* options[5] = { "Date & Time", "World clocks", "Wi-Fi connection", "Schedules", "Back to Home" };
  for (int i = 0; i < 5; i++) {
    int y2 = 48 + i * 40;
    bool sel = (i == menuIndex);
    uint16_t bg = sel ? ((i == 4) ? TFT_DARKGREEN : TFT_DARKCYAN) : TFT_BLACK;
    tft.fillRoundRect(10, y2 - 4, tft.width() - 20, 32, 6, bg);
    tft.setTextColor(sel ? TFT_WHITE : TFT_LIGHTGREY, bg);
    tft.setTextSize(2);
    tft.drawString(options[i], 18, y2 - 1);
  }
}

void drawMenuOptionOnly(int i) {
  if (i < 0 || i > 4) return;
  const char* options[5] = { "Date & Time", "World clocks", "Wi-Fi connection", "Schedules", "Back to Home" };
  int y2 = 48 + i * 40;
  bool sel = (i == menuIndex);
  uint16_t bg = sel ? ((i == 4) ? TFT_DARKGREEN : TFT_DARKCYAN) : TFT_BLACK;
  tft.fillRoundRect(10, y2 - 4, tft.width() - 20, 32, 6, bg);
  tft.setTextColor(sel ? TFT_WHITE : TFT_LIGHTGREY, bg);
  tft.setTextSize(2);
  tft.drawString(options[i], 18, y2 - 1);
}

// Setting Menu
void drawMenu() {
  tzListStaticDrawn = timeEditStaticDrawn = false;
  drawHeader("Settings Menu");
  drawMenuOptionsOnly();
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString("Rotate: move   Press: confirm", 10, 276);
}

// Schedule page
void drawScheduleButtonsOnly(int prevIndex = -1) {
  auto drawOne = [&](int idx) {
    if (idx == 0) drawButton(10, 48, 150, 36, "Add", scheduleMenuIndex == 0);
    else if (idx == 1) drawButton(170, 48, 150, 36, "Delete", scheduleMenuIndex == 1);
    else if (idx == 2) drawButton(330, 48, 140, 36, "Back", scheduleMenuIndex == 2, TFT_DARKGREEN);
  };
  if (prevIndex < 0) {
    drawOne(0);
    drawOne(1);
    drawOne(2);
    return;
  }
  drawOne(prevIndex);
  if (prevIndex != scheduleMenuIndex) drawOne(scheduleMenuIndex);
}

void drawScheduleListOnly() {
  tft.fillRect(10, 96, tft.width() - 20, 214, TFT_BLACK);
  int used = getUsedScheduleCount();
  tft.setTextSize(1);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString(String("Configured schedules: ") + used, 10, 98);

  if (used == 0) {
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.drawString("No schedules yet. Choose Add.", 10, 118);
    return;
  }

  int y = 118;
  int shown = 0;
  for (int r = 0; r < used && shown < 4; r++, shown++) {
    int slot = getUsedScheduleSlotByRow(r);
    if (slot < 0) continue;
    char l1[64], l2[64];
    formatScheduleSummary(slot, l1, sizeof(l1), l2, sizeof(l2));
    tft.fillRoundRect(10, y - 2, tft.width() - 20, 44, 4, TFT_BLACK);
    tft.drawRoundRect(10, y - 2, tft.width() - 20, 44, 4, TFT_DARKGREY);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setTextSize(1);
    tft.drawString(String("#") + String(r + 1) + " " + l1, 14, y + 2);
    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    tft.drawString(l2, 14, y + 20);
    y += 48;
  }
}

void drawScheduleListPage() {
  tzListStaticDrawn = timeEditStaticDrawn = false;
  drawHeader("Settings: Schedules");
  drawScheduleButtonsOnly(-1);
  tft.drawFastHLine(10, 90, tft.width() - 20, TFT_DARKGREY);
  drawScheduleListOnly();
  tft.setTextSize(1);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.drawString("Rotate: move   Press: confirm", 10, 300);
}

void drawScheduleAddDateFields(const ScheduleItem& d, bool isStart, int activeField) {
  const int x = 16, y = 124, w = 52, h = 40, gap = 8;
  int values[5] = { isStart ? d.sy : d.ey, isStart ? d.sm : d.em, isStart ? d.sd : d.ed, isStart ? d.sh : d.eh, isStart ? d.smin : d.emin };
  const char* labels[5] = { "YEAR", "MON", "DAY", "HOUR", "MIN" };
  tft.fillRect(10, 118, tft.width() - 20, 60, TFT_BLACK);
  for (int i = 0; i < 5; i++) {
    int bx = x + i * (w + gap);
    bool sel = (i == activeField);
    uint16_t bg = sel ? TFT_DARKCYAN : TFT_BLACK;
    tft.fillRoundRect(bx, y, w, h, 5, bg);
    tft.drawRoundRect(bx, y, w, h, 5, TFT_DARKGREY);
    tft.setTextColor(sel ? TFT_WHITE : TFT_LIGHTGREY, bg);
    tft.setTextDatum(lgfx::top_center);
    tft.setTextSize(1);
    tft.drawString(labels[i], bx + w / 2, y + 3);
    char buf[8];
    if (i == 0) snprintf(buf, sizeof(buf), "%04d", values[i]);
    else snprintf(buf, sizeof(buf), "%02d", values[i]);
    tft.setTextSize(2);
    tft.drawString(buf, bx + w / 2, y + 16);
  }
  tft.setTextDatum(lgfx::top_left);
}

void drawScheduleAddDynamicOnly() {
  tft.fillRect(10, 44, tft.width() - 20, 266, TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString("Step " + String(scheduleAddStep + 1) + "/5", 10, 46);

  if (scheduleAddStep == 0) {
    tft.setTextSize(2);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString("Select status", 10, 64);
    tft.setTextSize(1);
    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    tft.drawString("Rotate to choose emoji", 10, 88);
    tft.fillRoundRect(70, 104, tft.width() - 140, 124, 10, TFT_BLACK);
    tft.drawRoundRect(70, 104, tft.width() - 140, 124, 10, TFT_DARKCYAN);
    float saved = SCALE;
    SCALE = 0.8f;
    int cx = tft.width() / 2;
    int cy = 166;
    switch ((MyStatus)scheduleDraft.status) {
      case ST_FREE: drawHappyFace(cx, cy); break;
      case ST_BUSY: drawBusyIcon(cx, cy); break;
      case ST_SLEEPING: drawSleepingZs(cx, cy, TFT_BLACK); break;
      case ST_MISS_YOU: drawHeart(cx, cy); break;
      case ST_BAD_DAY: drawSadFace(cx, cy); break;
      default: break;
    }
    SCALE = saved;
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.setTextSize(1);
    tft.drawString("Press to edit start date/time", 10, 236);
  } else if (scheduleAddStep == 1 || scheduleAddStep == 2) {
    tft.setTextSize(2);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString(scheduleAddStep == 1 ? "Start date/time" : "End date/time", 10, 64);
    tft.setTextSize(1);
    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    tft.drawString("Rotate: change   Press: next field", 10, 90);
    drawScheduleAddDateFields(scheduleDraft, scheduleAddStep == 1, scheduleAddField);
  } else if (scheduleAddStep == 3) {
    char wbuf[24], mbuf[24];
    formatRepeatLabelFromDate(SCH_WEEKLY, scheduleDraft.sy, scheduleDraft.sm, scheduleDraft.sd, wbuf, sizeof(wbuf));
    formatRepeatLabelFromDate(SCH_MONTHLY, scheduleDraft.sy, scheduleDraft.sm, scheduleDraft.sd, mbuf, sizeof(mbuf));
    const char* opts[4] = { "Once", "Daily", wbuf, mbuf };
    tft.setTextSize(2);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString("Repeat mode", 10, 64);
    for (int i = 0; i < 4; i++) {
      drawButton(10, 102 + i * 40, tft.width() - 20, 32, opts[i], scheduleDraft.repeat == i);
    }
  } else {
    char l1[64], l2[64];
    formatScheduleItemSummary(scheduleDraft, l1, sizeof(l1), l2, sizeof(l2));

    tft.setTextSize(2);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString("Save schedule?", 10, 64);
    tft.setTextSize(1);
    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    tft.drawString(l1, 10, 92);
    tft.drawString(l2, 10, 108);
    if (scheduleAddInvalidRange) {
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.drawString("End must be later than start.", 10, 126);
    }
    if (scheduleAddFull) {
      tft.setTextColor(TFT_ORANGE, TFT_BLACK);
      tft.drawString("Schedule list is full. Delete one first.", 10, 142);
    }
    drawButton(10, 156, tft.width() - 20, 40, "Save", scheduleAddAction == 0, TFT_DARKGREEN);
    drawButton(10, 204, tft.width() - 20, 40, "Cancel", scheduleAddAction == 1);
  }
}

void drawScheduleAddPage() {
  tzListStaticDrawn = timeEditStaticDrawn = false;
  drawHeader("Schedule: Add");
  drawScheduleAddDynamicOnly();
  tft.setTextSize(1);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.drawString("Rotate: change   Press: next/confirm", 10, 300);
}

void drawScheduleDeleteRowsOnly() {
  tft.fillRect(10, 48, tft.width() - 20, 252, TFT_BLACK);
  int used = getUsedScheduleCount();
  tft.setTextSize(1);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString("Select schedule to delete", 10, 50);

  if (used == 0) {
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.drawString("No schedules. Choose Back.", 10, 74);
  }

  int y = 78;
  for (int r = 0; r < used; r++) {
    int slot = getUsedScheduleSlotByRow(r);
    if (slot < 0) continue;
    bool sel = (scheduleDeleteIndex == r);
    uint16_t bg = sel ? tft.color565(96, 16, 16) : TFT_BLACK;
    char l1[64], l2[64];
    formatScheduleSummary(slot, l1, sizeof(l1), l2, sizeof(l2));
    tft.fillRoundRect(10, y - 2, tft.width() - 20, 44, 4, bg);
    tft.drawRoundRect(10, y - 2, tft.width() - 20, 44, 4, sel ? TFT_WHITE : TFT_DARKGREY);
    tft.setTextColor(sel ? TFT_WHITE : TFT_CYAN, bg);
    tft.drawString(String("#") + String(r + 1) + " " + l1, 14, y + 2);
    tft.setTextColor(sel ? TFT_WHITE : TFT_LIGHTGREY, bg);
    tft.drawString(l2, 14, y + 20);
    y += 48;
  }

  bool selBack = (scheduleDeleteIndex == used);
  drawButton(10, 268, tft.width() - 20, 34, "Back", selBack, TFT_DARKGREEN);
}

void drawScheduleDeletePage() {
  tzListStaticDrawn = timeEditStaticDrawn = false;
  drawHeader("Schedule: Delete");
  drawScheduleDeleteRowsOnly();
}


// World clock
void resetWorldClockCache() {
  worldLayoutReady = false;
  for (int i = 0; i < 4; i++) worldPrevIdx[i] = worldPrevHour[i] = worldPrevMinute[i] = worldPrevSecond[i] = -1;
}

void drawWorldRowsOnly() {
  time_t now = time(nullptr);
  const int yTop = 56, rowH = 52;
  tft.setTextSize(2);
  int digitW = tft.textWidth("0");
  int colonW = tft.textWidth(":");
  int charH = tft.fontHeight();
  const int hX = 149;
  const int c1X = hX + digitW * 2;
  const int mX = c1X + colonW;
  const int c2X = mX + digitW * 2;
  const int sX = c2X + colonW;

  auto drawField2 = [&](int x, int y, int val, uint16_t fg, uint16_t bg) {
    char b[3];
    snprintf(b, sizeof(b), "%02d", val);
    tft.fillRect(x, y, digitW * 2, charH, bg);
    tft.setTextColor(fg, bg);
    tft.setTextSize(2);
    tft.drawString(b, x, y);
  };

  auto drawDigit1 = [&](int x, int y, int val, uint16_t fg, uint16_t bg) {
    char b[2] = { (char)('0' + val), '\0' };
    tft.fillRect(x, y, digitW, charH, bg);
    tft.setTextColor(fg, bg);
    tft.setTextSize(2);
    tft.drawString(b, x, y);
  };

  for (int row = 0; row < 4; row++) {
    int idx = (worldBaseIndex + row) % TZ_COUNT, y2 = yTop + row * rowH;
    uint16_t bg = (row == 0) ? TFT_NAVY : TFT_BLACK;
    if (!worldLayoutReady || worldPrevIdx[row] != idx) {
      tft.fillRoundRect(8, y2 - 2, tft.width() - 16, rowH - 6, 5, bg);
      tft.setTextSize(2);
      tft.setTextColor(row == 0 ? TFT_WHITE : TFT_CYAN, bg);
      tft.drawString(kTimezones[idx].label, 14, y2 + 2);
      tft.setTextColor(row == 0 ? TFT_YELLOW : TFT_GREEN, bg);
      tft.drawString(":", c1X, y2 + 18);
      tft.drawString(":", c2X, y2 + 18);
      worldPrevIdx[row] = idx;
      worldPrevHour[row] = worldPrevMinute[row] = worldPrevSecond[row] = -1;
    }
    struct tm ti;
    uint16_t fg = (row == 0) ? TFT_YELLOW : TFT_GREEN;
    if (!getTimeInZone(idx, now, &ti)) {
      if (worldPrevHour[row] != -2 || worldPrevMinute[row] != -2 || worldPrevSecond[row] != -2) {
        tft.fillRect(hX, y2 + 18, digitW * 6 + colonW * 2, charH, bg);
        tft.setTextColor(fg, bg);
        tft.setTextSize(2);
        tft.drawString("--", hX, y2 + 18);
        tft.drawString("--", mX, y2 + 18);
        tft.drawString("--", sX, y2 + 18);
        worldPrevHour[row] = worldPrevMinute[row] = worldPrevSecond[row] = -2;
      }
      continue;
    }
    if (worldPrevHour[row] < 0 || worldPrevHour[row] != ti.tm_hour) {
      drawField2(hX, y2 + 18, ti.tm_hour, fg, bg);
      worldPrevHour[row] = ti.tm_hour;
    }
    if (worldPrevMinute[row] < 0 || worldPrevMinute[row] != ti.tm_min) {
      drawField2(mX, y2 + 18, ti.tm_min, fg, bg);
      worldPrevMinute[row] = ti.tm_min;
    }
    if (worldPrevSecond[row] < 0 || worldPrevSecond[row] != ti.tm_sec) {
      int prevSec = worldPrevSecond[row];
      int newTens = ti.tm_sec / 10;
      int prevTens = (prevSec >= 0) ? (prevSec / 10) : -1;
      if (prevSec < 0 || newTens != prevTens) {
        drawDigit1(sX, y2 + 18, newTens, fg, bg);
      }
      drawDigit1(sX + digitW, y2 + 18, ti.tm_sec % 10, fg, bg);
      worldPrevSecond[row] = ti.tm_sec;
    }
  }
  worldLayoutReady = true;
}

void drawWorldView() {
  tzListStaticDrawn = timeEditStaticDrawn = false;
  drawHeader("World Clocks");
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString("Rotate: scroll   Press: menu", 10, 268);
  resetWorldClockCache();
  drawWorldRowsOnly();
}

// Wi-Fi page
void drawWiFiInfoDynamicOnly() {
  tft.fillRect(10, 112, tft.width() - 20, 146, TFT_BLACK);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString("Current Wi-Fi:", 10, 114);
  if (WiFi.status() == WL_CONNECTED) {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    String ssid = WiFi.SSID();
    tft.drawString(ssid.length() ? ssid : "(hidden)", 10, 130);
  } else {
    tft.setTextColor(TFT_ORANGE, TFT_BLACK);
    tft.drawString("Not connected", 10, 130);
  }
  tft.drawFastHLine(10, 144, tft.width() - 20, TFT_DARKGREY);
  int bw = tft.width() - 20;
  drawButton(10, 152, bw, 44, "Start to Connect", wifiMenuIndex == 0);
  drawButton(10, 208, bw, 44, "Back", wifiMenuIndex == 1);
}

void drawWiFiInfoButtonsOnly(int prevIndex = -1) {
  int bw = tft.width() - 20;
  if (prevIndex < 0) {
    drawButton(10, 152, bw, 44, "Start to Connect", wifiMenuIndex == 0);
    drawButton(10, 208, bw, 44, "Back", wifiMenuIndex == 1);
    return;
  }
  if (prevIndex == 0) drawButton(10, 152, bw, 44, "Start to Connect", wifiMenuIndex == 0);
  if (prevIndex == 1) drawButton(10, 208, bw, 44, "Back", wifiMenuIndex == 1);
  if (wifiMenuIndex == 0) drawButton(10, 152, bw, 44, "Start to Connect", true);
  if (wifiMenuIndex == 1) drawButton(10, 208, bw, 44, "Back", true);
}

void drawWiFiInfoPage() {
  tzListStaticDrawn = timeEditStaticDrawn = false;
  drawHeader("Settings: Wi-Fi");
  tft.setTextSize(1);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString("Connect a phone to the hotspot:", 10, 46);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString("  " AP_SSID, 10, 62);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString("Enter your home Wi-Fi and Pairing", 10, 80);
  tft.drawString("Code. Repeat when changing network.", 10, 96);
  drawWiFiInfoDynamicOnly();
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString("Rotate: move   Press: confirm", 10, 264);
}

void drawWiFiConnectingPage() {
  tzListStaticDrawn = timeEditStaticDrawn = false;
  drawHeader("Settings: Wi-Fi");
  tft.setTextDatum(lgfx::middle_center);
  int cx2 = tft.width() / 2;
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString("Hotspot active:", cx2, 90);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(AP_SSID, cx2, 118);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString("Connect your phone to this network,", cx2, 152);
  tft.drawString("then follow the on-screen steps.", cx2, 168);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString("Waiting...", cx2, 210);
  tft.setTextDatum(lgfx::top_left);
  drawButton(10, 250, tft.width() - 20, 44, "Stop and Exit", false);
}

void drawWiFiResultPage(bool success) {
  tzListStaticDrawn = timeEditStaticDrawn = false;
  drawHeader("Settings: Wi-Fi");
  tft.setTextDatum(lgfx::middle_center);
  int cx2 = tft.width() / 2;
  if (success) {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextSize(2);
    tft.drawString("Process Completed", cx2, 110);
    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    tft.setTextSize(1);
    tft.drawString("Device is now connected to Wi-Fi.", cx2, 140);
    tft.drawString("Your partner can now send updates.", cx2, 158);
  } else {
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setTextSize(2);
    tft.drawString("Connection Failed", cx2, 110);
    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    tft.setTextSize(1);
    tft.drawString("Please check your Wi-Fi password", cx2, 140);
    tft.drawString("and try again.", cx2, 158);
  }
  tft.setTextDatum(lgfx::top_left);
  drawButton(10, 200, tft.width() - 20, 44, "Back", true);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString("Press: go back", 10, 258);
}

#endif
