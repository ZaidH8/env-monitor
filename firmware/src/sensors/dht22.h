#pragma once
#include <gpiod.h>

struct ClimateReading {
    float temperature_c;
    float humidity_percent;
    bool valid;
};

class DHT22 {
public:
    DHT22(gpiod_chip* chip, int data_pin);
    ~DHT22();
    ClimateReading read();

private:
    gpiod_chip* chip;
    int pin_number;
};