/*
 * globals.h
 *
 * All shared types, the LGFX class, the TFT object, and every global
 * variable used across the .ino and .h files.
 *
 * Include this FIRST in status_timezone_hotspot.ino, and also at the
 * top of screen_draw.h and wifi_manager.h.
 */


#ifndef GLOBALS_H
#define GLOBALS_H

#include <LovyanGFX.hpp>
#include <WiFi.h>

#ifndef ENABLE_UART_DEBUG
#define ENABLE_UART_DEBUG 0
#endif


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

class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ILI9488 _panel;
  lgfx::Bus_Parallel8 _bus;
public:
  LGFX() {
    {
      auto cfg = _bus.config();
      cfg.port       = 0;
      cfg.freq_write = 8000000;  //   to reduce pixel corruption
      cfg.freq_read  = 4000000;  // ← added explicit read frequency
      cfg.pin_wr     = TFT_WR;
      cfg.pin_rd     = -1;
      cfg.pin_rs     = TFT_DC;
      cfg.pin_d0 = TFT_D0; cfg.pin_d1 = TFT_D1;
      cfg.pin_d2 = TFT_D2; cfg.pin_d3 = TFT_D3;
      cfg.pin_d4 = TFT_D4; cfg.pin_d5 = TFT_D5;
      cfg.pin_d6 = TFT_D6; cfg.pin_d7 = TFT_D7;
      _bus.config(cfg);
      _panel.setBus(&_bus);
    }
    {
      auto cfg = _panel.config();
      cfg.pin_cs   = TFT_CS;
      cfg.pin_rst  = TFT_RST;
      cfg.pin_busy = -1;
      cfg.memory_width  = 320; cfg.memory_height = 480;
      cfg.panel_width   = 320; cfg.panel_height  = 480;
      cfg.offset_x = 0;        cfg.offset_y = 0;
      cfg.readable    = false;   // ← added: no read back needed
      cfg.invert      = false;   // ← added: no inversion
      cfg.rgb_order   = false;   // ← added: BGR order for ILI9488
      cfg.dlen_16bit  = false;   // ← critical: forces 18-bit colour for ILI9488
      cfg.bus_shared  = false;   // ← added: bus not shared with other devices
      _panel.config(cfg);
    }
    setPanel(&_panel);
  }
};

struct TimezoneOption {
  const char* label;
  const char* posix;
  const char* countries;
};

struct Rect { int x, y, w, h; };

enum ScreenState {
  SCREEN_WELCOME = 0,
  SCREEN_BOOT_NETWORK,
  SCREEN_TZ_LIST,
  SCREEN_TIME_EDIT,
  SCREEN_DATE_TIME_MENU,
  SCREEN_SCHEDULE_LIST,
  SCREEN_SCHEDULE_ADD,
  SCREEN_SCHEDULE_DELETE,
  SCREEN_EMOJI_HOME,
  SCREEN_MENU,
  SCREEN_WORLD_VIEW,
  SCREEN_WIFI_INFO,
  SCREEN_WIFI_CONNECTING,
  SCREEN_WIFI_RESULT
};

enum MyStatus {
  ST_FREE = 0,
  ST_BUSY,
  ST_SLEEPING,
  ST_MISS_YOU,
  ST_BAD_DAY,
  ST_COUNT
};

const char* const PREF_NS      = "ld-device";
const char* const KEY_TZ_INDEX = "tz_idx";
const char* const KEY_AUTO_TIME = "auto_time";
const char* const KEY_SSID     = "ssid";
const char* const KEY_PASSWORD = "password";
const char* const KEY_PAIRING  = "pairing";
const char* const KEY_SCHEDULES = "scheds";

TimezoneOption kTimezones[] = {
  {"UTC-12","UTC+12","USA (Baker Is.)"},
  {"UTC-11","UTC+11","USA (Samoa) / Niue"},
  {"UTC-10","UTC+10","USA (Hawaii)"},
  {"UTC-09","UTC+9", "USA (Alaska)"},
  {"UTC-08","UTC+8", "USA / Canada (West)"},
  {"UTC-07","UTC+7", "USA / Canada (Mountain)"},
  {"UTC-06","UTC+6", "USA / Mexico"},
  {"UTC-05","UTC+5", "USA / Canada (East)"},
  {"UTC-04","UTC+4", "Canada (Atlantic) / Bolivia"},
  {"UTC-03","UTC+3", "Brazil / Argentina"},
  {"UTC-02","UTC+2", "Brazil (Fernando de Noronha)"},
  {"UTC-01","UTC+1", "Portugal (Azores) / Cape Verde"},
  {"UTC+00","UTC0",  "UK / Portugal / Ghana"},
  {"UTC+01","UTC-1", "France / Germany / Spain"},
  {"UTC+02","UTC-2", "Egypt / South Africa / Greece"},
  {"UTC+03","UTC-3", "Russia / Saudi Arabia / Kenya"},
  {"UTC+04","UTC-4", "UAE / Oman / Georgia"},
  {"UTC+05","UTC-5", "Pakistan / Uzbekistan"},
  {"UTC+06","UTC-6", "Bangladesh / Kazakhstan"},
  {"UTC+07","UTC-7", "Thailand / Vietnam / Indonesia"},
  {"UTC+08","UTC-8", "China / Singapore / Malaysia"},
  {"UTC+09","UTC-9", "Japan / South Korea"},
  {"UTC+10","UTC-10","Australia / Papua New Guinea"},
  {"UTC+11","UTC-11","Solomon Islands / New Caledonia"},
  {"UTC+12","UTC-12","New Zealand / Fiji"},
  {"UTC+13","UTC-13","Samoa / Tonga"},
  {"UTC+14","UTC-14","Kiribati (Line Islands)"},
};
const int TZ_COUNT = sizeof(kTimezones) / sizeof(kTimezones[0]);

