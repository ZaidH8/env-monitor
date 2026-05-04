import json
import logging
import paho.mqtt.client as mqtt
import database

logger = logging.getLogger(__name__)

BROKER_ADDRESS = "localhost"
BROKER_PORT = 1883
TOPIC = "sensors/room1/data"

socketio_instance = None

def set_socketio(socketio):
    global socketio_instance
    socketio_instance = socketio

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        logger.info("MQTT subscriber connected to broker")
        client.subscribe(TOPIC)
        logger.info("Subscribed to %s", TOPIC)
    else:
        logger.error("MQTT connection failed with code %d", rc)

def on_message(client, userdata, msg):
    try:
        # Decode the JSON payload
        data = json.loads(msg.payload.decode("utf-8"))
        logger.debug("Received: %s", data)

        # Store in database
        success = database.insert_reading(data)
        if success:
            logger.info("Stored reading: temp=%.1f humidity=%.1f distance=%.1f",
                       data["temperature"], data["humidity"], data["distance_cm"])

            # Push to web dashboard via WebSocket (set up in Week 4)
            if socketio_instance:
                socketio_instance.emit("new_reading", data)
        else:
            logger.warning("Reading rejected by validation")

    except json.JSONDecodeError:
        logger.error("Failed to decode JSON: %s", msg.payload)
    except Exception as e:
        logger.error("Error processing message: %s", e)

def on_disconnect(client, userdata, rc):
    if rc != 0:
        logger.warning("MQTT unexpected disconnect, will auto-reconnect")

def start_subscriber():
    """Start the MQTT subscriber. This runs in its own thread."""
    database.init_db()

    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message
    client.on_disconnect = on_disconnect

    # Auto-reconnect settings
    client.reconnect_delay_set(min_delay=1, max_delay=30)

    client.connect(BROKER_ADDRESS, BROKER_PORT)

    client.loop_start()
    logger.info("MQTT subscriber started in background thread")
    return client
