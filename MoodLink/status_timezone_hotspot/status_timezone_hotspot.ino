/*
 * This file includes the status, the timezone, the hotspot-Wi-Fi connection, and the data transaction functionalities.
 * Please copy the config.example.h  file to config.h first and fill in your AP_SSID / AP_PASSWORD / MQTT credentials
 */

#include "globals.h"
#include "config.h"
#include "web_pages.h"
#include "wifi_manager.h"
#include "screen_draw.h"
#include "mqtt_manager.h"

#include <Preferences.h>
#include <HTTPClient.h>
#include <math.h>
#include <string.h>
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
void drawTimeEditorFieldOnly(int i);
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
void drawWiFiIndicator(bool force);
void drawWelcomePage();
void drawBootNetworkPage();
void drawBootNetworkStatusOnly();
void drawBootNetworkButtonsOnly();
void drawDateTimeMenu();
void drawDateTimeMenuDynamicOnly();
void drawDateTimeMenuButtonsOnly(int prevIndex, bool forceFull);
void drawMenuOptionsOnly();
void drawMenuOptionOnly(int i);
void drawWiFiInfoDynamicOnly();
void drawWiFiInfoButtonsOnly(int prevIndex);
void drawScheduleListPage();
void drawScheduleButtonsOnly(int prevIndex);
void drawScheduleListOnly();
void drawScheduleAddPage();
void drawScheduleAddDynamicOnly();
void drawScheduleDeletePage();
void drawScheduleDeleteRowsOnly();
void applyTimezoneByIndex(int idx);
void saveTimezoneSetting();
void saveSchedulesSetting();
void loadSchedulesSetting();
void applyScheduleEngine();
void initScheduleDraftFromNow();
int getUsedScheduleCount();
int getUsedScheduleSlotByRow(int row);
void formatScheduleSummary(int slot, char* line1, size_t n1, char* line2, size_t n2);
void formatRepeatLabelFromDate(uint8_t repeat, int year, int month, int day, char* out, size_t n);
void formatScheduleItemSummary(const ScheduleItem& s, char* line1, size_t n1, char* line2, size_t n2);

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

void saveAutoTimeSetting() {
  Preferences pref;
  pref.begin(PREF_NS, false);
  pref.putBool(KEY_AUTO_TIME, autoTimeEnabled);
  pref.end();
}

void loadAutoTimeSetting() {
  Preferences pref;
  pref.begin(PREF_NS, true);
  autoTimeEnabled = pref.getBool(KEY_AUTO_TIME, true);
  pref.end();
}

int timezoneIndexFromOffsetSeconds(long offsetSec) {
  float h = (float)offsetSec / 3600.0f;
  int rounded = (h >= 0.0f) ? (int)floorf(h + 0.5f) : (int)ceilf(h - 0.5f);
  if (rounded < -12) rounded = -12;
  if (rounded > 14) rounded = 14;
  return rounded + 12;
}

bool fetchUtcOffsetFromIP(long* outOffsetSec) {
  HTTPClient http;
  http.setTimeout(3000);
  if (!http.begin("http://ip-api.com/json/?fields=status,offset")) return false;
  int code = http.GET();
  if (code != 200) {
    http.end();
    return false;
  }
  String body = http.getString();
  http.end();
  if (body.indexOf("\"status\":\"success\"") < 0) return false;
  int p = body.indexOf("\"offset\":");
  if (p < 0) return false;
  p += 9;
  while (p < body.length() && (body[p] == ' ' || body[p] == '\t')) p++;
  int e = p;
  while (e < body.length() && (body[e] == '-' || (body[e] >= '0' && body[e] <= '9'))) e++;
  if (e <= p) return false;
  *outOffsetSec = body.substring(p, e).toInt();
  return true;
}

bool syncTimeFromNtpUtc() {
  configTzTime("UTC0", "pool.ntp.org", "time.nist.gov", "time.google.com");
  uint32_t start = millis();
  while (millis() - start < 2500) {
    time_t now = time(nullptr);
    if (now > 100000) return true;
    delay(200);
  }
  return false;
}

