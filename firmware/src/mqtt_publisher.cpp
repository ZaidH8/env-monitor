#include "mqtt_publisher.h"
#include <iostream>
#include <chrono>
#include <thread>

MQTTPublisher::MQTTPublisher(const std::string& broker_address, const std::string& client_id)
    : client(broker_address, client_id), broker_address(broker_address) {}

MQTTPublisher::~MQTTPublisher() {
    if (client.is_connected()) {
        client.disconnect();
    }
}

bool MQTTPublisher::connect() {
    try {
        mqtt::connect_options options;
        options.set_keep_alive_interval(20);
        options.set_clean_session(true);

        // Set a Last Will message — this is published automatically if the
        // client disconnects unexpectedly. Subscribers can detect failures.
        mqtt::will_options will("sensors/room1/status", std::string("offline"), 1, true);
        options.set_will(will);

        client.connect(options);

        // Publish online status
        auto msg = mqtt::make_message("sensors/room1/status", "online");
        msg->set_retained(true);
        msg->set_qos(1);
        client.publish(msg);

        std::cout << "MQTT: connected to " << broker_address << "\n";
        return true;
    } catch (const mqtt::exception& e) {
        std::cerr << "MQTT connect failed: " << e.what() << "\n";
        return false;
    }
}

bool MQTTPublisher::publish(const std::string& topic, const std::string& payload) {
    try {
        auto msg = mqtt::make_message(topic, payload);
        msg->set_qos(1);  // QoS 1 = at least once delivery
        client.publish(msg);
        return true;
    } catch (const mqtt::exception& e) {
        std::cerr << "MQTT publish failed: " << e.what() << "\n";
        return false;
    }
}

bool MQTTPublisher::is_connected() {
    return client.is_connected();
}
