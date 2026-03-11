#include "hcsr04.h"
#include <chrono>
#include <thread>
#include <iostream>

// Maximum time to wait for ECHO response before giving up (microseconds)
// At max sensor range of 4m, echo takes ~23ms. 30ms is a safe timeout.
const int TIMEOUT_US = 30000;

HCSR04::HCSR04(gpiod_chip* chip, int trig_pin, int echo_pin) {
    // Set up TRIG pin as output, starting LOW
    trig_line = gpiod_chip_get_line(chip, trig_pin);
    gpiod_line_request_output(trig_line, "hcsr04_trig", 0);

    // Set up ECHO pin as input
    echo_line = gpiod_chip_get_line(chip, echo_pin);
    gpiod_line_request_input(echo_line, "hcsr04_echo");
}

HCSR04::~HCSR04() {
    gpiod_line_release(trig_line);
    gpiod_line_release(echo_line);
}

DistanceReading HCSR04::read() {
    DistanceReading result = {0.0f, false};

    // Step 1: Make sure TRIG is LOW and let it settle
    gpiod_line_set_value(trig_line, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));

    // Step 2: Send a 10 microsecond HIGH pulse on TRIG
    // This tells the sensor to fire an ultrasonic pulse
    gpiod_line_set_value(trig_line, 1);
    std::this_thread::sleep_for(std::chrono::microseconds(10));
    gpiod_line_set_value(trig_line, 0);

    // Step 3: Wait for ECHO to go HIGH
    // The sensor pulls ECHO HIGH when it starts listening for the echo
    auto wait_start = std::chrono::high_resolution_clock::now();
    while (gpiod_line_get_value(echo_line) == 0) {
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - wait_start).count();
        if (elapsed > TIMEOUT_US) {
            std::cerr << "HC-SR04: timeout waiting for ECHO HIGH\n";
            return result;
        }
    }

    // Step 4: ECHO just went HIGH — start the timer
    auto echo_start = std::chrono::high_resolution_clock::now();

    // Step 5: Wait for ECHO to go LOW
    // ECHO goes LOW when the reflected pulse returns to the sensor
    while (gpiod_line_get_value(echo_line) == 1) {
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - echo_start).count();
        if (elapsed > TIMEOUT_US) {
            std::cerr << "HC-SR04: timeout — nothing in range\n";
            return result;
        }
    }

    // Step 6: Calculate how long ECHO was HIGH
    auto echo_end = std::chrono::high_resolution_clock::now();
    long duration_us = std::chrono::duration_cast<std::chrono::microseconds>(echo_end - echo_start).count();

    // Step 7: Convert duration to distance
    // Sound travels at ~343 m/s. Pulse goes out AND back, so divide by 2.
    // The constant 58 handles all the unit conversion: cm = microseconds / 58
    float distance = duration_us / 58.0f;

    // Step 8: Sanity check — HC-SR04 only works between 2cm and 400cm
    if (distance < 2.0f || distance > 400.0f) {
        std::cerr << "HC-SR04: reading out of valid range (" << distance << " cm)\n";
        return result;
    }

    result.distance_cm = distance;
    result.valid = true;
    return result;
}
