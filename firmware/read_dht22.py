#!/home/zaid/env-monitor/backend/venv/bin/python3
import sys
import time
import board
import adafruit_dht

dht = adafruit_dht.DHT22(board.D17, use_pulseio=False)

# Try up to 5 times
for attempt in range(5):
    try:
        temperature = dht.temperature
        humidity = dht.humidity
        if temperature is not None and humidity is not None:
            print(f"{temperature},{humidity}")
            dht.exit()
            sys.exit(0)
    except RuntimeError:
        pass
    time.sleep(2)

dht.exit()
print("ERROR")
sys.exit(1)