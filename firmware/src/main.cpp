#include <iostream>
#include <thread>
#include <chrono>
#include <gpiod.h>
#include "sensors/hcsr04.h"
#include "sensors/dht22.h"

int main() {
    std::cout << "Starting sensor test...\n";

    gpiod_chip* chip = gpiod_chip_open_by_name("gpiochip4");
    if (!chip) {
        std::cerr << "ERROR: Could not open GPIO chip\n";
        return 1;
    }

    HCSR04 ultrasonic(chip, 23, 24);
    DHT22 climate(chip, 17);

    std::cout << "Sensors initialized. Reading every 3 seconds...\n\n";

    while (true) {
        DistanceReading dist = ultrasonic.read();
        if (dist.valid) {
            std::cout << "Distance: " << dist.distance_cm << " cm";
            std::cout << (dist.distance_cm < 200.0f ? " [OCCUPIED]" : " [EMPTY]") << "\n";
        } else {
            std::cout << "Distance: ERROR\n";
        }

        std::this_thread::sleep_for(std::chrono::seconds(3));

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