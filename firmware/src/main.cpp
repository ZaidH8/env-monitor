#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <gpiod.h>
#include <nlohmann/json.hpp>
#include "sensors/hcsr04.h"
#include "sensors/dht22.h"
#include "mqtt_publisher.h"

using json = nlohmann::json;

//Room is considered "occupied" if something is within 200cm
const float OCCUPANCY_THRESHOLD_CM = 200.0f;

// How often to publish readings
const int PUBLISH_INTERVAL_SEC = 2;

int main() {
    std::cout << "Smart Environment Monitor starting...\n";

    gpiod_chip* chip = gpiod_chip_open_by_name("gpiochip4");
    if (!chip) {
        std::cerr << "ERROR: Could not open GPIO chip\n";
        return 1;
    }

    HCSR04 ultrasonic(chip, 23, 24);
    DHT22 climate(chip, 4);

    MQTTPublisher mqtt("tcp://localhost:1883", "sensor_daemon");

    while (!mqtt.connect()) {
        std::cerr << "MQTT: retrying in 5 seconds...\n";
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    std::cout << "Publishing to sensors/room1/data every "
              << PUBLISH_INTERVAL_SEC << " seconds\n";

    while (true) {
        DistanceReading dist = ultrasonic.read();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        ClimateReading climate_data = climate.read();

        //Only publish if both readings are valid
        if (dist.valid && climate_data.valid) {
            long timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();

            json payload;
            payload["timestamp"] = timestamp;
            payload["temperature"] = climate_data.temperature_c;
            payload["humidity"] = climate_data.humidity_percent;
            payload["distance_cm"] = dist.distance_cm;
            payload["occupied"] = (dist.distance_cm < OCCUPANCY_THRESHOLD_CM);

            std::string json_str = payload.dump();

            //Reconnect if connection was lost
            if (!mqtt.is_connected()) {
                std::cerr << "MQTT: connection lost, reconnecting...\n";
                mqtt.connect();
            }

            if (mqtt.publish("sensors/room1/data", json_str)) {
                std::cout << "Published: " << json_str << "\n";
            }
        } else {
            std::cerr << "Skipping publish — invalid sensor reading\n";
        }

        std::this_thread::sleep_for(std::chrono::seconds(PUBLISH_INTERVAL_SEC));
    }

    gpiod_chip_close(chip);
    return 0;
}
