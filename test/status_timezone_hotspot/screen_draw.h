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

void drawHomeWiFiIndicator() {
  tft.fillRect(WIFI_IX, WIFI_IY, WIFI_IW, WIFI_IH, TFT_BLACK);
  bool connected = (WiFi.status() == WL_CONNECTED);
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

void drawWiFiIndicator() {
  const int iw = 30, ih = 22, ox = tft.width() - iw - 8, oy = 8;
  tft.fillRect(ox, oy, iw, ih, TFT_BLACK);
  bool connected = (WiFi.status() == WL_CONNECTED);
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
  drawWiFiIndicator();
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
void drawPartnerInfoStrip() {
  tft.fillRect(PINFO_X, 0, PINFO_W, TOP_STRIP_H, TFT_BLACK);
  char label[16];
  snprintf(label, sizeof(label), "P: %s", kTimezones[partnerTzIndex].label);
  tft.setTextDatum(lgfx::top_left);
  tft.setTextSize(1);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString(label, PINFO_X, PINFO_UTC_Y);
  char timeBuf[10] = "0:00:00";
  if (partnerTimeValid && partnerEpoch > 100000) {
    struct tm ti;
    const char* restore = kTimezones[tzIndex].posix;
    setenv("TZ", kTimezones[partnerTzIndex].posix, 1);
    tzset();
    time_t ep = partnerEpoch;
    localtime_r(&ep, &ti);
    setenv("TZ", restore, 1);
    tzset();
    strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", &ti);
  }
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(timeBuf, PINFO_X, PINFO_TIME_Y);
}

void drawPartnerTimeOnly() {
  tft.fillRect(PINFO_X, PINFO_TIME_Y, PINFO_W, 20, TFT_BLACK);
  char timeBuf[10] = "0:00:00";
  if (partnerTimeValid && partnerEpoch > 100000) {
    struct tm ti;
    const char* restore = kTimezones[tzIndex].posix;
    setenv("TZ", kTimezones[partnerTzIndex].posix, 1);
    tzset();
    time_t ep = partnerEpoch;
    localtime_r(&ep, &ti);
    setenv("TZ", restore, 1);
    tzset();
    strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", &ti);
  }
  tft.setTextDatum(lgfx::top_left);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(timeBuf, PINFO_X, PINFO_TIME_Y);
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
  struct tm ti;
  if (getLocalTimeSafe(&ti)) {
    char buf[10];
    strftime(buf, sizeof(buf), "%H:%M:%S", &ti);
    tft.setTextColor(TFT_WHITE, bg);
    tft.drawString(buf, SINFO_X, SINFO_TIME_Y);
  } else {
    tft.setTextColor(TFT_ORANGE, bg);
    tft.drawString("0:00:00", SINFO_X, SINFO_TIME_Y);
  }
}

void drawSelfPanelTimeOnly() {
  tft.fillRect(SINFO_X, SINFO_TIME_Y, SINFO_W, 12, warmBg());
  uint16_t bg = warmBg();
  tft.setTextDatum(lgfx::top_left);
  tft.setTextSize(1);
  struct tm ti;
  if (getLocalTimeSafe(&ti)) {
    char buf[10];
    strftime(buf, sizeof(buf), "%H:%M:%S", &ti);
    tft.setTextColor(TFT_WHITE, bg);
    tft.drawString(buf, SINFO_X, SINFO_TIME_Y);
  } else {
    tft.setTextColor(TFT_ORANGE, bg);
    tft.drawString("0:00:00", SINFO_X, SINFO_TIME_Y);
  }
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
  switch (partnerStatus) {
    case ST_FREE: drawHappyFace(PARTNER_CX, PARTNER_CY); break;
    case ST_BUSY: drawBusyIcon(PARTNER_CX, PARTNER_CY); break;
    case ST_SLEEPING: startSleepScene(PARTNER_CX, PARTNER_CY); break;
    case ST_MISS_YOU: drawHeart(PARTNER_CX, PARTNER_CY); break;
    case ST_BAD_DAY: drawSadFace(PARTNER_CX, PARTNER_CY); break;
    default: break;
  }
  SCALE = saved;
}

void drawPartnerLabel() {
  tft.fillRect(0, PARTNER_LABEL_Y - 2, DIVIDER_X, 36, TFT_BLACK);
  tft.setTextSize(3);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(lgfx::middle_center);
  tft.drawString(statusText[(int)partnerStatus], PARTNER_CX, PARTNER_LABEL_Y + 10);
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

  drawHomeWiFiIndicator();
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
  if (partnerStatus == ST_SLEEPING && sleepSceneDrawn) {
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


// Setting Menu
void drawMenu() {
  tzListStaticDrawn = timeEditStaticDrawn = false;
  drawHeader("Settings Menu");
  const char* options[5] = { "Set timezone", "Set date & time", "World clocks", "Wi-Fi connection", "Back to Home" };
  for (int i = 0; i < 5; i++) {
    int y2 = 48 + i * 42;
    bool sel = (i == menuIndex);
    uint16_t bg = sel ? ((i == 4) ? TFT_DARKGREEN : TFT_DARKCYAN) : TFT_BLACK;
    tft.fillRoundRect(10, y2 - 4, tft.width() - 20, 34, 6, bg);
    tft.setTextColor(sel ? TFT_WHITE : TFT_LIGHTGREY, bg);
    tft.setTextSize(2);
    tft.drawString(options[i], 18, y2);
  }
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString("Rotate: move   Press: confirm", 10, 258);
}


// World clock
void resetWorldClockCache() {
  worldLayoutReady = false;
  for (int i = 0; i < 4; i++) worldPrevIdx[i] = worldPrevHour[i] = worldPrevMinute[i] = worldPrevSecond[i] = -1;
}

void drawWorldRowsOnly() {
  time_t now = time(nullptr);
  const int yTop = 56, rowH = 52;
  for (int row = 0; row < 4; row++) {
    int idx = (worldBaseIndex + row) % TZ_COUNT, y2 = yTop + row * rowH;
    uint16_t bg = (row == 0) ? TFT_NAVY : TFT_BLACK;
    if (!worldLayoutReady || worldPrevIdx[row] != idx) {
      tft.fillRoundRect(8, y2 - 2, tft.width() - 16, rowH - 6, 5, bg);
      tft.setTextSize(2);
      tft.setTextColor(row == 0 ? TFT_WHITE : TFT_CYAN, bg);
      tft.drawString(kTimezones[idx].label, 14, y2 + 2);
      tft.setTextColor(row == 0 ? TFT_YELLOW : TFT_GREEN, bg);
      tft.drawString(":", 173, y2 + 18);
      tft.drawString(":", 209, y2 + 18);
      worldPrevIdx[row] = idx;
      worldPrevHour[row] = worldPrevMinute[row] = worldPrevSecond[row] = -1;
    }
    struct tm ti;
    uint16_t fg = (row == 0) ? TFT_YELLOW : TFT_GREEN;
    if (!getTimeInZone(idx, now, &ti)) {
      tft.setTextColor(fg, bg);
      tft.setTextSize(2);
      tft.drawString("--", 149, y2 + 18);
      tft.drawString("--", 185, y2 + 18);
      tft.drawString("--", 221, y2 + 18);
      continue;
    }
    auto drawField = [&](int val, int& prev, int x2) {
      if (val == prev) return;
      char b[3];
      snprintf(b, sizeof(b), "%02d", val);
      tft.fillRect(x2, y2 + 18, 22, 18, bg);
      tft.setTextColor(fg, bg);
      tft.setTextSize(2);
      tft.drawString(b, x2, y2 + 18);
      prev = val;
    };
    drawField(ti.tm_hour, worldPrevHour[row], 149);
    drawField(ti.tm_min, worldPrevMinute[row], 185);
    drawField(ti.tm_sec, worldPrevSecond[row], 221);
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
