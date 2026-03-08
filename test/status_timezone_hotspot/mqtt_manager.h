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

static const char TOPIC_PT_STATUS[] = MQTT_BASE "/" PARTNER_ID_STR "/status";
static const char TOPIC_PT_TIME[]   = MQTT_BASE "/" PARTNER_ID_STR "/time";
static const char TOPIC_PT_TZ[]     = MQTT_BASE "/" PARTNER_ID_STR "/tz";

// Internal state
static WiFiClient _mqttWifiClient;
static PubSubClient _mqttClient(_mqttWifiClient);

static uint32_t _mqttLastPublishMs = 0;
static uint32_t _mqttLastReconnectMs = 0;
static constexpr uint32_t MQTT_PUBLISH_INTERVAL_MS = 5000;
static constexpr uint32_t MQTT_RECONNECT_INTERVAL_MS = 10000;

// Track last-published values to avoid redundant publishes
static int _lastPublishedStatus = -1;
static int _lastPublishedTz = -1;

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
      partnerTimeValid = true;
      partnerInfoDirty = true;
    }
  } else if (strcmp(topic, TOPIC_PT_TZ) == 0) {
    int idx = atoi(buf);
    if (idx >= 0 && idx < TZ_COUNT) {
      partnerTzIndex = idx;
      partnerInfoDirty = true;
    }
  }
}

// Subscribe to partner topics
static void _mqttSubscribe() {
  _mqttClient.subscribe(TOPIC_PT_STATUS);
  _mqttClient.subscribe(TOPIC_PT_TIME);
  _mqttClient.subscribe(TOPIC_PT_TZ);
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
}

// Public API

void mqttInit() {
  _mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  _mqttClient.setCallback(_mqttCallback);
  _mqttClient.setKeepAlive(60);
  _mqttClient.setSocketTimeout(5);
}


void mqttLoop() {
  if (WiFi.status() != WL_CONNECTED) return;

  if (!_mqttClient.connected()) {
    uint32_t now = millis();
    if (now - _mqttLastReconnectMs >= MQTT_RECONNECT_INTERVAL_MS) {
      _mqttLastReconnectMs = now;
      _mqttConnect();
    }
    return;
  }

  _mqttClient.loop();

  uint32_t now = millis();
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