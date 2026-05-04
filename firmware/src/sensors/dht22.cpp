#include "dht22.h"
#include <iostream>
#include <sstream>
#include <array>
#include <memory>
#include <stdexcept>
#include <string>

DHT22::DHT22(gpiod_chip* chip, int data_pin) {
    this->chip = chip;
    this->pin_number = data_pin;
}

DHT22::~DHT22() {}

ClimateReading DHT22::read() {
    ClimateReading result = {0.0f, 0.0f, false};

    // Call the Python script and capture output
    std::array<char, 128> buffer;
    std::string output;

    FILE* pipe = popen("/home/zaid/env-monitor/backend/venv/bin/python3 /home/zaid/env-monitor/firmware/read_dht22.py", "r");
    if (!pipe) {
        std::cerr << "DHT22: failed to run Python script\n";
        return result;
    }

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        output += buffer.data();
    }
    pclose(pipe);

    if (!output.empty() && output.back() == '\n') {
        output.pop_back();
    }

    if (output == "ERROR" || output.empty()) {
        std::cerr << "DHT22: Python script returned error\n";
        return result;
    }

    std::stringstream ss(output);
    std::string temp_str, humidity_str;

    if (!std::getline(ss, temp_str, ',') || !std::getline(ss, humidity_str)) {
        std::cerr << "DHT22: failed to parse output: " << output << "\n";
        return result;
    }

    try {
        result.temperature_c = std::stof(temp_str);
        result.humidity_percent = std::stof(humidity_str);
        result.valid = true;
    } catch (...) {
        std::cerr << "DHT22: failed to convert values\n";
        return result;
    }

    return result;
}