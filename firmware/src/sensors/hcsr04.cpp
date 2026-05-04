#include "hcsr04.h"
#include <chrono>
#include <thread>
#include <iostream>


const int TIMEOUT_US = 30000;

HCSR04::HCSR04(gpiod_chip* chip, int trig_pin, int echo_pin) {
    trig_line = gpiod_chip_get_line(chip, trig_pin);
    gpiod_line_request_output(trig_line, "hcsr04_trig", 0);

    echo_line = gpiod_chip_get_line(chip, echo_pin);
    gpiod_line_request_input(echo_line, "hcsr04_echo");
}

HCSR04::~HCSR04() {
    gpiod_line_release(trig_line);
    gpiod_line_release(echo_line);
}

DistanceReading HCSR04::read() {
    DistanceReading result = {0.0f, false};

    gpiod_line_set_value(trig_line, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));

    gpiod_line_set_value(trig_line, 1);
    std::this_thread::sleep_for(std::chrono::microseconds(10));
    gpiod_line_set_value(trig_line, 0);

    auto wait_start = std::chrono::high_resolution_clock::now();
    while (gpiod_line_get_value(echo_line) == 0) {
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - wait_start).count();
        if (elapsed > TIMEOUT_US) {
            std::cerr << "HC-SR04: timeout waiting for ECHO HIGH\n";
            return result;
        }
    }

    auto echo_start = std::chrono::high_resolution_clock::now();


    while (gpiod_line_get_value(echo_line) == 1) {
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - echo_start).count();
        if (elapsed > TIMEOUT_US) {
            std::cerr << "HC-SR04: timeout — nothing in range\n";
            return result;
        }
    }

    auto echo_end = std::chrono::high_resolution_clock::now();
    long duration_us = std::chrono::duration_cast<std::chrono::microseconds>(echo_end - echo_start).count();


    float distance = duration_us / 58.0f;

    if (distance < 2.0f || distance > 400.0f) {
        std::cerr << "HC-SR04: reading out of valid range (" << distance << " cm)\n";
        return result;
    }

    result.distance_cm = distance;
    result.valid = true;
    return result;
}
