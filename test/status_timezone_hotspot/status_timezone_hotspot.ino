/*
 * This file includes the status, the timezone and the hotspot-Wi-Fi connection functionalities.
 * Please copy the config.example.h  file to config.h first and fill in your AP_SSID / AP_PASSWORD
 */

#include "globals.h"
#include "config.h"
#include "web_pages.h"
#include "wifi_manager.h"
#include "screen_draw.h"

#include <Preferences.h>
#include <math.h>
#include <esp_log.h>
#include <sys/time.h>
#include <time.h>

static constexpr int ENC_CLK = 16;
static constexpr int ENC_DT = 17;
static constexpr int ENC_SW = 18;

static constexpr uint32_t LONG_PRESS_MS = 2000;
static constexpr uint32_t DEBOUNCE_MS = 35;

void drawTimezoneList();
void drawTimezoneListRowsOnly();
void drawTimeEditor();
void drawTimeEditorFieldsOnly();
void drawMenu();
void drawWorldView();
void drawWorldRowsOnly();
void resetWorldClockCache();
void drawEmojiHome();
void tickEmojiHome();
void drawSelfEmoji();
void drawWiFiInfoPage();
void drawWiFiConnectingPage();
void drawWiFiResultPage(bool success);
void drawWiFiIndicator();

void IRAM_ATTR onEncChange() {
  static const int8_t trans[16] = {
    0, -1, 1, 0,
    1, 0, 0, -1,
    -1, 0, 0, 1,
    0, 1, -1, 0
  };
  uint8_t curr = (uint8_t)((digitalRead(ENC_CLK) << 1) | digitalRead(ENC_DT));
  uint8_t idx = (uint8_t)((encPrevState << 2) | curr);
  encDelta += trans[idx];
  encPrevState = curr;
}

void ensureWifiInit() {
  if (wifiInited) return;
  wifiInit();
  wifiInited = true;
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

// Button Handling
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

// Time utilities
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
  struct timeval tv = { epoch, 0 };
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
    case 2:
      {
        int mx = daysInMonth(editYear, editMonth);
        editDay += delta;
        if (editDay < 1) editDay = mx;
        if (editDay > mx) editDay = 1;
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
  }
  int mx = daysInMonth(editYear, editMonth);
  if (editDay > mx) editDay = mx;
}

// Screen handling
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
      startupFlow = false;
      screenState = SCREEN_EMOJI_HOME;
      drawEmojiHome();
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
    drawSelfEmoji();
    return;
  }
  if (takeLongPressEvent()) {
    clearButtonEvents();
    startupFlow = false;
    screenState = SCREEN_MENU;
    menuIndex = 0;
    drawMenu();
    return;
  }
  tickEmojiHome();
}

void handleMenu() {
  int d = consumeEncoderStep();
  if (d != 0) {
    menuIndex += d;
    if (menuIndex < 0) menuIndex = 4;
    if (menuIndex > 4) menuIndex = 0;
    drawMenu();
  }
  if (takeShortPressEvent()) {
    clearButtonEvents();
    switch (menuIndex) {
      case 0:
        screenState = SCREEN_TZ_LIST;
        tzListIndex = tzIndex;
        drawTimezoneList();
        drawTimezoneListRowsOnly();
        break;
      case 1:
        screenState = SCREEN_TIME_EDIT;
        loadEditorFromCurrentOrDefault();
        drawTimeEditor();
        drawTimeEditorFieldsOnly();
        break;
      case 2:
        screenState = SCREEN_WORLD_VIEW;
        worldBaseIndex = tzIndex;
        drawWorldView();
        break;
      case 3:
        screenState = SCREEN_WIFI_INFO;
        wifiMenuIndex = 0;
        drawWiFiInfoPage();
        break;
      case 4:
        screenState = SCREEN_EMOJI_HOME;
        drawEmojiHome();
        break;
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

void handleWiFiInfo() {
  ensureWifiInit();
  int d = consumeEncoderStep();
  if (d != 0) {
    wifiMenuIndex += d;
    if (wifiMenuIndex < 0) wifiMenuIndex = 1;
    if (wifiMenuIndex > 1) wifiMenuIndex = 0;
    drawWiFiInfoPage();
  }
  if (takeShortPressEvent()) {
    clearButtonEvents();
    if (wifiMenuIndex == 0) {
      screenState = SCREEN_WIFI_CONNECTING;
      drawWiFiConnectingPage();
      wifiStartHotspot();
    } else {
      screenState = SCREEN_MENU;
      drawMenu();
    }
  }
}

void handleWiFiConnecting() {
  ensureWifiInit();
  consumeEncoderStep();
  clearButtonEvents();
  if (wifiPollHotspot()) {
    screenState = SCREEN_WIFI_RESULT;
    drawWiFiResultPage(wifiResultOk);
    drawWiFiIndicator();
  }
}

void handleWiFiResult() {
  ensureWifiInit();
  consumeEncoderStep();
  if (takeShortPressEvent()) {
    clearButtonEvents();
    screenState = SCREEN_WIFI_INFO;
    wifiMenuIndex = 0;
    drawWiFiInfoPage();
  }
}

// Setup and loop
void setup() {
#if ENABLE_UART_DEBUG
  Serial.begin(115200);
#else
  esp_log_level_set("*", ESP_LOG_NONE);
#endif

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
    case SCREEN_TZ_LIST: handleTimezoneList(); break;
    case SCREEN_TIME_EDIT: handleTimeEdit(); break;
    case SCREEN_EMOJI_HOME: handleEmojiHome(); break;
    case SCREEN_MENU: handleMenu(); break;
    case SCREEN_WORLD_VIEW: handleWorldView(); break;
    case SCREEN_WIFI_INFO: handleWiFiInfo(); break;
    case SCREEN_WIFI_CONNECTING: handleWiFiConnecting(); break;
    case SCREEN_WIFI_RESULT: handleWiFiResult(); break;
    default: break;
  }

  delay(8);
}