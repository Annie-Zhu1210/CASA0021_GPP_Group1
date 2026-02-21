#include <Wire.h>
#include <Waveshare_LCD1602_RGB.h>

Waveshare_LCD1602_RGB lcd(16, 2);


int count = 0;

void setup() {
  lcd.init();

  // Adjust contrast (0–63)
  //lcd.setContrast(45);   // ← TRY VALUES 30–55 UNTIL TEXT IS CLEAR

  // Set full white backlight
  lcd.setRGB(125, 125, 125);

  lcd.setCursor(0, 0);
  lcd.send_string("Hello World!");
}

void loop() {
  lcd.send_string("Goodbye World!");
  
  count++;
}
