#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include "web_pages.h"

// External references to objects from main file
extern WebServer server;
extern DNSServer dnsServer;
extern Preferences preferences;
extern String saved_ssid;
extern String saved_password;
extern String pairing_code;
extern bool config_mode;

// Forward declaration
void startConfigMode();

bool connectToWiFi(const char* ssid, const char* password) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    Serial.print("Connecting to Wi-Fi");
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    Serial.println();
    
    return (WiFi.status() == WL_CONNECTED);
}

void handleSave() {
    String new_ssid = server.arg("ssid");
    String new_password = server.arg("password");
    String new_pairing = server.arg("pairing");
    
    Serial.println("\n=== Received Configuration ===");
    Serial.println("SSID: " + new_ssid);
    Serial.println("Pairing Code: " + new_pairing);
    
    // Save to persistent storage
    preferences.putString("ssid", new_ssid);
    preferences.putString("password", new_password);
    preferences.putString("pairing", new_pairing);
    
    saved_ssid = new_ssid;
    saved_password = new_password;
    pairing_code = new_pairing;
    
    // Send success page
    server.send(200, "text/html", success_html);
    
    delay(2000); // Give time for page to load
    
    // Stop AP and DNS server
    dnsServer.stop();
    server.stop();
    
    // Try to connect to the new Wi-Fi
    Serial.println("\nAttempting to connect to new Wi-Fi...");
    if (connectToWiFi(saved_ssid.c_str(), saved_password.c_str())) {
        Serial.println("Connected successfully!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        config_mode = false;
        WiFi.softAPdisconnect(true); // Turn off AP completely
    } else {
        Serial.println("Failed to connect - Restarting configuration mode");
        startConfigMode();
    }
}

void startConfigMode() {
    config_mode = true;
    
    // Stop any existing WiFi connection
    WiFi.disconnect();
    
    // Start Access Point
    Serial.println("Starting Access Point...");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
    
    // Start DNS server for captive portal (redirects all requests to ESP32)
    dnsServer.start(53, "*", IP);
    
    // Setup web server routes
    server.on("/", HTTP_GET, []() {
        server.send(200, "text/html", index_html);
    });
    
    server.on("/save", HTTP_POST, handleSave);
    
    // Catch-all for captive portal
    server.onNotFound([]() {
        server.send(200, "text/html", index_html);
    });
    
    server.begin();
    Serial.println("Web server started - Connect to '" + String(AP_SSID) + "' network");
}

#endif
