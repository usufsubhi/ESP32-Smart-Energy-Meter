#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const char* ssid = "id";
const char* password = "123123!@#";
const char* mqtt_server = "broker.hivemq.com";

WiFiClient espClient;
PubSubClient client(espClient);

#define DHTPIN 4
#define DHTTYPE DHT11
#define LDR_PIN 34
#define POT_PIN 35

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

DHT dht(DHTPIN, DHTTYPE);

RTC_DATA_ATTR float cumulativeEnergy = 0.0;
RTC_DATA_ATTR float tempHistory[5] = {0};
RTC_DATA_ATTR int tempIndex = 0;
RTC_DATA_ATTR int sampleCount = 0;
RTC_DATA_ATTR float maxTemp = -999.0;
RTC_DATA_ATTR float minTemp = 999.0;

unsigned long lastPublishTime = 0;
const long interval = 30000;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 20) {
    delay(500);
    Serial.print(".");
    tries++;
  }
  if(WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
  } else {
    Serial.println("\nWiFi Failed, running in local display mode.");
  }
}

void reconnect_mqtt() {
  if (WiFi.status() != WL_CONNECTED) return;
  if (!client.connected()) {
    String clientId = "ESP32SmartMeter-" + String(random(0xffff), HEX);
    client.connect(clientId.c_str());
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while(1);
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Smart Energy Meter");
  display.println("Initializing...");
  display.display();

  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!client.connected()) {
      reconnect_mqtt();
    }
    client.loop();
  }

  unsigned long currentMillis = millis();

  if (currentMillis - lastPublishTime >= interval || lastPublishTime == 0) {
    lastPublishTime = currentMillis;

    float currentTemp = dht.readTemperature();
    int potValue = analogRead(POT_PIN);
    int ldrValue = analogRead(LDR_PIN);

    if (isnan(currentTemp) || currentTemp > 60 || currentTemp < -10) {
      Serial.println("Invalid temperature reading, skipping...");
      return;
    }

    tempHistory[tempIndex] = currentTemp;
    tempIndex = (tempIndex + 1) % 5;
    if (sampleCount < 5) sampleCount++;
    
    float tempSum = 0;
    for (int i = 0; i < sampleCount; i++) {
      tempSum += tempHistory[i];
    }
    float movingAvgTemp = tempSum / sampleCount;

    if (currentTemp > maxTemp) maxTemp = currentTemp;
    if (currentTemp < minTemp) minTemp = currentTemp;

    float scalingFactor = 0.05;
    float instantPower = potValue * scalingFactor;

    cumulativeEnergy += instantPower;

    Serial.println("\n========== [Smart Meter Telemetry - 30s] ==========");
    Serial.print("Current Temp:      "); Serial.print(currentTemp); Serial.println(" C");
    Serial.print("Moving Avg (5):    "); Serial.print(movingAvgTemp); Serial.println(" C");
    Serial.print("Max / Min Temp:    "); Serial.print(maxTemp); Serial.print(" / "); Serial.print(minTemp); Serial.println(" C");
    Serial.print("Light Level (LDR): "); Serial.println(ldrValue);
    Serial.print("Instant Power:     "); Serial.print(instantPower); Serial.println(" W");
    Serial.print("Cumulative Energy: "); Serial.print(cumulativeEnergy); Serial.println(" Wh");

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("--- SMART METER ---");
    display.print("Temp:  "); display.print(currentTemp, 1); display.println(" C");
    display.print("AvgT:  "); display.print(movingAvgTemp, 1); display.println(" C");
    display.print("Min/Mx:"); display.print(minTemp, 0); display.print("/"); display.print(maxTemp, 0); display.println(" C");
    display.print("Power: "); display.print(instantPower, 1); display.println(" W");
    display.print("Total: "); display.print(cumulativeEnergy, 1); display.println(" Wh");
    display.display();

    if (client.connected()) {
      String payload = "{";
      payload += "\"temp\":" + String(currentTemp, 2) + ",";
      payload += "\"avgTemp\":" + String(movingAvgTemp, 2) + ",";
      payload += "\"minTemp\":" + String(minTemp, 2) + ",";
      payload += "\"maxTemp\":" + String(maxTemp, 2) + ",";
      payload += "\"ldr\":" + String(ldrValue) + ",";
      payload += "\"power\":" + String(instantPower, 2) + ",";
      payload += "\"energy\":" + String(cumulativeEnergy, 2);
      payload += "}";

      client.publish("esp32/smartmeter/telemetry", payload.c_str());
      Serial.println("Published via MQTT: " + payload);
    }
  }
}