void maybeAutoSyncTimeAndTimezone(bool force = false) {
  static bool lastWifiConnected = false;
  bool wifiConnected = (WiFi.status() == WL_CONNECTED);
  if (wifiConnected && !lastWifiConnected) force = true;
  lastWifiConnected = wifiConnected;

  if (!autoTimeEnabled) return;
  if (!wifiConnected) return;

  uint32_t nowMs = millis();
  uint32_t interval = autoTimeSynced ? 3600000UL : 15000UL;  // success: hourly, else retry every 15s
  if (!force && (nowMs - autoTimeLastAttemptMs < interval)) return;
  autoTimeLastAttemptMs = nowMs;

  long offsetSec = 0;
  bool tzOk = fetchUtcOffsetFromIP(&offsetSec);
  if (tzOk) {
    int idx = timezoneIndexFromOffsetSeconds(offsetSec);
    if (idx != tzIndex) {
      applyTimezoneByIndex(idx);
      saveTimezoneSetting();
      partnerInfoDirty = true;
    }
  }

  bool ntpOk = syncTimeFromNtpUtc();
  if (ntpOk) autoTimeSynced = true;

  if ((tzOk || ntpOk) && screenState == SCREEN_EMOJI_HOME) {
    drawEmojiHome();
  }
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

time_t makeLocalEpoch(int y, int m, int d, int h, int minute) {
  struct tm tmv = {};
  tmv.tm_year = y - 1900;
  tmv.tm_mon = m - 1;
  tmv.tm_mday = d;
  tmv.tm_hour = h;
  tmv.tm_min = minute;
  tmv.tm_sec = 0;
  tmv.tm_isdst = -1;
  return mktime(&tmv);
}

void clampDraftDate(uint16_t& y, uint8_t& m, uint8_t& d, uint8_t& h, uint8_t& minute) {
  if (y < 2000) y = 2000;
  if (y > 2099) y = 2099;
  if (m < 1) m = 12;
  if (m > 12) m = 1;
  int mx = daysInMonth((int)y, (int)m);
  if (d < 1) d = (uint8_t)mx;
  if (d > mx) d = (uint8_t)mx;
  if (h > 23) h = 0;
  if (minute > 59) minute = 0;
}

void formatRepeatLabelFromDate(uint8_t repeat, int year, int month, int day, char* out, size_t n) {
  static const char* kWday[7] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
  if (repeat == SCH_ONCE) {
    snprintf(out, n, "Once");
  } else if (repeat == SCH_DAILY) {
    snprintf(out, n, "Daily");
  } else if (repeat == SCH_WEEKLY) {
    time_t ep = makeLocalEpoch(year, month, day, 0, 0);
    struct tm ti;
    localtime_r(&ep, &ti);
    snprintf(out, n, "Weekly(%s)", kWday[(ti.tm_wday + 7) % 7]);
  } else {
    snprintf(out, n, "Monthly(%d)", day);
  }
}

int getUsedScheduleCount() {
  int c = 0;
  for (int i = 0; i < MAX_SCHEDULES; i++) if (schedules[i].used) c++;
  return c;
}

int getUsedScheduleSlotByRow(int row) {
  int k = 0;
  for (int i = 0; i < MAX_SCHEDULES; i++) {
    if (!schedules[i].used) continue;
    if (k == row) return i;
    k++;
  }
  return -1;
}

void formatScheduleSummary(int slot, char* line1, size_t n1, char* line2, size_t n2) {
  if (slot < 0 || slot >= MAX_SCHEDULES || !schedules[slot].used) {
    snprintf(line1, n1, "Invalid");
    snprintf(line2, n2, "");
    return;
  }
  const ScheduleItem& s = schedules[slot];
  char rep[32];
  formatRepeatLabelFromDate(s.repeat, s.sy, s.sm, s.sd, rep, sizeof(rep));
  snprintf(line1, n1, "%s  %s", statusText[s.status], rep);
  snprintf(line2, n2, "%02d/%02d %02d:%02d -> %02d/%02d %02d:%02d",
           s.sm, s.sd, s.sh, s.smin, s.em, s.ed, s.eh, s.emin);
}

void formatScheduleItemSummary(const ScheduleItem& s, char* line1, size_t n1, char* line2, size_t n2) {
  char rep[32];
  formatRepeatLabelFromDate(s.repeat, s.sy, s.sm, s.sd, rep, sizeof(rep));
  snprintf(line1, n1, "%s  %s", statusText[s.status], rep);
  snprintf(line2, n2, "%02d/%02d %02d:%02d -> %02d/%02d %02d:%02d",
           s.sm, s.sd, s.sh, s.smin, s.em, s.ed, s.eh, s.emin);
}

void saveSchedulesSetting() {
  Preferences pref;
  pref.begin(PREF_NS, false);
  pref.putBytes(KEY_SCHEDULES, schedules, sizeof(schedules));
  pref.end();
}

void loadSchedulesSetting() {
  Preferences pref;
  pref.begin(PREF_NS, true);
  size_t need = sizeof(schedules);
  size_t got = pref.getBytesLength(KEY_SCHEDULES);
  if (got == need) pref.getBytes(KEY_SCHEDULES, schedules, need);
  else memset(schedules, 0, need);
  pref.end();
}

void initScheduleDraftFromNow() {
  memset(&scheduleDraft, 0, sizeof(scheduleDraft));
  scheduleDraft.used = 1;
  scheduleDraft.status = (uint8_t)myStatus;
  scheduleDraft.repeat = SCH_ONCE;

  struct tm ti;
  if (!getLocalTimeSafe(&ti)) {
    scheduleDraft.sy = 2026;
    scheduleDraft.sm = 1;
    scheduleDraft.sd = 1;
    scheduleDraft.sh = 9;
    scheduleDraft.smin = 0;
  } else {
    scheduleDraft.sy = (uint16_t)(ti.tm_year + 1900);
    scheduleDraft.sm = (uint8_t)(ti.tm_mon + 1);
    scheduleDraft.sd = (uint8_t)ti.tm_mday;
    scheduleDraft.sh = (uint8_t)ti.tm_hour;
    scheduleDraft.smin = (uint8_t)ti.tm_min;
  }
  time_t st = makeLocalEpoch(scheduleDraft.sy, scheduleDraft.sm, scheduleDraft.sd, scheduleDraft.sh, scheduleDraft.smin);
  time_t en = st + 3600;
  struct tm et;
  localtime_r(&en, &et);
  scheduleDraft.ey = (uint16_t)(et.tm_year + 1900);
  scheduleDraft.em = (uint8_t)(et.tm_mon + 1);
  scheduleDraft.ed = (uint8_t)et.tm_mday;
  scheduleDraft.eh = (uint8_t)et.tm_hour;
  scheduleDraft.emin = (uint8_t)et.tm_min;

  scheduleAddStep = 0;
  scheduleAddField = 0;
  scheduleAddAction = 0;
  scheduleAddFull = false;
  scheduleAddInvalidRange = false;
}

void adjustScheduleDraftField(bool editStart, int field, int delta) {
  if (delta == 0) return;
  uint16_t& y = editStart ? scheduleDraft.sy : scheduleDraft.ey;
  uint8_t& mo = editStart ? scheduleDraft.sm : scheduleDraft.em;
  uint8_t& d = editStart ? scheduleDraft.sd : scheduleDraft.ed;
  uint8_t& h = editStart ? scheduleDraft.sh : scheduleDraft.eh;
  uint8_t& mi = editStart ? scheduleDraft.smin : scheduleDraft.emin;

  switch (field) {
    case 0: y = (uint16_t)((int)y + delta); break;
    case 1: mo = (uint8_t)((int)mo + delta); break;
    case 2: d = (uint8_t)((int)d + delta); break;
    case 3: h = (uint8_t)((int)h + delta); break;
    case 4: mi = (uint8_t)((int)mi + delta); break;
    default: break;
  }
  clampDraftDate(y, mo, d, h, mi);
}

int findFreeScheduleSlot() {
  for (int i = 0; i < MAX_SCHEDULES; i++) if (!schedules[i].used) return i;
  return -1;
}

bool saveScheduleDraftToList() {
  scheduleAddFull = false;
  scheduleAddInvalidRange = false;
  int slot = findFreeScheduleSlot();
  if (slot < 0) {
    scheduleAddFull = true;
    return false;
  }

  time_t st = makeLocalEpoch(scheduleDraft.sy, scheduleDraft.sm, scheduleDraft.sd, scheduleDraft.sh, scheduleDraft.smin);
  time_t en = makeLocalEpoch(scheduleDraft.ey, scheduleDraft.em, scheduleDraft.ed, scheduleDraft.eh, scheduleDraft.emin);
  if (en <= st) {
    scheduleAddInvalidRange = true;
    return false;
  }
  scheduleDraft.used = 1;

  schedules[slot] = scheduleDraft;
  saveSchedulesSetting();
  return true;
}

bool scheduleIsActiveAt(const ScheduleItem& s, time_t now, time_t* outStart) {
  if (!s.used || s.status >= ST_COUNT || s.repeat >= SCH_REPEAT_COUNT) return false;
  time_t baseStart = makeLocalEpoch((int)s.sy, (int)s.sm, (int)s.sd, (int)s.sh, (int)s.smin);
  time_t baseEnd = makeLocalEpoch((int)s.ey, (int)s.em, (int)s.ed, (int)s.eh, (int)s.emin);
  if (baseStart <= 100000) return false;
  if (baseEnd <= baseStart) baseEnd = baseStart + 60;
  time_t duration = baseEnd - baseStart;

  if (s.repeat == SCH_ONCE) {
    if (now >= baseStart && now < baseEnd) {
      *outStart = baseStart;
      return true;
    }
    return false;
  }

  struct tm nowTm;
  localtime_r(&now, &nowTm);
  struct tm candTm = nowTm;
  candTm.tm_hour = s.sh;
  candTm.tm_min = s.smin;
  candTm.tm_sec = 0;
  time_t cand = mktime(&candTm);

  if (s.repeat == SCH_DAILY) {
    if (cand > now) cand -= 86400;
  } else if (s.repeat == SCH_WEEKLY) {
    struct tm baseTm;
    localtime_r(&baseStart, &baseTm);
    int target = baseTm.tm_wday;
    int diff = (nowTm.tm_wday - target + 7) % 7;
    candTm.tm_mday = nowTm.tm_mday - diff;
    cand = mktime(&candTm);
    if (cand > now) cand -= 7 * 86400;
  } else if (s.repeat == SCH_MONTHLY) {
    struct tm baseTm;
    localtime_r(&baseStart, &baseTm);
    int targetDay = baseTm.tm_mday;
    int cy = nowTm.tm_year + 1900;
    int cm = nowTm.tm_mon + 1;
    int mx = daysInMonth(cy, cm);
    int cd = (targetDay > mx) ? mx : targetDay;
    cand = makeLocalEpoch(cy, cm, cd, s.sh, s.smin);
    if (cand > now) {
      cm -= 1;
      if (cm < 1) {
        cm = 12;
        cy -= 1;
      }
      mx = daysInMonth(cy, cm);
      cd = (targetDay > mx) ? mx : targetDay;
      cand = makeLocalEpoch(cy, cm, cd, s.sh, s.smin);
    }
  }

  if (cand < baseStart) return false;
  if (now >= cand && now < cand + duration) {
    *outStart = cand;
    return true;
  }
  return false;
}

void applyScheduleEngine() {
  uint32_t nowMs = millis();
  if (nowMs - scheduleLastCheckMs < 500) return;
  scheduleLastCheckMs = nowMs;

  time_t now = time(nullptr);
  if (now <= 100000) return;

  int bestSlot = -1;
  time_t bestStart = 0;
  for (int i = 0; i < MAX_SCHEDULES; i++) {
    time_t evStart = 0;
    if (!scheduleIsActiveAt(schedules[i], now, &evStart)) continue;
    if (bestSlot < 0 || evStart > bestStart) {
      bestSlot = i;
      bestStart = evStart;
    }
  }

  if (bestSlot >= 0) {
    MyStatus target = (MyStatus)schedules[bestSlot].status;
    if (!scheduleOverrideActive) {
      scheduleOverrideActive = true;
      scheduleRestoreStatus = myStatus;
    }
    scheduleActiveSlot = bestSlot;
    if (myStatus != target) {
      myStatus = target;
      mqttPublishStatusNow();
      if (screenState == SCREEN_EMOJI_HOME) drawSelfEmoji();
    }
    return;
  }

  if (scheduleOverrideActive) {
    myStatus = scheduleRestoreStatus;
    scheduleOverrideActive = false;
    scheduleActiveSlot = -1;
    mqttPublishStatusNow();
    if (screenState == SCREEN_EMOJI_HOME) drawSelfEmoji();
  }
}

// Screen handling
void handleWelcome() {
  consumeEncoderStep();
  if (takeShortPressEvent()) {
    clearButtonEvents();
    screenState = SCREEN_BOOT_NETWORK;
    bootMenuIndex = 0;
    drawBootNetworkPage();
  }
}

void handleBootNetwork() {
  static uint32_t lastStatusMs = 0;
  static int lastConn = -1;
  static String lastSsid = "";
  uint32_t nowMs = millis();
  if (nowMs - lastStatusMs >= 600) {
    lastStatusMs = nowMs;
    int conn = (WiFi.status() == WL_CONNECTED) ? 1 : 0;
    String ssid = conn ? WiFi.SSID() : "";
    if (conn != lastConn || ssid != lastSsid) {
      drawBootNetworkStatusOnly();
      lastConn = conn;
      lastSsid = ssid;
    }
    drawWiFiIndicator();
  }

  int d = consumeEncoderStep();
  if (d != 0) {
    bootMenuIndex += d;
    if (bootMenuIndex < 0) bootMenuIndex = 1;
    if (bootMenuIndex > 1) bootMenuIndex = 0;
    drawBootNetworkButtonsOnly();
  }
  if (takeShortPressEvent()) {
    clearButtonEvents();
    if (bootMenuIndex == 0) {
      wifiFromBootFlow = true;
      screenState = SCREEN_WIFI_INFO;
      wifiMenuIndex = 0;
      drawWiFiInfoPage();
    } else {
      wifiFromBootFlow = false;
      screenState = SCREEN_EMOJI_HOME;
      drawEmojiHome();
    }
  }
}

void handleDateTimeMenu() {
  int optionCount = autoTimeEnabled ? 2 : 4;
  int d = consumeEncoderStep();
  if (d != 0) {
    int prev = dateTimeMenuIndex;
    dateTimeMenuIndex += d;
    if (dateTimeMenuIndex < 0) dateTimeMenuIndex = optionCount - 1;
    if (dateTimeMenuIndex >= optionCount) dateTimeMenuIndex = 0;
    drawDateTimeMenuButtonsOnly(prev, false);
  }
  if (takeShortPressEvent()) {
    clearButtonEvents();
    if (dateTimeMenuIndex == 0) {
      autoTimeEnabled = !autoTimeEnabled;
      autoTimeSynced = false;
      saveAutoTimeSetting();
      if (autoTimeEnabled) maybeAutoSyncTimeAndTimezone(true);
      dateTimeMenuIndex = 0;
      drawDateTimeMenu();
      return;
    }

    if (autoTimeEnabled) {
      screenState = SCREEN_MENU;
      drawMenu();
      return;
    }

    switch (dateTimeMenuIndex) {
      case 1:
        fromDateTimeMenu = true;
        screenState = SCREEN_TZ_LIST;
        tzListIndex = tzIndex;
        drawTimezoneList();
        drawTimezoneListRowsOnly();
        break;
      case 2:
        fromDateTimeMenu = true;
        screenState = SCREEN_TIME_EDIT;
        loadEditorFromCurrentOrDefault();
        drawTimeEditor();
        drawTimeEditorFieldsOnly();
        break;
      default:
        screenState = SCREEN_MENU;
        drawMenu();
        break;
    }
  }
}

void handleScheduleList() {
  int d = consumeEncoderStep();
  if (d != 0) {
    int prev = scheduleMenuIndex;
    scheduleMenuIndex += d;
    if (scheduleMenuIndex < 0) scheduleMenuIndex = 2;
    if (scheduleMenuIndex > 2) scheduleMenuIndex = 0;
    drawScheduleButtonsOnly(prev);
  }
  if (takeShortPressEvent()) {
    clearButtonEvents();
    if (scheduleMenuIndex == 0) {
      initScheduleDraftFromNow();
      screenState = SCREEN_SCHEDULE_ADD;
      drawScheduleAddPage();
    } else if (scheduleMenuIndex == 1) {
      scheduleDeleteIndex = 0;
      screenState = SCREEN_SCHEDULE_DELETE;
      drawScheduleDeletePage();
    } else {
      screenState = SCREEN_MENU;
      drawMenu();
    }
  }
}

void handleScheduleAdd() {
  int d = consumeEncoderStep();
  if (d != 0) {
    scheduleAddFull = false;
    scheduleAddInvalidRange = false;
    if (scheduleAddStep == 0) {
      int next = (int)scheduleDraft.status + d;
      while (next < 0) next += ST_COUNT;
      while (next >= ST_COUNT) next -= ST_COUNT;
      scheduleDraft.status = (uint8_t)next;
    } else if (scheduleAddStep == 1) {
      adjustScheduleDraftField(true, scheduleAddField, d);
    } else if (scheduleAddStep == 2) {
      adjustScheduleDraftField(false, scheduleAddField, d);
    } else if (scheduleAddStep == 3) {
      int next = (int)scheduleDraft.repeat + d;
      while (next < 0) next += SCH_REPEAT_COUNT;
      while (next >= SCH_REPEAT_COUNT) next -= SCH_REPEAT_COUNT;
      scheduleDraft.repeat = (uint8_t)next;
    } else {
      scheduleAddAction = (d > 0) ? 1 : 0;
    }
    drawScheduleAddDynamicOnly();
  }

  if (takeShortPressEvent()) {
    clearButtonEvents();
    scheduleAddFull = false;
    scheduleAddInvalidRange = false;
    if (scheduleAddStep == 0) {
      scheduleAddStep = 1;
      scheduleAddField = 0;
      drawScheduleAddDynamicOnly();
      return;
    }
    if (scheduleAddStep == 1 || scheduleAddStep == 2) {
      scheduleAddField++;
      if (scheduleAddField > 4) {
        scheduleAddStep++;
        scheduleAddField = 0;
      }
      drawScheduleAddDynamicOnly();
      return;
    }
    if (scheduleAddStep == 3) {
      scheduleAddStep = 4;
      scheduleAddAction = 0;
      drawScheduleAddDynamicOnly();
      return;
    }

    if (scheduleAddAction == 0) {
      if (!saveScheduleDraftToList()) {
        drawScheduleAddDynamicOnly();
        return;
      }
    }
    screenState = SCREEN_SCHEDULE_LIST;
    scheduleMenuIndex = 0;
    drawScheduleListPage();
  }
}

void handleScheduleDelete() {
  int used = getUsedScheduleCount();
  int maxIndex = used;  // last one is Back
  int d = consumeEncoderStep();
  if (d != 0) {
    scheduleDeleteIndex += d;
    if (scheduleDeleteIndex < 0) scheduleDeleteIndex = maxIndex;
    if (scheduleDeleteIndex > maxIndex) scheduleDeleteIndex = 0;
    drawScheduleDeleteRowsOnly();
  }
  if (takeShortPressEvent()) {
    clearButtonEvents();
    if (scheduleDeleteIndex == used) {
      screenState = SCREEN_SCHEDULE_LIST;
      drawScheduleListPage();
      return;
    }
    int slot = getUsedScheduleSlotByRow(scheduleDeleteIndex);
    if (slot >= 0) {
      schedules[slot].used = 0;
      saveSchedulesSetting();
      if (scheduleActiveSlot == slot) scheduleActiveSlot = -1;
      scheduleLastCheckMs = 0;
      applyScheduleEngine();
    }
    used = getUsedScheduleCount();
    if (scheduleDeleteIndex > used) scheduleDeleteIndex = used;
    drawScheduleDeleteRowsOnly();
  }
}

void handleTimezoneList() {
  static uint32_t lastWifiIconMs = 0;
  uint32_t nowMs = millis();
  if (nowMs - lastWifiIconMs >= 500) {
    lastWifiIconMs = nowMs;
    drawWiFiIndicator();
  }

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
    if (fromDateTimeMenu) {
      fromDateTimeMenu = false;
      screenState = SCREEN_DATE_TIME_MENU;
      dateTimeMenuIndex = 0;
      drawDateTimeMenu();
    } else {
      screenState = SCREEN_EMOJI_HOME;
      drawEmojiHome();
    }
  }
}

