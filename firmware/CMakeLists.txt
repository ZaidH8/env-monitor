cmake_minimum_required(VERSION 3.10)
project(sensor_daemon)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(sensor_daemon
    src/main.cpp
    src/sensors/hcsr04.cpp
    src/sensors/dht22.cpp
)

target_link_libraries(sensor_daemon gpiod)

target_include_directories(sensor_daemon PRIVATE src)