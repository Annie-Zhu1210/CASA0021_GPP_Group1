#ifndef CONFIG_H
#define CONFIG_H


// Hotspot Configuration
#define AP_SSID "ProductName-Setup"
#define AP_PASSWORD "12345678" //minimum 8 characters


// Hold the ESP32 board boost button for 3 seconds to re-enter the config mode and reload the hotspot.
// Note: this is just for testing the hotspot functionality. 
// Later when the circuit has been completed, the user-firendly Wi-Fi information reset functionality will be controlled by the KY040 button.
const unsigned long LONG_PRESS_TIME = 3000;


// Instructions:
// 
// 1. Copy this file and rename it to "config.h"
// 2. Modify the values above

#endif