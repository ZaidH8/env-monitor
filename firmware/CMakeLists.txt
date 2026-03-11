cmake_minimum_required(VERSION 3.10)
project(sensor_daemon)

# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# List all source files
add_executable(sensor_daemon
    src/main.cpp
    src/sensors/hcsr04.cpp
    src/sensors/dht22.cpp
)

# Link against the GPIO library
target_link_libraries(sensor_daemon gpiod)

# Include the src directory so #include "sensors/hcsr04.h" works
target_include_directories(sensor_daemon PRIVATE src)