void handleTimeEdit() {
  static uint32_t lastWifiIconMs = 0;
  uint32_t nowMs = millis();
  if (nowMs - lastWifiIconMs >= 500) {
    lastWifiIconMs = nowMs;
    drawWiFiIndicator();
  }

  int d = consumeEncoderStep();
  if (d != 0) {
    int prevField = editField;
    adjustEditField(d);
    if (!timeEditStaticDrawn) drawTimeEditor();
    drawTimeEditorFieldOnly(prevField);
  }
  if (takeShortPressEvent()) {
    int prevField = editField;
    editField++;
    if (editField > 4) {
      applyManualDateTime();
      clearButtonEvents();
      if (fromDateTimeMenu) {
        fromDateTimeMenu = false;
        screenState = SCREEN_DATE_TIME_MENU;
        dateTimeMenuIndex = 0;
        drawDateTimeMenu();
      } else {
        screenState = SCREEN_EMOJI_HOME;
        drawEmojiHome();
      }
      return;
    }
    if (!timeEditStaticDrawn) drawTimeEditor();
    drawTimeEditorFieldOnly(prevField);
    drawTimeEditorFieldOnly(editField);
  }
}

void handleEmojiHome() {
  int d = consumeEncoderStep();
  if (d != 0) {
    if (scheduleOverrideActive) return;
    int next = (int)myStatus + (d > 0 ? 1 : -1);
    if (next < 0) next = ST_COUNT - 1;
    if (next >= ST_COUNT) next = 0;
    myStatus = (MyStatus)next;
    mqttPublishStatusNow();
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
    int prev = menuIndex;
    menuIndex += d;
    if (menuIndex < 0) menuIndex = 4;
    if (menuIndex > 4) menuIndex = 0;
    drawMenuOptionOnly(prev);
    drawMenuOptionOnly(menuIndex);
  }
  if (takeShortPressEvent()) {
    clearButtonEvents();
    switch (menuIndex) {
      case 0:
        screenState = SCREEN_DATE_TIME_MENU;
        dateTimeMenuIndex = 0;
        drawDateTimeMenu();
        break;
      case 1:
        screenState = SCREEN_WORLD_VIEW;
        worldBaseIndex = tzIndex;
        drawWorldView();
        break;
      case 2:
        screenState = SCREEN_WIFI_INFO;
        wifiFromBootFlow = false;
        wifiMenuIndex = 0;
        drawWiFiInfoPage();
        break;
      case 3:
        screenState = SCREEN_SCHEDULE_LIST;
        scheduleMenuIndex = 0;
        drawScheduleListPage();
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
  static uint32_t lastWifiInfoMs = 0;
  static int lastConn = -1;
  static String lastSsid = "";
  uint32_t nowMs = millis();
  if (nowMs - lastWifiInfoMs >= 700) {
    lastWifiInfoMs = nowMs;
    int conn = (WiFi.status() == WL_CONNECTED) ? 1 : 0;
    String ssid = conn ? WiFi.SSID() : "";
    if (conn != lastConn || ssid != lastSsid) {
      drawWiFiInfoDynamicOnly();
      lastConn = conn;
      lastSsid = ssid;
    }
    drawWiFiIndicator();
  }
  int d = consumeEncoderStep();
  if (d != 0) {
    int prev = wifiMenuIndex;
    wifiMenuIndex += d;
    if (wifiMenuIndex < 0) wifiMenuIndex = 1;
    if (wifiMenuIndex > 1) wifiMenuIndex = 0;
    drawWiFiInfoButtonsOnly(prev);
  }
  if (takeShortPressEvent()) {
    clearButtonEvents();
    if (wifiMenuIndex == 0) {
      screenState = SCREEN_WIFI_CONNECTING;
      drawWiFiConnectingPage();
      wifiStartHotspot();
    } else {
      if (wifiFromBootFlow) {
        wifiFromBootFlow = false;
        screenState = SCREEN_EMOJI_HOME;
        drawEmojiHome();
      } else {
        screenState = SCREEN_MENU;
        drawMenu();
      }
    }
  }
}

void handleWiFiConnecting() {
  ensureWifiInit();
  consumeEncoderStep();

  // Allow user to cancel the connection process and go back
  if (takeShortPressEvent()) {
    clearButtonEvents();
    wifiStopHotspot();
    if (wifiFromBootFlow) {
      wifiFromBootFlow = false;
      screenState = SCREEN_BOOT_NETWORK;
      bootMenuIndex = 0;
      drawBootNetworkPage();
    } else {
      screenState = SCREEN_WIFI_INFO;
      wifiMenuIndex = 0;
      drawWiFiInfoPage();
    }
    return;
  }

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
  loadAutoTimeSetting();
  loadSchedulesSetting();
  wifiInit();
  wifiInited = true;
  maybeAutoSyncTimeAndTimezone(true);

  startupFlow = false;
  screenState = SCREEN_WELCOME;
  drawWelcomePage();
  mqttInit();
}

void loop() {
  updateButtonEvents();

  switch (screenState) {
    case SCREEN_WELCOME: handleWelcome(); break;
    case SCREEN_BOOT_NETWORK: handleBootNetwork(); break;
    case SCREEN_TZ_LIST: handleTimezoneList(); break;
    case SCREEN_TIME_EDIT: handleTimeEdit(); break;
    case SCREEN_DATE_TIME_MENU: handleDateTimeMenu(); break;
    case SCREEN_SCHEDULE_LIST: handleScheduleList(); break;
    case SCREEN_SCHEDULE_ADD: handleScheduleAdd(); break;
    case SCREEN_SCHEDULE_DELETE: handleScheduleDelete(); break;
    case SCREEN_EMOJI_HOME: handleEmojiHome(); break;
    case SCREEN_MENU: handleMenu(); break;
    case SCREEN_WORLD_VIEW: handleWorldView(); break;
    case SCREEN_WIFI_INFO: handleWiFiInfo(); break;
    case SCREEN_WIFI_CONNECTING: handleWiFiConnecting(); break;
    case SCREEN_WIFI_RESULT: handleWiFiResult(); break;
    default: break;
  }

  wifiMaintainConnection();
  maybeAutoSyncTimeAndTimezone();
  applyScheduleEngine();
  delay(8);
  mqttLoop();
}
