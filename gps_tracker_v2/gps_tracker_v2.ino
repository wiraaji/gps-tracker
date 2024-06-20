#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include "ThingSpeak.h"
#include <TinyGPS++.h>
#include <HardwareSerial.h>

TinyGPSPlus gps;  // The TinyGPS++ object
String lat_data, lng_data;
#define GPS_BAUDRATE 9600

const char ssid[] = "Guest_Prod";       // Wi-Fi SSID
const char pass[] = "nihonseikiid123";  // Wi-Fi Password
WiFiClient client;

unsigned long myChannelNumber = 2567587;
const char *myWriteAPIKey = "QL8EIZ0P7RJCCJA7";

unsigned long previousMillis_ts = 0;
const long ts_update_interval = 20000;  // Update data every 20s

LiquidCrystal_I2C lcd(0x27, 16, 2);  // Set the LCD I2C address

void setup() {
  // Initialize the LCD
  lcd.init();
  lcd.backlight();
  lcd.print("Initializing...");
  Serial.begin(9600);
  Serial2.begin(GPS_BAUDRATE, SERIAL_8N1, 16, 17);  // Serial2 for GPS UART

  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);  // Initialize ThingSpeak
  connectToWiFi();
}

void loop() {
  if (Serial2.available() > 0) {
    if (gps.encode(Serial2.read())) {
      if (gps.location.isValid()) {
        lat_data = String(gps.location.lat(), 8);
        lng_data = String(gps.location.lng(), 8);

        // Display the data on the LCD
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Lat: ");
        lcd.print(lat_data);
        lcd.setCursor(0, 1);
        lcd.print("Lng: ");
        lcd.print(lng_data);

        Serial.print(F("Lat: "));
        Serial.print(gps.location.lat(), 6);
        Serial.print(F(" , Longitude: "));
        Serial.println(gps.location.lng(), 6);
      } else {
        lcd.clear();
        lcd.print("Location: INVALID");
        Serial.println(F("Location: INVALID"));
      }
      Serial.println();
    }
  }

  if (millis() > 5000 && gps.charsProcessed() < 10) {
    Serial.println(F("No GPS data received: check wiring"));
    lcd.print("No GPS data received: check wiring");
  }

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis_ts >= ts_update_interval) {
    previousMillis_ts = currentMillis;
    if (WiFi.status() != WL_CONNECTED) {
      reconnectWiFi();
    }

    ThingSpeak.setField(1, lat_data);  // Field for latitude
    ThingSpeak.setField(2, lng_data);  // Field for longitude
    Serial.println(lat_data);
    Serial.println(lng_data);
    delay(2000);

    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    if (x == 200) {
      Serial.println("Successfully updated the channel.");
    } else {
      Serial.println("Failed to update the channel. HTTP error code: " + String(x));
    }
    Serial.println();
  }
}

void connectToWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("Connecting");
    lcd.setCursor(0, 1);
    lcd.print(ssid);
    Serial.print("Connecting");
    Serial.println(ssid);
    WiFi.begin(ssid, pass);
    int i = 0;
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      // lcd.setCursor(12, 0);
      lcd.print(".");
      ++i;

      if (i == 20) {
        i = 0;
        Serial.println("\nFailed to connect.");
        lcd.println("\nFailed to connect.");
        break;
      }
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nConnected!");
      lcd.println("\nConnected!");
    }
    delay(3000);
  }
}

void reconnectWiFi() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Reconnecting");
  lcd.println(ssid);
  Serial.print("Reconnecting");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    lcd.print(".");
    ++i;

    if (i == 20) {
      i = 0;
      Serial.println("\nFailed to reconnect.");
      lcd.println("\nFailed to reconnect.");
      break;
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nReconnected!");
    lcd.println("\nReconnected!");
  }
}
