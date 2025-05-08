# 🌿 Smar Plant Monitor & Auto Watering System

This project implements a complete IoT-based smart irrigation system using an UNO R4, MQTT messaging, Telegraf with Podman, InfluxDB, and Grafana dashboards. The system monitors environmental conditions in real time and predicts pump usage for maintenance planning.

## 🔧 How It Works

### 1. 📟 **Arduino (UNO R4) + Sensors**
- Reads values from:
  - DHT11 (temperature & humidity)
  - Soil moisture sensor
  - Pump digital output status
- Publishes data every few seconds to the MQTT broker in JSON format on topic: `sensor/data`.

### 2. 🔌 **MQTT + Telegraf + Podman**
- Telegraf is containerized using **Podman**.
- MQTT input plugin listens to the topic `sensor/data`.
- Telegraf parses the JSON and sends data to **InfluxDB v2**.
- All credentials are stored in `.env` file and excluded via `.gitignore`.

### 3. 📊 **InfluxDB**
- Time-series data is stored in the `iot_data` bucket under the `iot_sensor` measurement.
- Fields include:
  - `temperature`, `humidity`, `moisture_percent`, `pump_status`, `dht_status`

### 4. 📈 **Grafana Dashboards**
- Connected to InfluxDB as a data source.
- Dashboards include:

#### ▶️ **Live Monitoring**
- Real-time panel with temperature, humidity, and moisture % updated every few seconds.

#### 🔮 **Pump Prediction**
- Displays:
  - Total runtime (in minutes)
  - Estimated days left (based on 100,000-minute lifespan)
  - Daily pump ON count

#### 🚨 **Pump Status Tracker**
- Smart alert logic:
  - ✅ OK: Moisture < 50 & Pump ON
  - ⚪ Normal: Moisture ≥ 50 & Pump OFF
  - ❌ Error: Moisture < 50 & Pump OFF
- System never entered error — indicating reliable pump logic.

---

## ✅ What We Learned

This project helped us:
- Integrate hardware (sensors + UNO R4) with software and cloud tools
- Use MQTT and containerized Telegraf (via Podman) to build scalable pipelines
- Securely handle credentials using `.env` and `.gitignore`
- Store and query time-series data efficiently with InfluxDB
- Build professional Grafana dashboards with real-time and historical insights

---

## 💡 Technologies Used

- Arduino (UNO R4)
- MQTT (PubSubClient)
- Podman (for running Telegraf)
- Telegraf (Location : SAD\sad\telegraf.conf)
- InfluxDB v2
- Grafana
- PowerShell (.ps1) for environment setup

---

## 📦 Getting Started

1. Flash `project1.ino` to UNO R4 with your Wi-Fi & MQTT details in `secrets.h`
2. Set up Telegraf using `telegraf.conf` and load `.env` using `load-env.ps1`
3. Start Podman container for Telegraf
4. Verify data in InfluxDB
5. Import Grafana dashboards from `/grafana Dashboards/*.txt`



