/*
 * This file handles MQTT publish/subscribe for the device.
 * Each device publishes its own status and epoch time, and subscribes
 * to the partner's status and time.
 *
 * Topic structure:
 * student/CASA0021Group1/device1/status
 * student/CASA0021Group1/device1/time
 * student/CASA0021Group1/device1/tz
 * student/CASA0021Group1/device2/status
 * student/CASA0021Group1/device2/time
 * student/CASA0021Group1/device2/tz
 *
 * All credentials and device identity are defined in config.h
 */

#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include "globals.h"
#include "config.h"  // MQTT_HOST, MQTT_PORT, MQTT_USER, MQTT_PASS,
                     // MQTT_BASE, DEVICE_ID  — all defined there
#include <PubSubClient.h>
#include <WiFi.h>

// Validate that config.h has everything we need
#ifndef MQTT_HOST
#error "MQTT_HOST is not defined. Please set it in config.h"
#endif
#ifndef MQTT_PORT
#error "MQTT_PORT is not defined. Please set it in config.h"
#endif
#ifndef MQTT_USER
#error "MQTT_USER is not defined. Please set it in config.h"
#endif
#ifndef MQTT_PASS
#error "MQTT_PASS is not defined. Please set it in config.h"
#endif
#ifndef MQTT_BASE
#error "MQTT_BASE is not defined. Please set it in config.h"
#endif
#ifndef DEVICE_ID
#error "DEVICE_ID is not defined. Please set it to 1 or 2 in config.h"
#endif

// Partner device is whichever this device is not
#if DEVICE_ID == 1
  #define DEVICE_ID_STR  "device1"
  #define PARTNER_ID_STR "device2"
#elif DEVICE_ID == 2
  #define DEVICE_ID_STR  "device2"
  #define PARTNER_ID_STR "device1"
#else
  #error "DEVICE_ID must be 1 or 2 in config.h"
#endif


// ── Topic helpers ─────────────────────────────────────────────────────────────
static const char TOPIC_MY_STATUS[] = MQTT_BASE "/" DEVICE_ID_STR "/status";
static const char TOPIC_MY_TIME[]   = MQTT_BASE "/" DEVICE_ID_STR "/time";
static const char TOPIC_MY_TZ[]     = MQTT_BASE "/" DEVICE_ID_STR "/tz";
static const char TOPIC_MY_HB[]     = MQTT_BASE "/" DEVICE_ID_STR "/hb";

static const char TOPIC_PT_STATUS[] = MQTT_BASE "/" PARTNER_ID_STR "/status";
static const char TOPIC_PT_TIME[]   = MQTT_BASE "/" PARTNER_ID_STR "/time";
static const char TOPIC_PT_TZ[]     = MQTT_BASE "/" PARTNER_ID_STR "/tz";
static const char TOPIC_PT_HB[]     = MQTT_BASE "/" PARTNER_ID_STR "/hb";

// Internal state
static WiFiClient _mqttWifiClient;
static PubSubClient _mqttClient(_mqttWifiClient);

static uint32_t _mqttLastPublishMs = 0;
static uint32_t _mqttLastReconnectMs = 0;
static uint32_t _mqttConnectedSinceMs = 0;
static constexpr uint32_t MQTT_PUBLISH_INTERVAL_MS = 5000;
static constexpr uint32_t MQTT_RECONNECT_INTERVAL_MS = 10000;
static constexpr uint32_t PARTNER_OFFLINE_TIMEOUT_MS = 30000;

// Track last-published values to avoid redundant publishes
static int _lastPublishedStatus = -1;
static int _lastPublishedTz = -1;
static void _mqttPublish();

static void _markPartnerSeen(time_t seenEpoch = 0) {
  partnerLastSeenMs = millis();
  if (seenEpoch > 100000) partnerLastSeenEpoch = seenEpoch;
  else {
    time_t now = time(nullptr);
    if (now > 100000) partnerLastSeenEpoch = now;
  }
  partnerPresenceKnown = true;
  partnerOfflineSinceEpoch = 0;
  if (!partnerOnline) {
    partnerOnline = true;
    partnerStatusDirty = true;
    partnerInfoDirty = true;
  }
}

static void _mqttPublishHeartbeatNow() {
  if (!_mqttClient.connected()) return;
  char buf[24];
  time_t now = time(nullptr);
  if (now > 100000) snprintf(buf, sizeof(buf), "%ld", (long)now);
  else snprintf(buf, sizeof(buf), "0");
  _mqttClient.publish(TOPIC_MY_HB, buf, false);
}

// Callback: called when a subscribed message arrives
static void _mqttCallback(char* topic, byte* payload, unsigned int length) {
  char buf[32];
  unsigned int len = (length < sizeof(buf) - 1) ? length : sizeof(buf) - 1;
  memcpy(buf, payload, len);
  buf[len] = '\0';

  if (strcmp(topic, TOPIC_PT_STATUS) == 0) {
    int s = atoi(buf);
    if (s >= 0 && s < ST_COUNT) {
      MyStatus incoming = (MyStatus)s;
      if (incoming != partnerStatus) {
        partnerStatus = incoming;
        partnerStatusDirty = true;
      }
    }
  } else if (strcmp(topic, TOPIC_PT_TIME) == 0) {
    long epoch = atol(buf);
    if (epoch > 100000) {
      partnerEpoch = (time_t)epoch;
      partnerLastSeenEpoch = (time_t)epoch;
      partnerTimeValid = true;
      partnerInfoDirty = true;
    }
  } else if (strcmp(topic, TOPIC_PT_TZ) == 0) {
    int idx = atoi(buf);
    if (idx >= 0 && idx < TZ_COUNT) {
      partnerTzIndex = idx;
      partnerInfoDirty = true;
    }
  } else if (strcmp(topic, TOPIC_PT_HB) == 0) {
    long ep = atol(buf);
    _markPartnerSeen((ep > 100000) ? (time_t)ep : 0);
  }
}

