#include <LovyanGFX.hpp>
#include <Preferences.h>
#include <sys/time.h>
#include <time.h>

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

/* ================= KY-040 PIN CONFIG ================= */
static constexpr int ENC_CLK = 16;
static constexpr int ENC_DT  = 17;
static constexpr int ENC_SW  = 18;

/* ================= INPUT TIMING ================= */
static constexpr uint32_t LONG_PRESS_MS = 5000;
static constexpr uint32_t DEBOUNCE_MS = 35;

/* ================= STORAGE ================= */
static const char* PREF_NS = "tz-manual";
static const char* KEY_TZ_INDEX = "tz_idx";

class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ILI9488 _panel;
  lgfx::Bus_Parallel8 _bus;

 public:
  LGFX() {
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

    {
      auto cfg = _panel.config();
      cfg.pin_cs = TFT_CS;
      cfg.pin_rst = TFT_RST;
      cfg.pin_busy = -1;
      cfg.memory_width = 320;
      cfg.memory_height = 480;
      cfg.panel_width = 320;
      cfg.panel_height = 480;
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

struct TimezoneOption {
  const char* iana;
  const char* posix;
  const char* countries;
};

TimezoneOption kTimezones[] = {
    {"UTC-12", "UTC+12", "USA (Baker Is.)"},
    {"UTC-11", "UTC+11", "USA (Samoa) / Niue"},
    {"UTC-10", "UTC+10", "USA (Hawaii)"},
    {"UTC-09", "UTC+9", "USA (Alaska)"},
    {"UTC-08", "UTC+8", "USA / Canada (West)"},
    {"UTC-07", "UTC+7", "USA / Canada (Mountain)"},
    {"UTC-06", "UTC+6", "USA / Mexico"},
    {"UTC-05", "UTC+5", "USA / Canada (East)"},
    {"UTC-04", "UTC+4", "Canada (Atlantic) / Bolivia"},
    {"UTC-03", "UTC+3", "Brazil / Argentina"},
    {"UTC-02", "UTC+2", "Brazil (Fernando de Noronha)"},
    {"UTC-01", "UTC+1", "Portugal (Azores) / Cape Verde"},
    {"UTC+00", "UTC0", "UK / Portugal / Ghana"},
    {"UTC+01", "UTC-1", "France / Germany / Spain"},
    {"UTC+02", "UTC-2", "Egypt / South Africa / Greece"},
    {"UTC+03", "UTC-3", "Russia / Saudi Arabia / Kenya"},
    {"UTC+04", "UTC-4", "UAE / Oman / Georgia"},
    {"UTC+05", "UTC-5", "Pakistan / Uzbekistan"},
    {"UTC+06", "UTC-6", "Bangladesh / Kazakhstan"},
    {"UTC+07", "UTC-7", "Thailand / Vietnam / Indonesia"},
    {"UTC+08", "UTC-8", "China / Singapore / Malaysia"},
    {"UTC+09", "UTC-9", "Japan / South Korea"},
    {"UTC+10", "UTC-10", "Australia / Papua New Guinea"},
    {"UTC+11", "UTC-11", "Solomon Islands / New Caledonia"},
    {"UTC+12", "UTC-12", "New Zealand / Fiji"},
    {"UTC+13", "UTC-13", "Samoa / Tonga"},
    {"UTC+14", "UTC-14", "Kiribati (Line Islands)"},
};

static constexpr int TZ_COUNT = sizeof(kTimezones) / sizeof(kTimezones[0]);

enum ScreenState {
  SCREEN_TZ_LIST = 0,
  SCREEN_TIME_EDIT,
  SCREEN_HOME,
  SCREEN_MENU,
  SCREEN_WORLD_VIEW,
  SCREEN_MESSAGE
};

ScreenState screenState = SCREEN_TZ_LIST;

bool startupFlow = true;
int tzIndex = 0;
int tzListIndex = 0;
int worldBaseIndex = 0;
int menuIndex = 0;

int editYear = 2026;
int editMonth = 1;
int editDay = 1;
int editHour = 12;
int editMinute = 0;
int editField = 0;

String messageLine1;
String messageLine2;
uint32_t messageUntilMs = 0;

volatile int32_t encDelta = 0;
volatile uint8_t encPrevState = 0;

bool homeClockLayoutReady = false;
bool homeClockWasInvalid = true;
int homePrevYear = -1;
int homePrevMonth = -1;
int homePrevDay = -1;
int homePrevHour = -1;
int homePrevMinute = -1;
int homePrevSecond = -1;

bool worldLayoutReady = false;
int worldPrevIdx[4] = {-1, -1, -1, -1};
int worldPrevHour[4] = {-1, -1, -1, -1};
int worldPrevMinute[4] = {-1, -1, -1, -1};
int worldPrevSecond[4] = {-1, -1, -1, -1};

/* ================= INPUT ================= */
void IRAM_ATTR onEncClkChange() {
  static const int8_t trans[16] = {
      0, -1,  1,  0,
      1,  0,  0, -1,
     -1,  0,  0,  1,
      0,  1, -1,  0
  };

  uint8_t curr = (uint8_t)((digitalRead(ENC_CLK) << 1) | digitalRead(ENC_DT));
  uint8_t idx = (uint8_t)((encPrevState << 2) | curr);
  encDelta += trans[idx];
  encPrevState = curr;
}

bool buttonPressedEvent() {
  static bool lastStable = HIGH;
  static bool lastRead = HIGH;
  static uint32_t lastChangeMs = 0;

  bool r = digitalRead(ENC_SW);
  uint32_t now = millis();

  if (r != lastRead) {
    lastRead = r;
    lastChangeMs = now;
  }

  if (now - lastChangeMs >= DEBOUNCE_MS) {
    if (lastStable != lastRead) {
      lastStable = lastRead;
      if (lastStable == LOW) return true;
    }
  }

  return false;
}

bool buttonLongPressEvent(uint32_t holdMs) {
  static bool wasDown = false;
  static bool longFired = false;
  static uint32_t downSince = 0;

  bool down = (digitalRead(ENC_SW) == LOW);
  uint32_t now = millis();

  if (down && !wasDown) {
    wasDown = true;
    longFired = false;
    downSince = now;
  }

  if (down && !longFired && (now - downSince >= holdMs)) {
    longFired = true;
    return true;
  }

  if (!down && wasDown) {
    wasDown = false;
    longFired = false;
  }

  return false;
}

int consumeEncoderDelta() {
  int32_t q;
  noInterrupts();
  q = encDelta;
  int32_t step = q / 4;
  encDelta = q % 4;
  interrupts();

  if (step > 0) return 1;
  if (step < 0) return -1;
  return 0;  
}

/* ================= TIME ================= */
void applyTimezoneByIndex(int idx) {
  if (idx < 0) idx = 0;
  if (idx >= TZ_COUNT) idx = TZ_COUNT - 1;
  tzIndex = idx;
  setenv("TZ", kTimezones[tzIndex].posix, 1);
  tzset();
}

void setSystemClock(time_t epoch) {
  struct timeval tv;
  tv.tv_sec = epoch;
  tv.tv_usec = 0;
  settimeofday(&tv, nullptr);
}

void saveTimezoneSetting() {
  Preferences pref;
  pref.begin(PREF_NS, false);
  pref.putInt(KEY_TZ_INDEX, tzIndex);
  pref.end();
}

void loadTimezoneSetting() {
  Preferences pref;
  pref.begin(PREF_NS, true);
  int idx = pref.getInt(KEY_TZ_INDEX, 0);
  pref.end();
  applyTimezoneByIndex(idx);
}

bool getLocalTimeSafe(struct tm* out) {
  time_t now = time(nullptr);
  if (now <= 100000) return false;
  localtime_r(&now, out);
  return true;
}

int daysInMonth(int year, int month) {
  if (month == 2) {
    bool leap = ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
    return leap ? 29 : 28;
  }
  if (month == 4 || month == 6 || month == 9 || month == 11) return 30;
  return 31;
}

void loadEditorFromCurrentOrDefault() {
  struct tm ti;
  if (getLocalTimeSafe(&ti)) {
    editYear = ti.tm_year + 1900;
    editMonth = ti.tm_mon + 1;
    editDay = ti.tm_mday;
    editHour = ti.tm_hour;
    editMinute = ti.tm_min;
  } else {
    editYear = 2026;
    editMonth = 1;
    editDay = 1;
    editHour = 12;
    editMinute = 0;
  }
  editField = 0;
}

void applyManualDateTime() {
  int maxDay = daysInMonth(editYear, editMonth);
  if (editDay > maxDay) editDay = maxDay;

  struct tm tmv = {};
  tmv.tm_year = editYear - 1900;
  tmv.tm_mon = editMonth - 1;
  tmv.tm_mday = editDay;
  tmv.tm_hour = editHour;
  tmv.tm_min = editMinute;
  tmv.tm_sec = 0;
  tmv.tm_isdst = -1;

  time_t epoch = mktime(&tmv);
  if (epoch > 100000) {
    setSystemClock(epoch);
  }
}

String formatClockInZone(int idx, time_t baseNow) {
  if (baseNow <= 100000) return "--:--";

  const char* restore = kTimezones[tzIndex].posix;
  setenv("TZ", kTimezones[idx].posix, 1);
  tzset();

  struct tm ti;
  localtime_r(&baseNow, &ti);
  char buf[8] = "--:--";
  strftime(buf, sizeof(buf), "%H:%M", &ti);

  setenv("TZ", restore, 1);
  tzset();

  return String(buf);
}

bool getTimeInZone(int idx, time_t baseNow, struct tm* out) {
  if (baseNow <= 100000 || idx < 0 || idx >= TZ_COUNT) return false;

  const char* restore = kTimezones[tzIndex].posix;
  setenv("TZ", kTimezones[idx].posix, 1);
  tzset();
  localtime_r(&baseNow, out);
  setenv("TZ", restore, 1);
  tzset();
  return true;
}

/* ================= UI ================= */
void drawHomeTimeOnly();
void drawWorldRowsOnly();

void resetHomeClockCache() {
  homeClockLayoutReady = false;
  homeClockWasInvalid = true;
  homePrevYear = -1;
  homePrevMonth = -1;
  homePrevDay = -1;
  homePrevHour = -1;
  homePrevMinute = -1;
  homePrevSecond = -1;
}

void drawClockField(int x, int y, int w, const char* text, uint16_t fg, uint16_t bg) {
  tft.fillRect(x, y, w, 22, bg);
  tft.setTextColor(fg, bg);
  tft.setTextSize(2);
  tft.drawString(text, x, y);
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

void drawHeader(const char* title) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(lgfx::top_left);
  tft.drawString(title, 10, 10);
  tft.drawFastHLine(10, 36, tft.width() - 20, TFT_DARKGREY);
}

void drawTimezoneList() {
  drawHeader(startupFlow ? "Startup 1/2: TZ" : "Set Timezone");

  int yTop = 55;
  int itemH = 52;

  for (int row = 0; row < 4; row++) {
    int idx = tzListIndex - 1 + row;
    if (idx < 0 || idx >= TZ_COUNT) continue;

    int y = yTop + row * itemH;
    bool selected = (idx == tzListIndex);
    uint16_t bg = selected ? TFT_NAVY : TFT_BLACK;

    tft.fillRoundRect(8, y - 2, tft.width() - 16, itemH - 6, 5, bg);
    tft.setTextColor(selected ? TFT_WHITE : TFT_CYAN, bg);
    tft.setTextSize(2);
    tft.drawString(kTimezones[idx].iana, 14, y + 2);

    tft.setTextColor(selected ? TFT_YELLOW : TFT_GREEN, bg);
    tft.drawString(String("Countries: ") + kTimezones[idx].countries, 14, y + 24);
  }

  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.setTextSize(2);
  if (startupFlow) {
    tft.drawString("Rotate: choose local timezone", 10, 266);
    tft.drawString("Press: next step", 10, 288);
  } else {
    tft.drawString("Rotate: choose timezone", 10, 266);
    tft.drawString("Press: save and back home", 10, 288);
  }
}

void drawTimeEditor() {
  drawHeader(startupFlow ? "Startup 2/2: Time" : "Set Date & Time");

  tft.setTextSize(2);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString(String("TZ: ") + kTimezones[tzIndex].iana, 10, 48);
  tft.drawString("Rotate: change value", 10, 72);
  tft.drawString("Press: next field / save", 10, 96);

  const int x = 16;
  const int y = 115;
  const int w = 52;
  const int h = 40;
  const int gap = 8;

  int values[5] = {editYear, editMonth, editDay, editHour, editMinute};
  const char* labels[5] = {"YEAR", "MON", "DAY", "HOUR", "MIN"};

  for (int i = 0; i < 5; i++) {
    int bx = x + i * (w + gap);
    bool selected = (i == editField);
    uint16_t bg = selected ? TFT_DARKCYAN : TFT_BLACK;

    tft.fillRoundRect(bx, y, w, h, 5, bg);
    tft.drawRoundRect(bx, y, w, h, 5, TFT_DARKGREY);

    tft.setTextColor(selected ? TFT_WHITE : TFT_LIGHTGREY, bg);
    tft.setTextDatum(lgfx::top_center);
    tft.setTextSize(2);
    tft.drawString(labels[i], bx + w / 2, y + 3);

    char buf[8];
    if (i == 0) snprintf(buf, sizeof(buf), "%04d", values[i]);
    else snprintf(buf, sizeof(buf), "%02d", values[i]);

    tft.setTextSize(2);
    tft.drawString(buf, bx + w / 2, y + 16);
  }

  tft.setTextDatum(lgfx::top_left);
}

void drawHome() {
  drawHeader("Timezone Demo");

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString("Local Time:", 10, 55);

  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString("Local TZ:", 10, 130);
  tft.drawString(kTimezones[tzIndex].iana, 10, 160);

  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString(String("Countries: ") + kTimezones[tzIndex].countries, 10, 192);

  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString("Hold knob 5s to open menu", 10, 220);

  resetHomeClockCache();
  drawHomeTimeOnly();
}

void drawHomeTimeOnly() {
  struct tm ti;
  if (getLocalTimeSafe(&ti)) {
    const int y = 85;

    if (!homeClockLayoutReady || homeClockWasInvalid) {
      tft.fillRect(10, y, tft.width() - 20, 30, TFT_BLACK);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.setTextSize(2);
      tft.drawString("-", 58, y);
      tft.drawString("-", 94, y);
      tft.drawString(":", 166, y);
      tft.drawString(":", 202, y);
      homeClockLayoutReady = true;
      homeClockWasInvalid = false;
    }

    if (ti.tm_year != homePrevYear) {
      char b[5];
      snprintf(b, sizeof(b), "%04d", ti.tm_year + 1900);
      drawClockField(10, y, 46, b, TFT_WHITE, TFT_BLACK);
      homePrevYear = ti.tm_year;
    }
    if (ti.tm_mon != homePrevMonth) {
      char b[3];
      snprintf(b, sizeof(b), "%02d", ti.tm_mon + 1);
      drawClockField(70, y, 22, b, TFT_WHITE, TFT_BLACK);
      homePrevMonth = ti.tm_mon;
    }
    if (ti.tm_mday != homePrevDay) {
      char b[3];
      snprintf(b, sizeof(b), "%02d", ti.tm_mday);
      drawClockField(106, y, 22, b, TFT_WHITE, TFT_BLACK);
      homePrevDay = ti.tm_mday;
    }
    if (ti.tm_hour != homePrevHour) {
      char b[3];
      snprintf(b, sizeof(b), "%02d", ti.tm_hour);
      drawClockField(142, y, 22, b, TFT_WHITE, TFT_BLACK);
      homePrevHour = ti.tm_hour;
    }
    if (ti.tm_min != homePrevMinute) {
      char b[3];
      snprintf(b, sizeof(b), "%02d", ti.tm_min);
      drawClockField(178, y, 22, b, TFT_WHITE, TFT_BLACK);
      homePrevMinute = ti.tm_min;
    }
    if (ti.tm_sec != homePrevSecond) {
      char b[3];
      snprintf(b, sizeof(b), "%02d", ti.tm_sec);
      drawClockField(214, y, 22, b, TFT_WHITE, TFT_BLACK);
      homePrevSecond = ti.tm_sec;
    }
  } else {
    tft.fillRect(10, 85, tft.width() - 20, 30, TFT_BLACK);
    tft.setTextColor(TFT_ORANGE, TFT_BLACK);
    tft.setTextSize(2);
    tft.drawString("Not set yet", 10, 85);
    homeClockWasInvalid = true;
  }
}

void drawMenu() {
  drawHeader("Menu");

  const char* options[3] = {
      "Set timezone",
      "Set date & time",
      "World clocks",
  };

  for (int i = 0; i < 3; i++) {
    int y = 62 + i * 38;
    bool selected = (i == menuIndex);
    uint16_t bg = selected ? TFT_DARKCYAN : TFT_BLACK;

    tft.fillRoundRect(10, y - 4, tft.width() - 20, 30, 6, bg);
    tft.setTextColor(selected ? TFT_WHITE : TFT_LIGHTGREY, bg);
    tft.setTextSize(2);
    tft.drawString(options[i], 18, y);
  }

  tft.setTextSize(2);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString("Rotate: move   Press: confirm", 10, 190);
}

void drawWorldRowsOnly() {
  time_t now = time(nullptr);
  int yTop = 50;
  int rowH = 52;

  for (int row = 0; row < 4; row++) {
    int idx = (worldBaseIndex + row) % TZ_COUNT;
    int y = yTop + row * rowH;
    uint16_t bg = (row == 0) ? TFT_NAVY : TFT_BLACK;

    if (!worldLayoutReady || worldPrevIdx[row] != idx) {
      tft.fillRoundRect(8, y - 2, tft.width() - 16, rowH - 6, 5, bg);

      tft.setTextSize(2);
      tft.setTextColor(row == 0 ? TFT_WHITE : TFT_CYAN, bg);
      tft.drawString(kTimezones[idx].iana, 14, y + 2);

      tft.setTextColor(row == 0 ? TFT_YELLOW : TFT_GREEN, bg);
      tft.drawString(":", 173, y + 16);
      tft.drawString(":", 209, y + 16);

      worldPrevIdx[row] = idx;
      worldPrevHour[row] = -1;
      worldPrevMinute[row] = -1;
      worldPrevSecond[row] = -1;
    }

    struct tm ti;
    if (!getTimeInZone(idx, now, &ti)) {
      drawClockField(149, y + 16, 22, "--", row == 0 ? TFT_YELLOW : TFT_GREEN, bg);
      drawClockField(185, y + 16, 22, "--", row == 0 ? TFT_YELLOW : TFT_GREEN, bg);
      drawClockField(221, y + 16, 22, "--", row == 0 ? TFT_YELLOW : TFT_GREEN, bg);
      continue;
    }

    uint16_t fg = row == 0 ? TFT_YELLOW : TFT_GREEN;
    if (ti.tm_hour != worldPrevHour[row]) {
      char b[3];
      snprintf(b, sizeof(b), "%02d", ti.tm_hour);
      drawClockField(149, y + 16, 22, b, fg, bg);
      worldPrevHour[row] = ti.tm_hour;
    }
    if (ti.tm_min != worldPrevMinute[row]) {
      char b[3];
      snprintf(b, sizeof(b), "%02d", ti.tm_min);
      drawClockField(185, y + 16, 22, b, fg, bg);
      worldPrevMinute[row] = ti.tm_min;
    }
    if (ti.tm_sec != worldPrevSecond[row]) {
      char b[3];
      snprintf(b, sizeof(b), "%02d", ti.tm_sec);
      drawClockField(221, y + 16, 22, b, fg, bg);
      worldPrevSecond[row] = ti.tm_sec;
    }
  }

  worldLayoutReady = true;
}

void drawWorldView() {
  drawHeader("World Clocks");

  tft.setTextSize(2);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString("Rotate: scroll timezones", 10, 262);
  tft.drawString("Press: back to menu", 10, 276);

  resetWorldClockCache();
  drawWorldRowsOnly();
}

void showMessage(const String& l1, const String& l2, uint32_t durationMs) {
  messageLine1 = l1;
  messageLine2 = l2;
  messageUntilMs = millis() + durationMs;
  screenState = SCREEN_MESSAGE;

  drawHeader("Status");
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString(l1, 12, 90);

  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString(l2, 12, 128);
}

/* ================= FLOW ================= */
void adjustEditField(int delta) {
  if (delta == 0) return;

  switch (editField) {
    case 0:
      editYear += delta;
      if (editYear < 2000) editYear = 2099;
      if (editYear > 2099) editYear = 2000;
      break;
    case 1:
      editMonth += delta;
      if (editMonth < 1) editMonth = 12;
      if (editMonth > 12) editMonth = 1;
      break;
    case 2: {
      int maxDay = daysInMonth(editYear, editMonth);
      editDay += delta;
      if (editDay < 1) editDay = maxDay;
      if (editDay > maxDay) editDay = 1;
      break;
    }
    case 3:
      editHour += delta;
      if (editHour < 0) editHour = 23;
      if (editHour > 23) editHour = 0;
      break;
    case 4:
      editMinute += delta;
      if (editMinute < 0) editMinute = 59;
      if (editMinute > 59) editMinute = 0;
      break;
    default:
      break;
  }

  int maxDay = daysInMonth(editYear, editMonth);
  if (editDay > maxDay) editDay = maxDay;
}

void handleTimezoneList() {
  int d = consumeEncoderDelta();
  if (d != 0) {
    tzListIndex += d;
    if (tzListIndex < 0) tzListIndex = TZ_COUNT - 1;
    if (tzListIndex >= TZ_COUNT) tzListIndex = 0;
    drawTimezoneList();
  }

  if (buttonPressedEvent()) {
    applyTimezoneByIndex(tzListIndex);
    saveTimezoneSetting();

    if (startupFlow) {
      screenState = SCREEN_TIME_EDIT;
      loadEditorFromCurrentOrDefault();
      drawTimeEditor();
    } else {
      showMessage("Timezone saved", kTimezones[tzIndex].iana, 1200);
    }
  }
}

void handleTimeEdit() {
  int d = consumeEncoderDelta();
  if (d != 0) {
    adjustEditField(d);
    drawTimeEditor();
  }

  if (buttonPressedEvent()) {
    editField++;
    if (editField > 4) {
      applyManualDateTime();

      if (startupFlow) {
        startupFlow = false;
        showMessage("Setup complete", "Entering home screen", 1300);
      } else {
        showMessage("Time saved", "Manual clock updated", 1200);
      }
      return;
    }
    drawTimeEditor();
  }
}

void handleHome() {
  static time_t lastSecond = -1;
  time_t now = time(nullptr);
  if (now != lastSecond) {
    lastSecond = now;
    drawHomeTimeOnly();
  }

  if (buttonLongPressEvent(LONG_PRESS_MS)) {
    screenState = SCREEN_MENU;
    menuIndex = 0;
    drawMenu();
  }
}

void handleMenu() {
  int d = consumeEncoderDelta();
  if (d != 0) {
    menuIndex += d;
    if (menuIndex < 0) menuIndex = 2;
    if (menuIndex > 2) menuIndex = 0;
    drawMenu();
  }

  if (buttonPressedEvent()) {
    if (menuIndex == 0) {
      startupFlow = false;
      screenState = SCREEN_TZ_LIST;
      tzListIndex = tzIndex;
      drawTimezoneList();
    } else if (menuIndex == 1) {
      startupFlow = false;
      screenState = SCREEN_TIME_EDIT;
      loadEditorFromCurrentOrDefault();
      drawTimeEditor();
    } else {
      screenState = SCREEN_WORLD_VIEW;
      worldBaseIndex = tzIndex;
      drawWorldView();
    }
  }
}

void handleWorldView() {
  static uint32_t lastRefresh = 0;

  int d = consumeEncoderDelta();
  if (d != 0) {
    worldBaseIndex += d;
    while (worldBaseIndex < 0) worldBaseIndex += TZ_COUNT;
    while (worldBaseIndex >= TZ_COUNT) worldBaseIndex -= TZ_COUNT;
    resetWorldClockCache();
    drawWorldRowsOnly();
  }

  if (millis() - lastRefresh > 1000) {
    lastRefresh = millis();
    drawWorldRowsOnly();
  }

  if (buttonPressedEvent()) {
    screenState = SCREEN_MENU;
    drawMenu();
  }
}

void handleMessage() {
  if ((int32_t)(millis() - messageUntilMs) >= 0) {
    if (startupFlow) {
      screenState = SCREEN_TIME_EDIT;
      drawTimeEditor();
    } else {
      screenState = SCREEN_HOME;
      drawHome();
    }
  }
}

void setup() {
  Serial.begin(115200);

  tft.init();
  tft.setRotation(3);

  pinMode(ENC_CLK, INPUT_PULLUP);
  pinMode(ENC_DT, INPUT_PULLUP);
  pinMode(ENC_SW, INPUT_PULLUP);
  encPrevState = (uint8_t)((digitalRead(ENC_CLK) << 1) | digitalRead(ENC_DT));
  attachInterrupt(digitalPinToInterrupt(ENC_CLK), onEncClkChange, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_DT), onEncClkChange, CHANGE);

  loadTimezoneSetting();

  startupFlow = true;
  screenState = SCREEN_TZ_LIST;
  tzListIndex = tzIndex;
  drawTimezoneList();
}

void loop() {
  switch (screenState) {
    case SCREEN_TZ_LIST:
      handleTimezoneList();
      break;
    case SCREEN_TIME_EDIT:
      handleTimeEdit();
      break;
    case SCREEN_HOME:
      handleHome();
      break;
    case SCREEN_MENU:
      handleMenu();
      break;
    case SCREEN_WORLD_VIEW:
      handleWorldView();
      break;
    case SCREEN_MESSAGE:
      handleMessage();
      break;
    default:
      break;
  }

  delay(8);
}
