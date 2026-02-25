/* ================= SETTINGS UI ================= */
void drawHeader(const char* title) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(lgfx::top_left);
  tft.drawString(title, 10, 10);
  tft.drawFastHLine(10, 36, tft.width() - 20, TFT_DARKGREY);
}

void resetWorldClockCache() {
  worldLayoutReady = false;
  for (int i = 0; i < 4; i++) {
    worldPrevIdx[i] = -1;
    worldPrevHour[i] = -1;
    worldPrevMinute[i] = -1;
    worldPrevSecond[i] = -1;
  }
}

void drawTimezoneList() {
  tzListStaticDrawn = true;
  timeEditStaticDrawn = false;
  drawHeader(startupFlow ? "Startup 1/2: TZ" : "Settings: TZ");

  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString("Rotate: choose timezone", 10, 266);
  tft.drawString("Press: next", 10, 286);

  tft.fillRect(8, 52, tft.width() - 16, 210, TFT_BLACK);
}

void drawTimezoneListRowsOnly() {
  int yTop = 55;
  int itemH = 52;

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
  const int x = 16;
  const int y = 118;
  const int w = 52;
  const int h = 40;
  const int gap = 8;

  int values[5] = {editYear, editMonth, editDay, editHour, editMinute};
  const char* labels[5] = {"YEAR", "MON", "DAY", "HOUR", "MIN"};

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

void drawMenu() {
  tzListStaticDrawn = false;
  timeEditStaticDrawn = false;
  drawHeader("Settings Menu");

  const char* options[3] = {
      "Set timezone",
      "Set date & time",
      "World clocks",
  };

  for (int i = 0; i < 3; i++) {
    int y = 72 + i * 44;
    bool selected = (i == menuIndex);
    uint16_t bg = selected ? TFT_DARKCYAN : TFT_BLACK;
    tft.fillRoundRect(10, y - 4, tft.width() - 20, 34, 6, bg);
    tft.setTextColor(selected ? TFT_WHITE : TFT_LIGHTGREY, bg);
    tft.setTextSize(2);
    tft.drawString(options[i], 18, y);
  }

  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString("Rotate: move  Press: confirm", 10, 226);
}

void drawWorldRowsOnly() {
  time_t now = time(nullptr);
  int yTop = 56;
  int rowH = 52;

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
      worldPrevHour[row] = -1;
      worldPrevMinute[row] = -1;
      worldPrevSecond[row] = -1;
    }

    struct tm ti;
    uint16_t fg = row == 0 ? TFT_YELLOW : TFT_GREEN;
    if (!getTimeInZone(idx, now, &ti)) {
      tft.setTextColor(fg, bg);
      tft.setTextSize(2);
      tft.drawString("--", 149, y + 18);
      tft.drawString("--", 185, y + 18);
      tft.drawString("--", 221, y + 18);
      continue;
    }

    if (ti.tm_hour != worldPrevHour[row]) {
      char b[3];
      snprintf(b, sizeof(b), "%02d", ti.tm_hour);
      tft.fillRect(149, y + 18, 22, 18, bg);
      tft.setTextColor(fg, bg);
      tft.setTextSize(2);
      tft.drawString(b, 149, y + 18);
      worldPrevHour[row] = ti.tm_hour;
    }
    if (ti.tm_min != worldPrevMinute[row]) {
      char b[3];
      snprintf(b, sizeof(b), "%02d", ti.tm_min);
      tft.fillRect(185, y + 18, 22, 18, bg);
      tft.setTextColor(fg, bg);
      tft.setTextSize(2);
      tft.drawString(b, 185, y + 18);
      worldPrevMinute[row] = ti.tm_min;
    }
    if (ti.tm_sec != worldPrevSecond[row]) {
      char b[3];
      snprintf(b, sizeof(b), "%02d", ti.tm_sec);
      tft.fillRect(221, y + 18, 22, 18, bg);
      tft.setTextColor(fg, bg);
      tft.setTextSize(2);
      tft.drawString(b, 221, y + 18);
      worldPrevSecond[row] = ti.tm_sec;
    }
  }

  worldLayoutReady = true;
}

void drawWorldView() {
  tzListStaticDrawn = false;
  timeEditStaticDrawn = false;
  drawHeader("World Clocks");
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString("Rotate: scroll  Press: menu", 10, 268);
  resetWorldClockCache();
  drawWorldRowsOnly();
}
