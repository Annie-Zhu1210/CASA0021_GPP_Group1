#include <LovyanGFX.hpp>
#include <Preferences.h>
#include <math.h>
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

/* ================= INPUT / STORAGE ================= */
static constexpr uint32_t LONG_PRESS_MS = 5000;
static constexpr uint32_t DEBOUNCE_MS = 35;
static const char* PREF_NS = "status-tz";
static const char* KEY_TZ_INDEX = "tz_idx";

class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ILI9488 _panel;
  lgfx::Bus_Parallel8 _bus;

 public:
  LGFX() {
    {
      auto cfg = _bus.config();
      cfg.port = 0;
      cfg.freq_write = 2000000;
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
  const char* label;
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
  SCREEN_EMOJI_HOME,
  SCREEN_MENU,
  SCREEN_WORLD_VIEW
};

enum MyStatus {
  ST_FREE = 0,
  ST_BUSY,
  ST_SLEEPING,
  ST_MISS_YOU,
  ST_BAD_DAY,
  ST_COUNT
};

const char* statusText[ST_COUNT] = {
  "FREE", "BUSY", "SLEEPING", "MISS YOU", "BAD DAY"
};

ScreenState screenState = SCREEN_TZ_LIST;
MyStatus myStatus = ST_FREE;
MyStatus partnerStatus = ST_MISS_YOU;

int tzIndex = 0;
int tzListIndex = 0;
int menuIndex = 0;
int worldBaseIndex = 0;
bool startupFlow = true;

int editYear = 2026;
int editMonth = 1;
int editDay = 1;
int editHour = 12;
int editMinute = 0;
int editField = 0;

float SCALE = 1.2f;
volatile int32_t encDelta = 0;
volatile uint8_t encPrevState = 0;

bool gShortPressEvent = false;
bool gLongPressEvent = false;

bool homeOverlayDrawn = false;
time_t lastHomeSecond = -1;

bool tzListStaticDrawn = false;
bool timeEditStaticDrawn = false;
bool worldLayoutReady = false;
int worldPrevIdx[4] = {-1, -1, -1, -1};
int worldPrevHour[4] = {-1, -1, -1, -1};
int worldPrevMinute[4] = {-1, -1, -1, -1};
int worldPrevSecond[4] = {-1, -1, -1, -1};

static uint32_t zLastMs = 0;
static uint8_t zFrame = 0;
static bool sleepSceneDrawn = false;
struct Rect { int x, y, w, h; };
static Rect zPrev[3] = {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}};

/* ================= CROSS-TAB FUNCTIONS ================= */
void drawTimezoneList();
void drawTimezoneListRowsOnly();
void drawTimeEditor();
void drawTimeEditorFieldsOnly();
void drawMenu();
void drawWorldView();
void drawWorldRowsOnly();
void resetWorldClockCache();
void drawEmojiHome();
void drawTopLeftTimeOnly();
void updateZzzAnimation(int cx, int cy);