LGFX tft;

// Screen & status state
ScreenState screenState   = SCREEN_WELCOME;
MyStatus    myStatus      = ST_FREE;

// Partner status & timezone — defaults for pre-MQTT layout testing
MyStatus    partnerStatus  = ST_MISS_YOU;
int         partnerTzIndex = 20;

// Partner time override for pre-MQTT testing
bool        partnerTimeValid = false;
time_t      partnerEpoch     = 0;
bool        partnerStatusDirty = false;
bool        partnerInfoDirty   = false;

const char* statusText[ST_COUNT] = {
  "FREE", "BUSY", "SLEEPING", "MISS YOU", "BAD DAY"
};

enum ScheduleRepeat {
  SCH_ONCE = 0,
  SCH_DAILY,
  SCH_WEEKLY,
  SCH_MONTHLY,
  SCH_REPEAT_COUNT
};

struct ScheduleItem {
  uint8_t used;
  uint8_t status;
  uint8_t repeat;
  uint16_t sy;
  uint8_t sm;
  uint8_t sd;
  uint8_t sh;
  uint8_t smin;
  uint16_t ey;
  uint8_t em;
  uint8_t ed;
  uint8_t eh;
  uint8_t emin;
};

static constexpr int MAX_SCHEDULES = 4;
ScheduleItem schedules[MAX_SCHEDULES] = {};

// Timezone and menu navigation
int  tzIndex        = 0;
int  tzListIndex    = 0;
int  menuIndex      = 0;
int  bootMenuIndex  = 0;
int  dateTimeMenuIndex = 0;
int  worldBaseIndex = 0;
bool startupFlow    = false;
bool wifiFromBootFlow = false;
bool fromDateTimeMenu = false;
int  scheduleMenuIndex = 0;
int  scheduleDeleteIndex = 0;

// Date/time auto mode
bool autoTimeEnabled = true;
bool autoTimeSynced = false;
uint32_t autoTimeLastAttemptMs = 0;

// Time editor fields
int editYear   = 2026, editMonth  = 1, editDay    = 1;
int editHour   = 12,   editMinute = 0, editField  = 0;

// Schedule add editor
ScheduleItem scheduleDraft = {};
int scheduleAddStep = 0;     // 0=status 1=start 2=end 3=repeat 4=save/cancel
int scheduleAddField = 0;    // Y/M/D/H/M for start/end steps
int scheduleAddAction = 0;   // 0=save 1=cancel
bool scheduleAddFull = false;
bool scheduleAddInvalidRange = false;

// Runtime schedule override
bool scheduleOverrideActive = false;
int  scheduleActiveSlot = -1;
MyStatus scheduleRestoreStatus = ST_FREE;
uint32_t scheduleLastCheckMs = 0;

// Emoji home screen
float  SCALE           = 1.2f;
bool   homeOverlayDrawn = false;
time_t lastHomeSecond   = -1;
bool   sleepSceneDrawn  = false;
int    homeWifiIconState = -1;
int    headerWifiIconState = -1;
int    prevSelfHour = -1, prevSelfMinute = -1, prevSelfSecond = -1;
int    prevPartnerHour = -1, prevPartnerMinute = -1, prevPartnerSecond = -1;
bool   prevSelfTimeValid = false;
bool   prevPartnerTimeValid = false;

// World clock dirty-flags
bool worldLayoutReady      = false;
int  worldPrevIdx[4]       = {-1,-1,-1,-1};
int  worldPrevHour[4]      = {-1,-1,-1,-1};
int  worldPrevMinute[4]    = {-1,-1,-1,-1};
int  worldPrevSecond[4]    = {-1,-1,-1,-1};

// Screen static-drawn flags
bool tzListStaticDrawn    = false;
bool timeEditStaticDrawn  = false;

// Sleeping-bear Zzz animation
uint32_t zLastMs = 0;
uint8_t  zFrame  = 0;
Rect     zPrev[3] = {{0,0,0,0},{0,0,0,0},{0,0,0,0}};

// Partner sleeping-bear Zzz
uint32_t zLastMsSmall = 0;
uint8_t  zFrameSmall  = 0;
Rect     zPrevSmall[3] = {{0,0,0,0},{0,0,0,0},{0,0,0,0}};

// Wi-Fi page state
int  wifiMenuIndex = 0;
bool wifiResultOk  = false;
bool wifiInited    = false;

// Encoder
volatile int32_t encDelta     = 0;
volatile uint8_t encPrevState = 0;

bool gShortPressEvent = false;
bool gLongPressEvent  = false;

#endif
