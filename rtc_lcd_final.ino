/*
  Arduino UNO R4 WiFi - RTC synced over WiFi -> 16x2 I2C LCD

  Display layout (16 columns, starts at the left edge):
    Time: 09:06:23
    Date: 22/11/2026

  Libraries: "LiquidCrystal I2C" (Frank de Brabander). WiFiS3 and RTC come with the UNO R4 core.
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFiS3.h>
#include "RTC.h"

const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";

// Time zone offset in seconds from UTC. India (IST, UTC+5:30) = 19800
const long TZ_OFFSET = 19800;

LiquidCrystal_I2C lcd(0x27, 16, 2);   // change to 0x3F if the screen stays blank

bool syncTime() {
  lcd.clear();
  lcd.print("Connecting WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - start > 20000) return false;
    delay(500);
  }

  lcd.clear();
  lcd.print("Getting time...");
  unsigned long epoch = 0;
  for (int i = 0; i < 10 && epoch < 1700000000UL; i++) {
    epoch = WiFi.getTime();
    delay(500);
  }
  if (epoch < 1700000000UL) return false;

  RTCTime t;
  t.setUnixTime((time_t)(epoch + TZ_OFFSET));
  RTC.setTime(t);
  return true;
}

void setup() {
  lcd.init();
  lcd.backlight();
  RTC.begin();

  if (!syncTime()) {
    lcd.clear();
    lcd.print("Sync failed!");
    lcd.setCursor(0, 1);
    lcd.print("Check WiFi/pass");
    while (true) delay(1000);
  }
  lcd.clear();
}

void loop() {
  RTCTime now;
  RTC.getTime(now);

  char line[17];

  char text[24];

  // Top row: "Time: HH:MM:SS" (padded to 16 characters to clear leftovers)
  snprintf(text, sizeof(text), "Time: %02d:%02d:%02d",
           now.getHour(), now.getMinutes(), now.getSeconds());
  snprintf(line, sizeof(line), "%-16s", text);
  lcd.setCursor(0, 0);
  lcd.print(line);

  // Bottom row: "Date: D/M/YYYY" (no leading zeros on day and month)
  snprintf(text, sizeof(text), "Date: %d/%d/%04d",
           now.getDayOfMonth(), Month2int(now.getMonth()), now.getYear());
  snprintf(line, sizeof(line), "%-16s", text);
  lcd.setCursor(0, 1);
  lcd.print(line);

  delay(250);
}
