#ifndef CONFIG_H
#define CONFIG_H

// Hotspot Configuration
#define AP_SSID "ProductName-Setup"
#define AP_PASSWORD "12345678"  // minimum 8 characters

// Device Identity
#define DEVICE_ID 1  // Change to 2 for another device

// MQTT Broker
#define MQTT_HOST "your.mqtt.broker.host"
#define MQTT_PORT 1884
#define MQTT_USER "your_mqtt_username"
#define MQTT_PASS "your_mqtt_password"
#define MQTT_BASE "student/CASA0021Group1"

// Instructions:
//
// 1. Copy this file and rename it to "config.h"
// 2. Fill in your actual AP_SSID, AP_PASSWORD, MQTT credentials, and DEVICE_ID

#endif
