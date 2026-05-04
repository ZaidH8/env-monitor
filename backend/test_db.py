import time
import logging
import database
import mqtt_subscriber

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s %(levelname)s %(name)s: %(message)s"
)

print("Initializing database...")
database.init_db()

print("Starting MQTT subscriber...")
client = mqtt_subscriber.start_subscriber()

print("Listening for sensor data. Press Ctrl+C to stop.")
print("Make sure your C++ sensor daemon is running in another terminal.")
print()

try:
    while True:
        time.sleep(5)
        recent = database.get_recent(5)
        if recent:
            print(f"\nLast {len(recent)} readings in database:")
            for r in recent:
                print(f"  temp={r['temperature']}C  humidity={r['humidity']}%  distance={r['distance_cm']}cm  occupied={bool(r['occupied'])}")
        else:
            print("No readings in database yet...")
except KeyboardInterrupt:
    print("\nStopping...")
    client.loop_stop()
