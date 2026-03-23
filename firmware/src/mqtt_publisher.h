#pragma once
#include <string>
#include <mqtt/client.h>

class MQTTPublisher {
public:
    MQTTPublisher(const std::string& broker_address, const std::string& client_id);
    ~MQTTPublisher();

    bool connect();
    bool publish(const std::string& topic, const std::string& payload);
    bool is_connected();

private:
    mqtt::client client;
    std::string broker_address;
};
