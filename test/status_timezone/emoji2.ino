/* ================= EMOJI DRAW HELPERS ================= */
uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) { return tft.color565(r, g, b); }

void clearCenterIconArea(int cx, int cy) {
  const int w = 220;
  const int h = 220;
  tft.fillRect(cx - w / 2, cy - h / 2, w, h, TFT_BLACK);
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
  int mouthY = (int)(y + (15 * SCALE));

  int prevX = x - mouthWidth;
  int prevY = mouthY;
  for (int i = -mouthWidth; i <= mouthWidth; i++) {
    float t = (float)i / mouthWidth;
    int curveY = mouthY + (int)(mouthHeight * (1 - t * t));
    int drawX = x + i;
    drawThickLine(prevX, prevY, drawX, curveY, 2, TFT_BLACK);
    prevX = drawX;
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

  int leftEyeX = x - eyeOffsetX;
  int rightEyeX = x + eyeOffsetX;
  int eyeY = y - eyeOffsetY;

  tft.fillCircle(leftEyeX, eyeY, eyeSize + 1, TFT_BLACK);
  tft.fillCircle(rightEyeX, eyeY, eyeSize + 1, TFT_BLACK);

  int mouthWidth = (int)(28 * SCALE);
  int mouthHeight = (int)(12 * SCALE);
  int mouthY = (int)(y + (32 * SCALE));

  int prevX = x - mouthWidth;
  int prevY = mouthY;
  for (int i = -mouthWidth; i <= mouthWidth; i++) {
    float t = (float)i / mouthWidth;
    int curveY = mouthY - (int)(mouthHeight * (1 - t * t));
    int drawX = x + i;
    drawThickLine(prevX, prevY, drawX, curveY, 2, TFT_BLACK);
    prevX = drawX;
    prevY = curveY;
  }

  uint16_t tearFill = tft.color565(80, 180, 255);
  uint16_t tearOut = tft.color565(40, 120, 220);
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

void drawHeartShape(int x, int y, float s, uint16_t color) {
  int circleOffset = (int)(15 * s);
  int circleR = (int)(15 * s);
  int triWidth = (int)(30 * s);
  int triHeight = (int)(40 * s);

  tft.fillCircle(x - circleOffset, y, circleR, color);
  tft.fillCircle(x + circleOffset, y, circleR, color);
  tft.fillTriangle(x - triWidth, y, x + triWidth, y, x, y + triHeight, color);
}

void drawHeart(int x, int y) {
  uint16_t shadow = tft.color565(130, 0, 25);
  uint16_t base = tft.color565(230, 20, 60);
  uint16_t inner = tft.color565(255, 70, 110);
  uint16_t gloss1 = tft.color565(255, 180, 200);
  uint16_t gloss2 = tft.color565(255, 230, 235);

  float s = SCALE;
  drawHeartShape(x + (int)(2 * s), y + (int)(3 * s), s, shadow);
  drawHeartShape(x, y, s, base);
  drawHeartShape(x - (int)(2 * s), y - (int)(2 * s), s * 0.86f, inner);

  int shineX = x - (int)(10 * s);
  int shineY = y - (int)(6 * s);
  tft.fillCircle(shineX, shineY, (int)(6 * s), gloss1);
  tft.fillCircle(shineX - (int)(2 * s), shineY - (int)(2 * s), (int)(3 * s), gloss2);
  tft.fillCircle(x - (int)(2 * s), y - (int)(14 * s), (int)(2 * s), gloss2);
}

/* ================= SLEEP ANIM ================= */
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

  int bedY = cy + U(70);
  int bedW = U(230);
  int bedH = U(62);

  tft.fillRoundRect(cx - bedW / 2 + U(3), bedY - bedH / 2 + U(4), bedW, bedH, U(22), bedShadow);
  tft.fillRoundRect(cx - bedW / 2, bedY - bedH / 2, bedW, bedH, U(22), bedBase);
  tft.fillRoundRect(cx - bedW / 2 + U(10), bedY - bedH / 2 + U(10), bedW - U(20), U(14), U(16), bedHi);

  int pillowX = cx - U(70);
  int pillowY = cy + U(18);
  int pillowW = U(130);
  int pillowH = U(62);

  tft.fillRoundRect(pillowX - pillowW / 2 + U(3), pillowY - pillowH / 2 + U(3), pillowW, pillowH, U(22), bedShadow);
  tft.fillRoundRect(pillowX - pillowW / 2, pillowY - pillowH / 2, pillowW, pillowH, U(22), pillowBase);
  tft.fillRoundRect(pillowX - pillowW / 2 + U(10), pillowY - pillowH / 2 + U(10), pillowW - U(20), U(16), U(14), pillowHi);

  int hx = pillowX - U(4);
  int hy = pillowY - U(4);
  int headR = U(28);

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

  int bx = cx + U(25);
  int by = cy + U(52);
  int blankW = U(195);
  int blankH = U(90);
  int blankX = bx - blankW / 2;
  int blankY = by - U(38);

  tft.fillRoundRect(blankX + U(4), blankY + U(7), blankW, blankH, U(28), blanketSh);
  tft.fillRoundRect(blankX, blankY, blankW, blankH, U(28), blanketBase);
  tft.drawRoundRect(blankX, blankY, blankW, blankH, U(28), blanketEdge);
  tft.fillRoundRect(blankX + U(10), blankY + U(10), blankW - U(20), U(18), U(14), blanketHi);
  tft.drawArc(blankX + U(70), blankY + U(58), U(32), U(18), 210, 330, blanketHi);
  tft.drawArc(blankX + U(128), blankY + U(68), U(38), U(22), 210, 330, blanketHi);
  tft.drawArc(blankX + U(30), blankY + U(30), U(22), U(16), 40, 140, blanketSh);

  int pawX = blankX + U(125);
  int pawY = blankY + U(24);
  int pawR = U(10);

  tft.fillCircle(pawX + U(2), pawY + U(2), pawR, furShadow);
  tft.fillCircle(pawX, pawY, pawR, furBase);

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
    int zx = cx + (int)(50 * s) + (int)(i * (14 * s));
    int zy = cy - (int)(58 * s) - (int)(phase * (26 * s)) + (int)(sin((zFrame + i * 20) * 0.05f) * (2 * s));
    int zs = (int)((10 + i * 3) * s);

    uint16_t zc = (i == 0) ? rgb565(210, 230, 255)
                 : (i == 1) ? zColMid
                 : rgb565(140, 180, 240);

    int pad = 5;
    zPrev[i] = {zx - pad, zy - pad, zs + pad * 2, zs + pad * 2 + 8};
    drawZ(s, zx, zy, zs, zc);
  }
}

void startSleepScene(int cx, int cy) {
  clearCenterIconArea(cx, cy);
  drawSleepingBearStatic(cx, cy);
  for (int i = 0; i < 3; i++) zPrev[i] = {0, 0, 0, 0};
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

/* ================= EMOJI HOME ================= */
void drawPartnerIconTopRight() {
  int px = tft.width() - 70;
  int py = 22;

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
  if (!getLocalTimeSafe(&ti)) {
    tft.fillRect(14, 30, 128, 16, TFT_NAVY);
    tft.setTextColor(TFT_ORANGE, TFT_NAVY);
    tft.setTextSize(1);
    tft.drawString("Not set", 14, 31);
    return;
  }

  char buf[10];
  strftime(buf, sizeof(buf), "%H:%M:%S", &ti);
  tft.fillRect(14, 30, 128, 16, TFT_NAVY);
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
  drawTopLeftInfoStatic();
  drawTopLeftTimeOnly();

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
  tft.setTextDatum(lgfx::textdatum_t::top_left);

  lastHomeSecond = -1;
}