/* ================= INPUT ================= */
void IRAM_ATTR onEncChange() {
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

int consumeEncoderStep() {
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

void updateButtonEvents() {
  static bool stable = HIGH;
  static bool lastRead = HIGH;
  static uint32_t lastChangeMs = 0;
  static bool pressed = false;
  static bool longFired = false;
  static uint32_t pressedAt = 0;

  bool r = digitalRead(ENC_SW);
  uint32_t now = millis();

  if (r != lastRead) {
    lastRead = r;
    lastChangeMs = now;
  }

  if (now - lastChangeMs >= DEBOUNCE_MS && stable != lastRead) {
    stable = lastRead;
    if (stable == LOW) {
      pressed = true;
      longFired = false;
      pressedAt = now;
    } else {
      if (pressed && !longFired) gShortPressEvent = true;
      pressed = false;
    }
  }

  if (pressed && !longFired && (now - pressedAt >= LONG_PRESS_MS)) {
    longFired = true;
    gLongPressEvent = true;
  }
}

bool takeShortPressEvent() {
  bool v = gShortPressEvent;
  gShortPressEvent = false;
  return v;
}

bool takeLongPressEvent() {
  bool v = gLongPressEvent;
  gLongPressEvent = false;
  return v;
}

void clearButtonEvents() {
  gShortPressEvent = false;
  gLongPressEvent = false;
}

/* ================= TIME ================= */
void applyTimezoneByIndex(int idx) {
  if (idx < 0) idx = 0;
  if (idx >= TZ_COUNT) idx = TZ_COUNT - 1;
  tzIndex = idx;
  setenv("TZ", kTimezones[tzIndex].posix, 1);
  tzset();
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
  int idx = pref.getInt(KEY_TZ_INDEX, 20);
  pref.end();
  applyTimezoneByIndex(idx);
}

bool getLocalTimeSafe(struct tm* out) {
  time_t now = time(nullptr);
  if (now <= 100000) return false;
  localtime_r(&now, out);
  return true;
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

void setSystemClock(time_t epoch) {
  struct timeval tv;
  tv.tv_sec = epoch;
  tv.tv_usec = 0;
  settimeofday(&tv, nullptr);
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
  if (epoch > 100000) setSystemClock(epoch);
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
  int d = consumeEncoderStep();
  if (d != 0) {
    tzListIndex += d;
    if (tzListIndex < 0) tzListIndex = TZ_COUNT - 1;
    if (tzListIndex >= TZ_COUNT) tzListIndex = 0;
    if (!tzListStaticDrawn) drawTimezoneList();
    drawTimezoneListRowsOnly();
  }

  if (takeShortPressEvent()) {
    applyTimezoneByIndex(tzListIndex);
    saveTimezoneSetting();
    clearButtonEvents();
    if (startupFlow) {
      screenState = SCREEN_TIME_EDIT;
      loadEditorFromCurrentOrDefault();
      drawTimeEditor();
      drawTimeEditorFieldsOnly();
    } else {
      screenState = SCREEN_EMOJI_HOME;
      drawEmojiHome();
    }
  }
}

void handleTimeEdit() {
  int d = consumeEncoderStep();
  if (d != 0) {
    adjustEditField(d);
    if (!timeEditStaticDrawn) drawTimeEditor();
    drawTimeEditorFieldsOnly();
  }

  if (takeShortPressEvent()) {
    editField++;
    if (editField > 4) {
      applyManualDateTime();
      clearButtonEvents();
      if (startupFlow) {
        startupFlow = false;
        screenState = SCREEN_EMOJI_HOME;
        drawEmojiHome();
      } else {
        screenState = SCREEN_EMOJI_HOME;
        drawEmojiHome();
      }
      return;
    }
    if (!timeEditStaticDrawn) drawTimeEditor();
    drawTimeEditorFieldsOnly();
  }
}

void handleEmojiHome() {
  int d = consumeEncoderStep();
  if (d != 0) {
    int next = (int)myStatus + (d > 0 ? 1 : -1);
    if (next < 0) next = ST_COUNT - 1;
    if (next >= ST_COUNT) next = 0;
    myStatus = (MyStatus)next;
    drawEmojiHome();
  }

  if (takeLongPressEvent()) {
    clearButtonEvents();
    startupFlow = false;
    screenState = SCREEN_MENU;
    menuIndex = 0;
    drawMenu();
    return;
  }

  time_t now = time(nullptr);
  if (now > 100000 && now != lastHomeSecond) {
    lastHomeSecond = now;
    drawTopLeftTimeOnly();
  }

  if (myStatus == ST_SLEEPING && sleepSceneDrawn) {
    int cx = tft.width() / 2;
    int cy = tft.height() / 2 - 20;
    updateZzzAnimation(cx, cy);
  }
}

void handleMenu() {
  int d = consumeEncoderStep();
  if (d != 0) {
    menuIndex += d;
    if (menuIndex < 0) menuIndex = 2;
    if (menuIndex > 2) menuIndex = 0;
    drawMenu();
  }

  if (takeShortPressEvent()) {
    clearButtonEvents();
    if (menuIndex == 0) {
      screenState = SCREEN_TZ_LIST;
      tzListIndex = tzIndex;
      drawTimezoneList();
      drawTimezoneListRowsOnly();
    } else if (menuIndex == 1) {
      screenState = SCREEN_TIME_EDIT;
      loadEditorFromCurrentOrDefault();
      drawTimeEditor();
      drawTimeEditorFieldsOnly();
    } else {
      screenState = SCREEN_WORLD_VIEW;
      worldBaseIndex = tzIndex;
      drawWorldView();
    }
  }
}

void handleWorldView() {
  static uint32_t lastRefresh = 0;

  int d = consumeEncoderStep();
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

  if (takeShortPressEvent()) {
    clearButtonEvents();
    screenState = SCREEN_EMOJI_HOME;
    drawEmojiHome();
  }
}

/* ================= SETUP & LOOP ================= */
void setup() {
  Serial.begin(115200);

  tft.init();
  tft.setRotation(3);
  tft.setTextWrap(false, false);

  pinMode(ENC_CLK, INPUT_PULLUP);
  pinMode(ENC_DT, INPUT_PULLUP);
  pinMode(ENC_SW, INPUT_PULLUP);

  encPrevState = (uint8_t)((digitalRead(ENC_CLK) << 1) | digitalRead(ENC_DT));
  attachInterrupt(digitalPinToInterrupt(ENC_CLK), onEncChange, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_DT), onEncChange, CHANGE);

  loadTimezoneSetting();

  startupFlow = true;
  screenState = SCREEN_TZ_LIST;
  tzListIndex = tzIndex;
  drawTimezoneList();
  drawTimezoneListRowsOnly();
}

void loop() {
  updateButtonEvents();

  switch (screenState) {
    case SCREEN_TZ_LIST:
      handleTimezoneList();
      break;
    case SCREEN_TIME_EDIT:
      handleTimeEdit();
      break;
    case SCREEN_EMOJI_HOME:
      handleEmojiHome();
      break;
    case SCREEN_MENU:
      handleMenu();
      break;
    case SCREEN_WORLD_VIEW:
      handleWorldView();
      break;
    default:
      break;
  }

  delay(8);
}
