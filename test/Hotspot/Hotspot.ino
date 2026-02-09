#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <DNSServer.h>
#include "config.h"
#include "wifi_manager.h"
#include "web_pages.h"

// Objects
WebServer server(80);
DNSServer dnsServer;
Preferences preferences;

// Variables
String saved_ssid = "";
String saved_password = "";
String pairing_code = "";
bool config_mode = false;
const int CONFIG_BUTTON_PIN = 0;
unsigned long button_press_start = 0;

void setup() {
    Serial.begin(115200);
    pinMode(CONFIG_BUTTON_PIN, INPUT_PULLUP);
    
    // Initialize preferences (persistent storage)
    preferences.begin("wifi-config", false);
    
    // Load saved credentials
    saved_ssid = preferences.getString("ssid", "");
    saved_password = preferences.getString("password", "");
    pairing_code = preferences.getString("pairing", "");
    
    Serial.println("\n=== Long Distance Device Starting ===");
    
    // Check if credentials have been saved
    if (saved_ssid.length() > 0) {
        Serial.println("Found saved Wi-Fi credentials");
        Serial.println("Attempting to connect to: " + saved_ssid);
        
        // Connect to saved Wi-Fi
        if (connectToWiFi(saved_ssid.c_str(), saved_password.c_str())) {
            Serial.println("Connected successfully!");
            Serial.print("IP Address: ");
            Serial.println(WiFi.localIP());
            config_mode = false;
        } else {
            Serial.println("Connection failed - Starting configuration mode");
            startConfigMode();
        }
    } else {
        Serial.println("No saved credentials - Starting configuration mode");
        startConfigMode();
    }
}

void loop() {
    // Check for long press on boot button to re-enter config mode
    checkConfigButton();
    
    if (config_mode) {
        dnsServer.processNextRequest();
        server.handleClient();
    } else {
        // Normal operation mode
        
        // Check if Wi-Fi disconnected
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("Wi-Fi disconnected! Attempting to reconnect...");
            if (!connectToWiFi(saved_ssid.c_str(), saved_password.c_str())) {
                Serial.println("Reconnection failed - Starting configuration mode");
                startConfigMode();
            }
        }
    }
    
    delay(10);
}

void checkConfigButton() {
    // Check if boot button on ESP32 board is held for reconfiguration
    if (digitalRead(CONFIG_BUTTON_PIN) == LOW) {
        if (button_press_start == 0) {
            button_press_start = millis();
        } else if (millis() - button_press_start >= LONG_PRESS_TIME) {
            Serial.println("\n=== Long press detected - Entering configuration mode ===");
            startConfigMode();
            button_press_start = 0;
        }
    } else {
        button_press_start = 0;
    }
}