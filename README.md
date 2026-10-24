# ⚡ ESP32 IoT Smart Energy Meter

An IoT-based Smart Energy and Environmental Monitoring system built on the **ESP32**. This project tracks real-time power consumption, cumulative energy, ambient temperature (with a 5-point moving average), and light levels, displaying data locally on an OLED screen and publishing telemetry via **MQTT**.

## 🚀 Features
- **Real-Time Power & Energy Tracking:** Simulates load sensing via analog input to calculate instant power ($W$) and cumulative energy ($Wh$).
- **Environmental Telemetry:** Tracks temperature using a DHT11 sensor with automatic outlier filtering, min/max tracking, and a 5-sample moving average.
- **Ambient Light Sensing:** Monitors surrounding light levels using an LDR.
- **Dual Mode Operation:** Publishes JSON payloads to an MQTT broker (`broker.hivemq.com`) over Wi-Fi, with automatic fallback to local OLED display mode if Wi-Fi is unavailable.
- **RTC Memory Retention:** Uses `RTC_DATA_ATTR` to preserve energy and temperature history across deep sleep cycles.

## 🛠️ Hardware & Pinout

| Component | ESP32 Pin | Notes |
| :--- | :--- | :--- |
| **DHT11 Sensor** | `GPIO 4` | Temperature sensing |
| **LDR (Light Sensor)** | `GPIO 34` | Analog input (ADC1) |
| **Potentiometer (Load Sim)**| `GPIO 35` | Analog input (ADC1) |
| **SSD1306 OLED (SDA)** | `GPIO 21` | Default I2C Data |
| **SSD1306 OLED (SCL)** | `GPIO 22` | Default I2C Clock |

## 📦 Required Libraries
Install the following libraries via the Arduino IDE Library Manager:
1. `PubSubClient` by Nick O'Leary
2. `DHT sensor library` by Adafruit
3. `Adafruit GFX Library` by Adafruit
4. `Adafruit SSD1306` by Adafruit

## 📡 MQTT Telemetry Format
- **Broker:** `broker.hivemq.com` (Port `1883`)
- **Topic:** `esp32/smartmeter/telemetry`
- **Publish Interval:** 30 seconds

**Sample JSON Payload:**
```json
{
  "temp": 26.40,
  "avgTemp": 26.15,
  "minTemp": 24.00,
  "maxTemp": 27.50,
  "ldr": 1845,
  "power": 102.45,
  "energy": 450.20
}
```
