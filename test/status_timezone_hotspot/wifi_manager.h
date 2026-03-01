// This file covers the non-blocking Wi-Fi management for the device.


#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "globals.h"
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include "web_pages.h"
#include "config.h"

static WebServer _wifiServer(80);
static DNSServer _dnsServer;

static String _savedSSID = "";
static String _savedPassword = "";
static String _savedPairing = "";

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
      attempts++;
    }
    if (WiFi.status() == WL_CONNECTED) return true;
    delay(1000);
    WiFi.begin(ssid, pass);
  }
  return (WiFi.status() == WL_CONNECTED);
}

void wifiInit() {
  _loadCredentials();
  if (_savedSSID.length() > 0) {
    Serial.println("[WiFi] Boot connect to: " + _savedSSID);
    _attemptConnect(_savedSSID.c_str(), _savedPassword.c_str());
    if (WiFi.status() == WL_CONNECTED)
      Serial.println("[WiFi] Connected. IP: " + WiFi.localIP().toString());
    else
      Serial.println("[WiFi] Boot connect failed — will retry in background.");
  } else {
    Serial.println("[WiFi] No saved credentials.");
  }
}

void wifiMaintainConnection() {
  if (_hotspotState != HotspotState::IDLE) return;
  if (WiFi.status() == WL_CONNECTED) return;
  if (_savedSSID.length() == 0) return;
  uint32_t now = millis();
  if (now - _lastReconnectAttemptMs < RECONNECT_INTERVAL_MS) return;
  _lastReconnectAttemptMs = now;
  Serial.println("[WiFi] Disconnected — retrying...");
  WiFi.begin(_savedSSID.c_str(), _savedPassword.c_str());
}

static void _handleSave() {
  String newSSID = _wifiServer.arg("ssid");
  String newPass = _wifiServer.arg("password");
  String newPairing = _wifiServer.arg("pairing");
  Serial.println("[WiFi] Form received. SSID: " + newSSID);
  _wifiServer.send(200, "text/html", success_html);
  _dnsServer.stop();
  _wifiServer.stop();
  WiFi.softAPdisconnect(true);
  _saveCredentials(newSSID, newPass, newPairing);
  _hotspotState = HotspotState::SUBMITTED;
  bool ok = _attemptConnect(newSSID.c_str(), newPass.c_str());
  Serial.println(ok ? "[WiFi] Connected!" : "[WiFi] Connection failed.");
  wifiResultOk = ok;
  _hotspotState = HotspotState::DONE;
}

void wifiStartHotspot() {
  _hotspotState = HotspotState::RUNNING;
  wifiResultOk = false;
  WiFi.disconnect(true);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  IPAddress ip = WiFi.softAPIP();
  Serial.println("[WiFi] AP started. IP: " + ip.toString());
  _dnsServer.start(53, "*", ip);
  _wifiServer.on("/", HTTP_GET, []() {
    _wifiServer.send(200, "text/html", index_html);
  });
  _wifiServer.on("/save", HTTP_POST, _handleSave);
  _wifiServer.onNotFound([]() {
    _wifiServer.send(200, "text/html", index_html);
  });
  _wifiServer.begin();
  Serial.println("[WiFi] Captive portal running on SSID: " AP_SSID);
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