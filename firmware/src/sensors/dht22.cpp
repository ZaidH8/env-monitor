#include "dht22.h"
#include <chrono>
#include <thread>
#include <iostream>

DHT22::DHT22(gpiod_chip* chip, int data_pin) {
    this->chip = chip;
    this->pin_number = data_pin;
}

DHT22::~DHT22() {}

ClimateReading DHT22::read() {
    ClimateReading result = {0.0f, 0.0f, false};
    uint8_t data[5] = {0, 0, 0, 0, 0};

    // === SEND START SIGNAL ===
    gpiod_line* line = gpiod_chip_get_line(chip, pin_number);
    gpiod_line_request_output(line, "dht22", 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // let it settle
    gpiod_line_set_value(line, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));  // longer low pulse
    gpiod_line_set_value(line, 1);
    std::this_thread::sleep_for(std::chrono::microseconds(40));  // brief high before release

    // Switch to input
    gpiod_line_release(line);
    line = gpiod_chip_get_line(chip, pin_number);
    gpiod_line_request_input(line, "dht22");
    std::this_thread::sleep_for(std::chrono::microseconds(10));

    // === WAIT FOR SENSOR RESPONSE ===
    auto start = std::chrono::high_resolution_clock::now();
    auto timeout = [&]() {
        return std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now() - start).count() > 10000;
    };

    // Wait for LOW
    start = std::chrono::high_resolution_clock::now();
    while (gpiod_line_get_value(line) == 1) {
        if (timeout()) {
            std::cerr << "DHT22: no response (check wiring and pull-up resistor)\n";
            gpiod_line_release(line);
            return result;
        }
    }

    // Wait for HIGH
    start = std::chrono::high_resolution_clock::now();
    while (gpiod_line_get_value(line) == 0) {
        if (timeout()) {
            std::cerr << "DHT22: stuck LOW\n";
            gpiod_line_release(line);
            return result;
        }
    }

    // Wait for HIGH to end
    start = std::chrono::high_resolution_clock::now();
    while (gpiod_line_get_value(line) == 1) {
        if (timeout()) {
            std::cerr << "DHT22: stuck HIGH\n";
            gpiod_line_release(line);
            return result;
        }
    }

    // === READ 40 BITS ===
    for (int i = 0; i < 40; i++) {
        // Wait for LOW to end
        start = std::chrono::high_resolution_clock::now();
        while (gpiod_line_get_value(line) == 0) {
            if (timeout()) {
                std::cerr << "DHT22: timeout reading bit " << i << "\n";
                gpiod_line_release(line);
                return result;
            }
        }

        // Measure HIGH duration
        auto high_start = std::chrono::high_resolution_clock::now();
        start = std::chrono::high_resolution_clock::now();
        while (gpiod_line_get_value(line) == 1) {
            if (timeout()) {
                std::cerr << "DHT22: timeout during HIGH bit " << i << "\n";
                gpiod_line_release(line);
                return result;
            }
        }
        auto high_duration = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now() - high_start).count();

        data[i / 8] <<= 1;
        if (high_duration > 40) {
            data[i / 8] |= 1;
        }
    }

    gpiod_line_release(line);

    // === VERIFY CHECKSUM ===
    uint8_t checksum = data[0] + data[1] + data[2] + data[3];
    if (checksum != data[4]) {
        std::cerr << "DHT22: checksum failed\n";
        return result;
    }

    // === DECODE ===
    result.humidity_percent = ((data[0] << 8) | data[1]) / 10.0f;
    int16_t raw_temp = ((data[2] & 0x7F) << 8) | data[3];
    result.temperature_c = raw_temp / 10.0f;
    if (data[2] & 0x80) result.temperature_c *= -1;

    if (result.humidity_percent < 0 || result.humidity_percent > 100 ||
        result.temperature_c < -40 || result.temperature_c > 80) {
        std::cerr << "DHT22: reading out of range\n";
        return result;
    }

    result.valid = true;
    return result;
}