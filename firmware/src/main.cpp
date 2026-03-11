#include <iostream>
#include <thread>
#include <chrono>
#include <gpiod.h>
#include "sensors/hcsr04.h"
#include "sensors/dht22.h"

int main() {
    std::cout << "Starting sensor test...\n";

    // Open the GPIO chip
    // Change "gpiochip4" to "gpiochip0" if you have a Pi 4
    gpiod_chip* chip = gpiod_chip_open_by_name("gpiochip0");
    if (!chip) {
        std::cerr << "ERROR: Could not open GPIO chip. Are you running as root?\n";
        return 1;
    }

    // Create sensor objects
    // HCSR04: TRIG on GPIO23, ECHO on GPIO24
    // DHT22: DATA on GPIO4
    HCSR04 ultrasonic(chip, 23, 24);
    DHT22 climate(chip, 4);

    std::cout << "Sensors initialized. Reading every 3 seconds...\n\n";

    while (true) {
        // Read distance
        DistanceReading dist = ultrasonic.read();
        if (dist.valid) {
            std::cout << "Distance: " << dist.distance_cm << " cm";
            if (dist.distance_cm < 200.0f) {
                std::cout << " [OCCUPIED]";
            } else {
                std::cout << " [EMPTY]";
            }
            std::cout << "\n";
        } else {
            std::cout << "Distance: ERROR\n";
        }

        // DHT22 needs at least 2 seconds between reads
        std::this_thread::sleep_for(std::chrono::seconds(2));

        // Read temperature and humidity
        ClimateReading climate_data = climate.read();
        if (climate_data.valid) {
            std::cout << "Temperature: " << climate_data.temperature_c << " C\n";
            std::cout << "Humidity:    " << climate_data.humidity_percent << " %\n";
        } else {
            std::cout << "Climate: ERROR\n";
        }

        std::cout << "---\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    gpiod_chip_close(chip);
    return 0;
}
