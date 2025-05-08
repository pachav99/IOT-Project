// ==== Required Libraries ====
#include <WiFiS3.h>               // For WiFi connection (UNO R4 WiFi)
#include <WiFiSSLClient.h>        // SSL Client for secure MQTT
#include <PubSubClient.h>         // MQTT client
#include <DHT.h>                  // DHT11 Temperature/Humidity sensor
#include <Wire.h>                 // For I2C communication
#include <LiquidCrystal_I2C.h>    // I2C LCD
#include <ArduinoJson.h>          // For JSON payloads
#include <Adafruit_NeoPixel.h>    // RGB LED Strip (WS2812)
#include "secrets.h"              // WiFi & MQTT credentials

// ==== Sensor and Pin Definitions ====
#define DHTPIN 2                  // DHT11 data pin
#define DHTTYPE DHT11            // Sensor type
DHT dht(DHTPIN, DHTTYPE);        // Initialize DHT sensor

#define MOISTURE_PIN A0          // Analog pin for soil moisture
#define DRY_SOIL_THRESHOLD 50.0  // % threshold for dry soil

#define BUZZER_PIN 8             // Buzzer pin
#define PUMP_PIN 9               // Relay-controlled pump pin

// ==== LCD Configuration ====
LiquidCrystal_I2C lcd(0x27, 16, 2);  // LCD address and dimensions

// ==== NeoPixel LED Configuration ====
#define LED_PIN 6
#define NUM_PIXELS 8
Adafruit_NeoPixel strip(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

// ==== Networking Clients ====
WiFiSSLClient wifiSSLClient;
PubSubClient client(wifiSSLClient);

// ==== Status Variables ====
String previousDHTStatus = "unknown";
String previousMoistureStatus = "unknown";
unsigned long previousBlinkMillis = 0;
const long blinkInterval = 500;
bool blinkState = false;
String pumpStatus = "off";

// ==== Mario Melody Tune ====
int melody[] = {660, 660, 0, 660, 0, 520, 660, 0, 440};
int noteDurations[] = {100, 100, 100, 100, 100, 100, 100, 100, 100};

// ==== Play Short Mario Tune on Buzzer ====
void playMarioTheme() {
  for (int i = 0; i < 9; i++) {
    int note = melody[i];
    int duration = noteDurations[i];
    if (note == 0) {
      noTone(BUZZER_PIN); // Pause
    } else {
      tone(BUZZER_PIN, note, duration); // Play note
    }
    delay(duration + 30);
  }
  noTone(BUZZER_PIN); // Ensure buzzer turns off
}

// ==== WiFi Connection ====
void setup_wifi() {
  Serial.print("Connecting to WiFi");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting to");
  lcd.setCursor(0, 1);
  lcd.print("WiFi...");
  while (WiFi.begin(ssid, password) != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  lcd.clear();
  lcd.print("WiFi connected!");
}

// ==== MQTT Reconnection Logic ====
void reconnect() {
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    String clientId = "UNO_R4_Client_" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("MQTT connected.");
      client.publish(mqtt_status_topic, "device_online", true);
    } else {
      Serial.print("Failed, rc=");
      Serial.println(client.state());
      delay(3000); // Retry after delay
    }
  }
}

// ==== Update NeoPixel LED based on Moisture ====
void updateMoistureLED(float moisturePercent, bool blinkState) {
  if (moisturePercent < DRY_SOIL_THRESHOLD) {
    // Blink red if dry
    uint32_t color = blinkState ? strip.Color(255, 0, 0) : strip.Color(0, 0, 0);
    for (int i = 0; i < NUM_PIXELS; i++) {
      strip.setPixelColor(i, color);
    }
    strip.show();
  } else {
    // Green if healthy
    for (int i = 0; i < NUM_PIXELS; i++) {
      strip.setPixelColor(i, strip.Color(0, 255, 0));
    }
    strip.show();
  }
}

// ==== Initialization ====
void setup() {
  Serial.begin(9600);
  dht.begin();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");
  delay(2000);
  lcd.clear();

  strip.begin();
  strip.show(); // Turn off LEDs initially

  pinMode(PUMP_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, LOW);   // Pump off
  noTone(BUZZER_PIN);            // Buzzer off

  setup_wifi();                  // Connect to WiFi
  client.setServer(mqtt_server, mqtt_port); // Set MQTT broker
}

// ==== Main Loop ====
void loop() {
  // Handle blinking interval
  unsigned long currentMillis = millis();
  if (currentMillis - previousBlinkMillis >= blinkInterval) {
    blinkState = !blinkState;
    previousBlinkMillis = currentMillis;
  }

  // Reconnect WiFi/MQTT if needed
  if (WiFi.status() != WL_CONNECTED) setup_wifi();
  if (!client.connected()) reconnect();
  client.loop();

  // Read sensors
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  int moistureValue = analogRead(MOISTURE_PIN);
  float moisturePercent = map(moistureValue, 1023, 0, 0, 100); // Map to %

  // Determine sensor statuses
  String dhtStatus = (isnan(temperature) || isnan(humidity)) ? "offline" : "online";
  String moistureStatus = (moisturePercent < DRY_SOIL_THRESHOLD) ? "dry" : "normal";

  // Update LED indicator
  updateMoistureLED(moisturePercent, blinkState);

  // Auto water + alert if dry
  if (moistureStatus == "dry") {
    digitalWrite(PUMP_PIN, HIGH);
    playMarioTheme(); // Play buzzer alert
    pumpStatus = "on";
  } else {
    digitalWrite(PUMP_PIN, LOW);
    noTone(BUZZER_PIN);
    pumpStatus = "off";
  }

  // Display sensor readings
  if (dhtStatus == "online") {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("T:");
    lcd.print(temperature, 1);
    lcd.print((char)223);
    lcd.print("C H:");
    lcd.print(humidity, 0);
    lcd.setCursor(0, 1);
    lcd.print("Moist:");
    lcd.print(moisturePercent, 0);
    lcd.print("%");

    Serial.print("Temp: ");
    Serial.print(temperature);
    Serial.print("°C, Humidity: ");
    Serial.print(humidity);
    Serial.print("%, Moisture: ");
    Serial.print(moisturePercent);
    Serial.println("%");
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("DHT Sensor Err");
    Serial.println("Failed to read from DHT!");
  }

  // Prepare JSON data to publish
  StaticJsonDocument<256> json;
  json["sensor"] = "DHT11+Moisture";
  json["temperature"] = temperature;
  json["humidity"] = humidity;
  json["moisture_raw"] = moistureValue;
  json["moisture_percent"] = moisturePercent;
  json["dht_status"] = dhtStatus;
  json["moisture_status"] = moistureStatus;
  json["pump_status"] = pumpStatus;

  char payload[256];
  serializeJson(json, payload);
  client.publish(mqtt_topic_data, payload);  // Publish sensor data

  // Publish status change notifications
  if (dhtStatus != previousDHTStatus) {
    client.publish(mqtt_status_topic, ("dht:" + dhtStatus).c_str(), true);
    previousDHTStatus = dhtStatus;
  }

  if (moistureStatus != previousMoistureStatus) {
    client.publish(mqtt_status_topic, ("moisture:" + moistureStatus).c_str(), true);
    previousMoistureStatus = moistureStatus;

    // Send alert if dry
    if (moistureStatus == "dry") {
      client.publish(mqtt_alert_topic, "Alert: Soil is too dry!", true);
    }
  }

  delay(1000); // Short delay before next loop
}
