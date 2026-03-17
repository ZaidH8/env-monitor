#include <iostream>
#include <thread>
#include <chrono>
#include <gpiod.h>
#include "sensors/hcsr04.h"

int main() {
    std::cout << "Smart Environment Monitor starting...\n";

    gpiod_chip* chip = gpiod_chip_open_by_name("gpiochip4");
    if (!chip) {
        std::cerr << "ERROR: Could not open GPIO chip\n";
        return 1;
    }

    HCSR04 ultrasonic(chip, 23, 24);

    std::cout << "Reading distance every 2 seconds...\n\n";

    while (true) {
        DistanceReading dist = ultrasonic.read();
        if (dist.valid) {
            std::cout << "Distance: " << dist.distance_cm << " cm";
            std::cout << (dist.distance_cm < 200.0f ? " [OCCUPIED]" : " [EMPTY]") << "\n";
        } else {
            std::cout << "Distance: ERROR\n";
        }
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    gpiod_chip_close(chip);
    return 0;
}