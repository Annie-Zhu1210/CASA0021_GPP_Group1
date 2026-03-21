// This file covers the non-blocking Wi-Fi management for the device.


#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "globals.h"
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include "web_pages.h"
#include "config.h"
#include <esp_task_wdt.h>

#if ENABLE_UART_DEBUG
#define DBG_LOG(msg) Serial.println(msg)
#else
#define DBG_LOG(msg) \
  do { \
  } while (0)
#endif

static WebServer _wifiServer(80);
static DNSServer _dnsServer;

static String _savedSSID = "";
static String _savedPassword = "";
static String _savedPairing = "";

static String _escapeJson(const String& s) {
  String out;
  out.reserve(s.length() + 8);
  for (size_t i = 0; i < s.length(); i++) {
    char c = s[i];
    if (c == '\\' || c == '"') out += '\\';
    out += c;
  }
  return out;
}

enum class HotspotState { IDLE,
                          RUNNING,
                          SUBMITTED,
                          DONE };
static HotspotState _hotspotState = HotspotState::IDLE;

static uint32_t _lastReconnectAttemptMs = 0;
static constexpr uint32_t RECONNECT_INTERVAL_MS = 15000;
static constexpr int RECONNECT_MAX_TRIES = 3;

static void _loadCredentials() {
  Preferences pref;
  pref.begin(PREF_NS, true);
  _savedSSID = pref.getString(KEY_SSID, "");
  _savedPassword = pref.getString(KEY_PASSWORD, "");
  _savedPairing = pref.getString(KEY_PAIRING, "");
  pref.end();
}

static void _saveCredentials(const String& ssid,
                             const String& pass,
                             const String& pairing) {
  Preferences pref;
  pref.begin(PREF_NS, false);
  pref.putString(KEY_SSID, ssid);
  pref.putString(KEY_PASSWORD, pass);
  pref.putString(KEY_PAIRING, pairing);
  pref.end();
  _savedSSID = ssid;
  _savedPassword = pass;
  _savedPairing = pairing;
}

static bool _attemptConnect(const char* ssid, const char* pass,
                            int maxTries = RECONNECT_MAX_TRIES) {
  if (strlen(ssid) == 0) return false;
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  for (int t = 0; t < maxTries; t++) {
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      esp_task_wdt_reset();
      attempts++;
    }
    if (WiFi.status() == WL_CONNECTED) return true;
    delay(1000);
    esp_task_wdt_reset();
    WiFi.begin(ssid, pass);
  }
  return (WiFi.status() == WL_CONNECTED);
}

void wifiInit() {
  _loadCredentials();
  if (_savedSSID.length() == 0) {
    DBG_LOG("[WiFi] No saved credentials.");
    return;
  }
  DBG_LOG("[WiFi] Boot connect to: " + _savedSSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(_savedSSID.c_str(), _savedPassword.c_str());
  // Non-blocking — just kick off the connection, don't wait for it
  // wifiMaintainConnection() will retry in the background via loop()
  DBG_LOG("[WiFi] Connection started in background.");
}

void wifiMaintainConnection() {
  if (_hotspotState != HotspotState::IDLE) return;
  if (WiFi.status() == WL_CONNECTED) return;
  if (_savedSSID.length() == 0) return;
  uint32_t now = millis();
  if (now - _lastReconnectAttemptMs < RECONNECT_INTERVAL_MS) return;
  _lastReconnectAttemptMs = now;
  DBG_LOG("[WiFi] Disconnected - retrying...");
  WiFi.begin(_savedSSID.c_str(), _savedPassword.c_str());
}

static void _handleSave() {
  String newSSID = _wifiServer.arg("ssid");
  String newPass = _wifiServer.arg("password");
  String newPairing = _wifiServer.arg("pairing");
  DBG_LOG("[WiFi] Form received. SSID: " + newSSID);

  // Reject if pairing code doesn't match
  if (newPairing != PAIRING_CODE) {
    DBG_LOG("[WiFi] Wrong pairing code: " + newPairing);
    _wifiServer.send(200, "text/html",
      "<html><body style='font-family:Arial;text-align:center;padding:50px'>"
      "<h2 style='color:#c0392b'>Wrong pairing code</h2>"
      "<p>Please check the code with your partner and try again.</p>"
      "<a href='/'>Go back</a>"
      "</body></html>");
    return;
  }

  _wifiServer.send(200, "text/html", success_html);
  _dnsServer.stop();
  _wifiServer.stop();
  WiFi.softAPdisconnect(true);
  _saveCredentials(newSSID, newPass, newPairing);
  _hotspotState = HotspotState::SUBMITTED;
  bool ok = _attemptConnect(newSSID.c_str(), newPass.c_str());
  DBG_LOG(ok ? "[WiFi] Connected!" : "[WiFi] Connection failed.");
  wifiResultOk = ok;
  _hotspotState = HotspotState::DONE;
}

static void _handleScan() {
  int n = WiFi.scanNetworks(false, true);
  String json = "[";
  for (int i = 0; i < n; i++) {
    if (i > 0) json += ",";
    String ssid = _escapeJson(WiFi.SSID(i));
    int rssi = WiFi.RSSI(i);
    bool open = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN);
    json += "{\"ssid\":\"";
    json += ssid;
    json += "\",\"rssi\":";
    json += String(rssi);
    json += ",\"open\":";
    json += (open ? "true" : "false");
    json += "}";
  }
  json += "]";
  WiFi.scanDelete();
  _wifiServer.send(200, "application/json", json);
}

void wifiStartHotspot() {
  _hotspotState = HotspotState::RUNNING;
  wifiResultOk = false;
  WiFi.disconnect(true);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  IPAddress ip = WiFi.softAPIP();
  DBG_LOG("[WiFi] AP started. IP: " + ip.toString());
  _dnsServer.start(53, "*", ip);
  _wifiServer.on("/", HTTP_GET, []() {
    _wifiServer.send(200, "text/html", index_html);
  });
  _wifiServer.on("/scan", HTTP_GET, _handleScan);
  _wifiServer.on("/save", HTTP_POST, _handleSave);
  _wifiServer.onNotFound([]() {
    _wifiServer.send(200, "text/html", index_html);
  });
  _wifiServer.begin();
  DBG_LOG("[WiFi] Captive portal running on SSID: " AP_SSID);
}

void wifiStopHotspot() {
  if (_hotspotState == HotspotState::IDLE) return;
  _dnsServer.stop();
  _wifiServer.stop();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  _hotspotState = HotspotState::IDLE;
  DBG_LOG("[WiFi] Hotspot stopped by user.");
  // Resume background connection attempts with saved credentials
  if (_savedSSID.length() > 0) {
    WiFi.begin(_savedSSID.c_str(), _savedPassword.c_str());
  }
}

bool wifiPollHotspot() {
  switch (_hotspotState) {
    case HotspotState::IDLE: return true;
    case HotspotState::RUNNING:
      _dnsServer.processNextRequest();
      _wifiServer.handleClient();
      return false;
    case HotspotState::SUBMITTED: return false;
    case HotspotState::DONE:
      _hotspotState = HotspotState::IDLE;
      return true;
    default: return false;
  }
}

#endif