// Subscribe to partner topics
static void _mqttSubscribe() {
  _mqttClient.subscribe(TOPIC_PT_STATUS);
  _mqttClient.subscribe(TOPIC_PT_TIME);
  _mqttClient.subscribe(TOPIC_PT_TZ);
  _mqttClient.subscribe(TOPIC_PT_HB);
}

// Connect / reconnect
static bool _mqttConnect() {
  if (WiFi.status() != WL_CONNECTED) return false;

  char clientId[32];
  snprintf(clientId, sizeof(clientId), "LD-%s-%04X",
         DEVICE_ID_STR,
         (uint16_t)(ESP.getEfuseMac() & 0xFFFF));

  if (_mqttClient.connect(clientId, MQTT_USER, MQTT_PASS)) {
    _mqttSubscribe();
    _lastPublishedStatus = -1;
    _lastPublishedTz = -1;
    mqttConnected = true;
    _mqttConnectedSinceMs = millis();
    _mqttPublishHeartbeatNow();  // announce presence immediately after connect
    _mqttPublish();              // publish status/tz/time immediately as well
    _mqttLastPublishMs = millis();
    return true;
  }
  return false;
}

// Publish own status, time, and timezone
static void _mqttPublish() {
  char buf[24];

  if ((int)myStatus != _lastPublishedStatus) {
    snprintf(buf, sizeof(buf), "%d", (int)myStatus);
    _mqttClient.publish(TOPIC_MY_STATUS, buf, true);
    _lastPublishedStatus = (int)myStatus;
  }

  if (tzIndex != _lastPublishedTz) {
    snprintf(buf, sizeof(buf), "%d", tzIndex);
    _mqttClient.publish(TOPIC_MY_TZ, buf, true);
    _lastPublishedTz = tzIndex;
  }

  time_t now = time(nullptr);
  if (now > 100000) {
    snprintf(buf, sizeof(buf), "%ld", (long)now);
    _mqttClient.publish(TOPIC_MY_TIME, buf, false);
  }
  if (now > 100000) snprintf(buf, sizeof(buf), "%ld", (long)now);
  else snprintf(buf, sizeof(buf), "0");
  _mqttClient.publish(TOPIC_MY_HB, buf, false);
}

// Public API

void mqttInit() {
  _mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  _mqttClient.setCallback(_mqttCallback);
  _mqttClient.setKeepAlive(60);
  _mqttClient.setSocketTimeout(5);
}


void mqttLoop() {
  uint32_t now = millis();
  if (WiFi.status() != WL_CONNECTED) {
    mqttConnected = false;
    if (partnerOnline || partnerPresenceKnown) {
      partnerOnline = false;
      partnerPresenceKnown = false;  // unknown while local network is down
      partnerTimeValid = false;
      partnerStatusDirty = true;
      partnerInfoDirty = true;
    }
    return;
  }

  if (!_mqttClient.connected()) {
    mqttConnected = false;
    if (now - _mqttLastReconnectMs >= MQTT_RECONNECT_INTERVAL_MS) {
      _mqttLastReconnectMs = now;
      _mqttConnect();
    }
    return;
  }

  mqttConnected = true;
  _mqttClient.loop();

  // If we've been connected for a while but have never seen partner heartbeat,
  // mark as offline (known).
  if (!partnerPresenceKnown && (now - _mqttConnectedSinceMs >= PARTNER_OFFLINE_TIMEOUT_MS)) {
    partnerPresenceKnown = true;
    partnerOnline = false;
    partnerTimeValid = false;
    time_t ep = time(nullptr);
    if (ep > 100000) partnerOfflineSinceEpoch = ep;
    partnerStatusDirty = true;
    partnerInfoDirty = true;
  }

  if (partnerOnline && (now - partnerLastSeenMs >= PARTNER_OFFLINE_TIMEOUT_MS)) {
    partnerOnline = false;
    partnerPresenceKnown = true;
    partnerTimeValid = false;
    time_t ep = time(nullptr);
    if (ep > 100000) partnerOfflineSinceEpoch = ep;
    partnerStatusDirty = true;
    partnerInfoDirty = true;
  }

  if (now - _mqttLastPublishMs >= MQTT_PUBLISH_INTERVAL_MS) {
    _mqttLastPublishMs = now;
    _mqttPublish();
  }
}

void mqttPublishStatusNow() {
  if (!_mqttClient.connected()) return;
  char buf[8];
  snprintf(buf, sizeof(buf), "%d", (int)myStatus);
  _mqttClient.publish(TOPIC_MY_STATUS, buf, true);
  _lastPublishedStatus = (int)myStatus;
}

#endif
