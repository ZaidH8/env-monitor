# Smart Environment Monitor

A complete end-to-end IoT monitoring system built on Raspberry Pi 5. Reads 
temperature, humidity, and room occupancy from physical sensors, streams data 
over MQTT, stores it in a SQLite database, and displays it on a live web 
dashboard accessible from any device on the network.



## Demo

[Dashboard](docs/dashboard.png)



## Features

- Real-time temperature and humidity monitoring via DHT22 sensor
- Room occupancy detection via HC-SR04 ultrasonic sensor  
- Live web dashboard with WebSocket push — updates instantly with no page refresh
- Historical data charts with 1 hour, 6 hour, and 24 hour range selection
- Daily statistics page showing min, max, average, and occupancy percentage
- Entire system starts automatically on boot via Linux systemd services
- Data persisted in SQLite database with timestamp indexing


## System Architecture
[DHT22 + HC-SR04]
        │
        ▼
[C++ Sensor Daemon]     ← libgpiod, reads GPIO pins directly
        │
        ▼ MQTT (QoS 1)
[Mosquitto Broker]
        │
        ▼
[Python Backend]        ← paho-mqtt subscriber, validates and stores data
        │
        ├──→ [SQLite Database]
        │
        └──→ [Flask REST API + WebSockets]
        │
        ▼
[React Dashboard]  ← live updates via WebSocket


## Tech Stack

| Layer | Technology | Why |
|---|---|---|
| Firmware | C++17, libgpiod | Direct GPIO access, industry standard on embedded Linux |
| Messaging | MQTT, Mosquitto | Standard IoT protocol used in AWS IoT, Azure IoT Hub |
| Backend | Python 3, Flask, Flask-SocketIO | REST API + real-time WebSocket push |
| Database | SQLite | Lightweight time-series storage with indexed queries |
| Frontend | React, Recharts, socket.io-client | Live dashboard with historical charts |
| Deployment | Linux systemd | Auto-start on boot, auto-restart on failure |
| Hardware | Raspberry Pi 5, DHT22, HC-SR04 | |


## Hardware

- Raspberry Pi 5
- DHT22 temperature and humidity sensor (with built-in pull-up resistor)
- HC-SR04 ultrasonic distance sensor
- Voltage divider (1kΩ + 2kΩ resistors) on HC-SR04 ECHO pin to step down 5V to 3.3V

### Wiring

| Component | Pin | Pi Physical Pin |
|---|---|---|
| HC-SR04 VCC | 5V | Pin 2 |
| HC-SR04 GND | GND | Pin 6 |
| HC-SR04 TRIG | GPIO23 | Pin 16 |
| HC-SR04 ECHO | GPIO24 (via voltage divider) | Pin 18 |
| DHT22 VCC | 3.3V | Pin 1 |
| DHT22 DATA | GPIO17 | Pin 11 |
| DHT22 GND | GND | Pin 9 |


## Project Structure
smart-env-monitor/
├── firmware/                  ← C++ sensor daemon
│   ├── CMakeLists.txt
│   └── src/
│       ├── main.cpp
│       ├── mqtt_publisher.cpp/h
│       └── sensors/
│           ├── hcsr04.cpp/h
│           └── dht22.cpp/h
├── backend/                   ← Python Flask server
│   ├── app.py
│   ├── database.py
│   ├── mqtt_subscriber.py
│   └── requirements.txt
├── frontend/                  ← React dashboard
│   └── src/
│       ├── components/
│       │   ├── CurrentReadings.js
│       │   ├── SensorChart.js
│       │   └── StatsPage.js
│       ├── App.js
│       └── config.js
└── systemd/                   ← systemd service files
├── sensor-daemon.service
└── sensor-backend.service


## What I Learned

- GPIO programming on Linux - using libgpiod 
- DHT22 single-wire protocol — decoding 40 bits of data using pulse-width timing at microsecond precision
- HC-SR04 ultrasonic ranging — measuring echo pulse duration to calculate distance, and building a voltage divider to protect the Pi's 3.3V GPIO pin from the sensor's 5V output
- MQTT publish/subscribe protocol 
- SQLite time-series database design — schema design, timestamp indexing, and aggregate queries
- REST API design with Flask — building endpoints for recent readings, time-range queries, and daily statistics
- Real-time WebSockets — pushing live sensor data to the browser instantly using Flask-SocketIO
- React with live data — managing WebSocket state, time-series charts with Recharts, multi-page routing
- Linux systemd services — deploying software as production services that survive reboots and auto-restart on failure


## Challenges

- The DHT22 single-wire protocol is extremely timing-sensitive. Linux kernel scheduling interruptions caused inconsistent bit readings. Solved by using the Adafruit CircuitPython DHT library which runs closer to the kernel level
- The HC-SR04 ECHO pin outputs 5V which would damage the Pi's 3.3V GPIO. Built a voltage divider using a 1 kilo ohm and 2 kilo ohm resistor to safely step the voltage down
- Raspberry Pi OS claimed GPIO4 via a one-wire overlay in config.txt, blocking the DHT22. Resolved by removing the overlay and moving to GPIO17

