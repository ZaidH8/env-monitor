cmake_minimum_required(VERSION 3.10)
project(sensor_daemon)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(sensor_daemon
    src/main.cpp
    src/sensors/hcsr04.cpp
    src/sensors/dht22.cpp
    src/mqtt_publisher.cpp
)

# Link GPIO and MQTT libraries
target_link_libraries(sensor_daemon gpiod paho-mqtt3c paho-mqtt3as paho-mqttpp3)

target_include_directories(sensor_daemon PRIVATE src)
