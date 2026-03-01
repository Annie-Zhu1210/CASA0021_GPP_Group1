/*
 * All TFT drawing functions for the device.
 * Existing screens been merged: emojis for status and timezone settings
 * Plus three new screens: drawWiFiInfoPage(), drawWiFiConnectingPage(), and drawWiFiResultPage(bool success)
 * And an overlay indicating the Wi-Fi on the top-right corner: drawWiFiIndicator()
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

// Wi-Fi Status
void drawWiFiIndicator() {
  const int ox = 270, oy = 8, iw = 30, ih = 22;
  tft.fillRect(ox, oy, iw, ih, TFT_BLACK);
  bool connected = (WiFi.status() == WL_CONNECTED);
  uint16_t barCol = connected ? TFT_GREEN : tft.color565(80, 80, 80);

  struct {
    int x, y, w, h;
  } bars[3] = {
    { ox + 0, oy + 14, 7, 8 },
    { ox + 10, oy + 8, 7, 14 },
    { ox + 20, oy + 2, 7, 20 },
  };
  for (int i = 0; i < 3; i++) {
    if (connected) tft.fillRect(bars[i].x, bars[i].y, bars[i].w, bars[i].h, barCol);
    else tft.drawRect(bars[i].x, bars[i].y, bars[i].w, bars[i].h, barCol);
  }
  if (!connected) {
    int cx = ox + iw / 2, cy = oy + ih + 4;
    tft.drawLine(cx - 4, cy - 4, cx + 4, cy + 4, TFT_RED);
    tft.drawLine(cx + 4, cy - 4, cx - 4, cy + 4, TFT_RED);
  }
}

// A Shared header - Wi-Fi icon on the top right corner
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
  uint16_t bg = TFT_BLACK;
  if (selected) bg = (accentCol != 0) ? accentCol : TFT_DARKCYAN;
  tft.fillRoundRect(x, y, w, h, 6, bg);
  tft.drawRoundRect(x, y, w, h, 6, selected ? TFT_WHITE : TFT_DARKGREY);
  tft.setTextColor(selected ? TFT_WHITE : TFT_LIGHTGREY, bg);
  tft.setTextDatum(lgfx::middle_center);
  tft.setTextSize(2);
  tft.drawString(label, x + w / 2, y + h / 2);
  tft.setTextDatum(lgfx::top_left);
}

// Emojis
void clearCenterIconArea(int cx, int cy) {
  tft.fillRect(cx - 110, cy - 110, 220, 220, TFT_BLACK);
}

void drawThickCircle(int x, int y, int r, int thickness, uint16_t color) {
  for (int i = 0; i < thickness; i++) tft.drawCircle(x, y, r - i, color);
}

void drawThickLine(int x0, int y0, int x1, int y1, int thickness, uint16_t color) {
  int half = thickness / 2;
  for (int dx = -half; dx <= half; dx++)
    for (int dy = -half; dy <= half; dy++)
      tft.drawLine(x0 + dx, y0 + dy, x1 + dx, y1 + dy, color);
}

// Status icons
void drawHappyFace(int x, int y) {
  int r = (int)(45 * SCALE);
  int eyeOffsetX = (int)(15 * SCALE);
  int eyeOffsetY = (int)(10 * SCALE);
  int eyeSize = (int)(5 * SCALE);
  tft.fillCircle(x, y, r, TFT_YELLOW);
  drawThickCircle(x, y, r, 2, TFT_BLACK);
  tft.fillCircle(x - eyeOffsetX, y - eyeOffsetY, eyeSize + 1, TFT_BLACK);
  tft.fillCircle(x + eyeOffsetX, y - eyeOffsetY, eyeSize + 1, TFT_BLACK);
  int mouthWidth = (int)(28 * SCALE);
  int mouthHeight = (int)(12 * SCALE);
  int mouthY = (int)(y + 15 * SCALE);
  int prevX = x - mouthWidth, prevY = mouthY;
  for (int i = -mouthWidth; i <= mouthWidth; i++) {
    float t = (float)i / mouthWidth;
    int curveY = mouthY + (int)(mouthHeight * (1 - t * t));
    drawThickLine(prevX, prevY, x + i, curveY, 2, TFT_BLACK);
    prevX = x + i;
    prevY = curveY;
  }
}

void drawSadFace(int x, int y) {
  int r = (int)(45 * SCALE);
  int eyeOffsetX = (int)(15 * SCALE);
  int eyeOffsetY = (int)(10 * SCALE);
  int eyeSize = (int)(5 * SCALE);
  tft.fillCircle(x, y, r, TFT_ORANGE);
  drawThickCircle(x, y, r, 2, TFT_BLACK);
  int leftEyeX = x - eyeOffsetX, rightEyeX = x + eyeOffsetX;
  int eyeY = y - eyeOffsetY;
  tft.fillCircle(leftEyeX, eyeY, eyeSize + 1, TFT_BLACK);
  tft.fillCircle(rightEyeX, eyeY, eyeSize + 1, TFT_BLACK);
  int mouthWidth = (int)(28 * SCALE);
  int mouthHeight = (int)(12 * SCALE);
  int mouthY = (int)(y + 32 * SCALE);
  int prevX = x - mouthWidth, prevY = mouthY;
  for (int i = -mouthWidth; i <= mouthWidth; i++) {
    float t = (float)i / mouthWidth;
    int curveY = mouthY - (int)(mouthHeight * (1 - t * t));
    drawThickLine(prevX, prevY, x + i, curveY, 2, TFT_BLACK);
    prevX = x + i;
    prevY = curveY;
  }
  uint16_t tearFill = tft.color565(80, 180, 255);
  uint16_t tearOut = tft.color565(40, 120, 220);
  int tearX = rightEyeX + (int)(4 * SCALE);
  int tearY = eyeY + (int)(12 * SCALE);
  int tearR = (int)(6 * SCALE);
  tft.fillCircle(tearX, tearY, tearR, tearFill);
  tft.fillTriangle(tearX - tearR, tearY, tearX + tearR, tearY,
                   tearX, tearY + tearR * 2, tearFill);
  drawThickCircle(tearX, tearY, tearR, 2, tearOut);
  drawThickLine(tearX - tearR, tearY, tearX, tearY + tearR * 2, 2, tearOut);
  drawThickLine(tearX + tearR, tearY, tearX, tearY + tearR * 2, 2, tearOut);
}

void drawBusyIcon(int x, int y) {
  int r = (int)(45 * SCALE);
  int lineOffset = (int)(32 * SCALE);
  drawThickCircle(x, y, r, 2, TFT_RED);
  drawThickLine(x - lineOffset, y + lineOffset,
                x + lineOffset, y - lineOffset, 2, TFT_RED);
}

void drawHeartShape(int x, int y, float s, uint16_t color) {
  tft.fillCircle(x - (int)(15 * s), y, (int)(15 * s), color);
  tft.fillCircle(x + (int)(15 * s), y, (int)(15 * s), color);
  tft.fillTriangle(x - (int)(30 * s), y, x + (int)(30 * s), y,
                   x, y + (int)(40 * s), color);
}

void drawHeart(int x, int y) {
  float s = SCALE;
  drawHeartShape(x + (int)(2 * s), y + (int)(3 * s), s, tft.color565(130, 0, 25));
  drawHeartShape(x, y, s, tft.color565(230, 20, 60));
  drawHeartShape(x - (int)(2 * s), y - (int)(2 * s), s * 0.86f, tft.color565(255, 70, 110));
  int shineX = x - (int)(10 * s), shineY = y - (int)(6 * s);
  tft.fillCircle(shineX, shineY, (int)(6 * s), tft.color565(255, 180, 200));
  tft.fillCircle(shineX - (int)(2 * s), shineY - (int)(2 * s), (int)(3 * s), tft.color565(255, 230, 235));
  tft.fillCircle(x - (int)(2 * s), y - (int)(14 * s), (int)(2 * s), tft.color565(255, 230, 235));
}

// Sleeping bear animation
void drawZ(float s, int x, int y, int size, uint16_t col) {
  int t = max(1, (int)(2 * s));
  for (int i = 0; i < t; i++) {
    tft.drawLine(x, y + i, x + size, y + i, col);
    tft.drawLine(x + size, y + i, x, y + size + i, col);
    tft.drawLine(x, y + size + i, x + size, y + size + i, col);
  }
}

void drawSleepingBearStatic(int cx, int cy) {
  float s = 0.72f;
  auto U = [&](float v) -> int {
    return max(1, (int)lroundf(v * s));
  };

  uint16_t bedBase = rgb565(70, 120, 255);
  uint16_t bedHi = rgb565(140, 185, 255);
  uint16_t bedShadow = rgb565(18, 20, 28);
  uint16_t pillowBase = rgb565(245, 245, 255);
  uint16_t pillowHi = rgb565(255, 255, 255);
  uint16_t blanketBase = rgb565(55, 165, 175);
  uint16_t blanketHi = rgb565(135, 220, 220);
  uint16_t blanketSh = rgb565(30, 115, 125);
  uint16_t blanketEdge = rgb565(20, 90, 100);
  uint16_t furBase = rgb565(180, 130, 90);
  uint16_t furShadow = rgb565(140, 100, 70);
  uint16_t furHi = rgb565(225, 185, 145);
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

  int bx = cx + U(25), by = cy + U(52), blankW = U(195), blankH = U(90);
  int blankX = bx - blankW / 2, blankY = by - U(38);
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

void eraseOldZzz() {
  for (int i = 0; i < 3; i++)
    if (zPrev[i].w > 0 && zPrev[i].h > 0)
      tft.fillRect(zPrev[i].x, zPrev[i].y, zPrev[i].w, zPrev[i].h, TFT_BLACK);
}

void drawZzzOnly(int cx, int cy) {
  uint16_t zColMid = rgb565(180, 210, 255);
  for (int i = 0; i < 3; i++) {
    float phase = (zFrame + i * 18) / 255.0f;
    int zx = cx + 50 + i * 14;
    int zy = cy - 58 - (int)(phase * 26)
             + (int)(sin((zFrame + i * 20) * 0.05f) * 2);
    int zs = 10 + i * 3;
    uint16_t zc = (i == 0)   ? rgb565(210, 230, 255)
                  : (i == 1) ? zColMid
                             : rgb565(140, 180, 240);
    int pad = 5;
    zPrev[i] = { zx - pad, zy - pad, zs + pad * 2, zs + pad * 2 + 8 };
    drawZ(1.0f, zx, zy, zs, zc);
  }
}

void startSleepScene(int cx, int cy) {
  clearCenterIconArea(cx, cy);
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

// Emoji Home screen
void drawPartnerIconTopRight() {
  int px = 210, py = 22;
  float oldScale = SCALE;
  SCALE = 0.9f;
  if (partnerStatus == ST_MISS_YOU) drawHeart(px, py);
  else tft.fillCircle(px, py, 10, TFT_GREEN);
  SCALE = oldScale;
}

void drawTopLeftInfoStatic() {
  tft.fillRoundRect(8, 8, 150, 48, 5, TFT_NAVY);
  tft.drawRoundRect(8, 8, 150, 48, 5, TFT_DARKGREY);
  tft.setTextColor(TFT_CYAN, TFT_NAVY);
  tft.setTextSize(1);
  tft.drawString(kTimezones[tzIndex].label, 14, 12);
  tft.setTextColor(TFT_WHITE, TFT_NAVY);
  tft.setTextSize(2);
  tft.drawString("--:--:--", 14, 30);
  homeOverlayDrawn = true;
}

void drawTopLeftTimeOnly() {
  if (!homeOverlayDrawn) return;
  struct tm ti;
  tft.fillRect(14, 30, 128, 16, TFT_NAVY);
  if (!getLocalTimeSafe(&ti)) {
    tft.setTextColor(TFT_ORANGE, TFT_NAVY);
    tft.setTextSize(1);
    tft.drawString("Not set", 14, 31);
    return;
  }
  char buf[10];
  strftime(buf, sizeof(buf), "%H:%M:%S", &ti);
  tft.setTextColor(TFT_WHITE, TFT_NAVY);
  tft.setTextSize(2);
  tft.drawString(buf, 14, 30);
}

void drawEmojiHome() {
  tzListStaticDrawn = false;
  timeEditStaticDrawn = false;
  tft.fillScreen(TFT_BLACK);
  sleepSceneDrawn = false;
  drawPartnerIconTopRight();
  drawWiFiIndicator();
  drawTopLeftInfoStatic();
  drawTopLeftTimeOnly();
  int cx = tft.width() / 2;
  int cy = tft.height() / 2 - 20;
  clearCenterIconArea(cx, cy);
  switch (myStatus) {
    case ST_FREE: drawHappyFace(cx, cy); break;
    case ST_BUSY: drawBusyIcon(cx, cy); break;
    case ST_SLEEPING: startSleepScene(cx, cy); break;
    case ST_MISS_YOU: drawHeart(cx, cy); break;
    case ST_BAD_DAY: drawSadFace(cx, cy); break;
    default: break;
  }
  tft.setTextSize(3);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(lgfx::textdatum_t::middle_center);
  tft.drawString(statusText[(int)myStatus], tft.width() / 2, tft.height() - 40);
  tft.setTextDatum(lgfx::textdatum_t::top_left);
  lastHomeSecond = -1;
}

// Timezone list
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
    bool selected = (idx == tzListIndex);
    uint16_t bg = selected ? TFT_NAVY : TFT_BLACK;
    tft.fillRoundRect(8, y - 2, tft.width() - 16, itemH - 6, 5, bg);
    tft.setTextColor(selected ? TFT_WHITE : TFT_CYAN, bg);
    tft.setTextSize(2);
    tft.drawString(kTimezones[idx].label, 14, y + 2);
    tft.setTextColor(selected ? TFT_YELLOW : TFT_GREEN, bg);
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
    bool selected = (i == editField);
    uint16_t bg = selected ? TFT_DARKCYAN : TFT_BLACK;
    tft.fillRoundRect(bx, y, w, h, 5, bg);
    tft.drawRoundRect(bx, y, w, h, 5, TFT_DARKGREY);
    tft.setTextColor(selected ? TFT_WHITE : TFT_LIGHTGREY, bg);
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
  tzListStaticDrawn = false;
  timeEditStaticDrawn = false;
  drawHeader("Settings Menu");
  const char* options[5] = {
    "Set timezone",
    "Set date & time",
    "World clocks",
    "Wi-Fi connection",
    "Back to Home",
  };
  for (int i = 0; i < 5; i++) {
    int y = 48 + i * 42;
    bool selected = (i == menuIndex);
    uint16_t bg = TFT_BLACK;
    if (selected) bg = (i == 4) ? TFT_DARKGREEN : TFT_DARKCYAN;
    tft.fillRoundRect(10, y - 4, tft.width() - 20, 34, 6, bg);
    tft.setTextColor(selected ? TFT_WHITE : TFT_LIGHTGREY, bg);
    tft.setTextSize(2);
    tft.drawString(options[i], 18, y);
  }
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString("Rotate: move   Press: confirm", 10, 258);
}

// World clocks
void resetWorldClockCache() {
  worldLayoutReady = false;
  for (int i = 0; i < 4; i++)
    worldPrevIdx[i] = worldPrevHour[i] = worldPrevMinute[i] = worldPrevSecond[i] = -1;
}

void drawWorldRowsOnly() {
  time_t now = time(nullptr);
  const int yTop = 56, rowH = 52;
  for (int row = 0; row < 4; row++) {
    int idx = (worldBaseIndex + row) % TZ_COUNT;
    int y = yTop + row * rowH;
    uint16_t bg = (row == 0) ? TFT_NAVY : TFT_BLACK;
    if (!worldLayoutReady || worldPrevIdx[row] != idx) {
      tft.fillRoundRect(8, y - 2, tft.width() - 16, rowH - 6, 5, bg);
      tft.setTextSize(2);
      tft.setTextColor(row == 0 ? TFT_WHITE : TFT_CYAN, bg);
      tft.drawString(kTimezones[idx].label, 14, y + 2);
      tft.setTextColor(row == 0 ? TFT_YELLOW : TFT_GREEN, bg);
      tft.drawString(":", 173, y + 18);
      tft.drawString(":", 209, y + 18);
      worldPrevIdx[row] = idx;
      worldPrevHour[row] = worldPrevMinute[row] = worldPrevSecond[row] = -1;
    }
    struct tm ti;
    uint16_t fg = (row == 0) ? TFT_YELLOW : TFT_GREEN;
    if (!getTimeInZone(idx, now, &ti)) {
      tft.setTextColor(fg, bg);
      tft.setTextSize(2);
      tft.drawString("--", 149, y + 18);
      tft.drawString("--", 185, y + 18);
      tft.drawString("--", 221, y + 18);
      continue;
    }
    auto drawField = [&](int val, int& prev, int x) {
      if (val == prev) return;
      char b[3];
      snprintf(b, sizeof(b), "%02d", val);
      tft.fillRect(x, y + 18, 22, 18, bg);
      tft.setTextColor(fg, bg);
      tft.setTextSize(2);
      tft.drawString(b, x, y + 18);
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

// Wi-Fi pages
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
  tft.drawString("Settings are saved automatically -", 10, 114);
  tft.drawString("you only need to do this once.", 10, 130);
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
  int cx = tft.width() / 2;
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString("Hotspot active:", cx, 90);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(AP_SSID, cx, 118);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString("Connect your phone to this network,", cx, 152);
  tft.drawString("then follow the on-screen steps.", cx, 168);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString("Waiting...", cx, 210);
  tft.setTextDatum(lgfx::top_left);
}

void drawWiFiResultPage(bool success) {
  tzListStaticDrawn = timeEditStaticDrawn = false;
  drawHeader("Settings: Wi-Fi");
  tft.setTextDatum(lgfx::middle_center);
  int cx = tft.width() / 2;
  if (success) {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextSize(2);
    tft.drawString("Process Completed", cx, 110);
    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    tft.setTextSize(1);
    tft.drawString("Device is now connected to Wi-Fi.", cx, 140);
    tft.drawString("Your partner can now send updates.", cx, 158);
  } else {
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setTextSize(2);
    tft.drawString("Connection Failed", cx, 110);
    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    tft.setTextSize(1);
    tft.drawString("Please check your Wi-Fi password", cx, 140);
    tft.drawString("and try again.", cx, 158);
  }
  tft.setTextDatum(lgfx::top_left);
  drawButton(10, 200, tft.width() - 20, 44, "Back", true);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString("Press: go back", 10, 258);
}

#endif