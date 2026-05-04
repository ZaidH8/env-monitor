#pragma once
#include <gpiod.h>

// Holds the result of one distance measurement
struct DistanceReading {
    float distance_cm;
    bool valid;  
};

class HCSR04 {
public:
    HCSR04(gpiod_chip* chip, int trig_pin, int echo_pin);
    ~HCSR04();
    DistanceReading read();

private:
    gpiod_line* trig_line;
    gpiod_line* echo_line;
};
