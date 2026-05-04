import eventlet
eventlet.monkey_patch()
import logging
import threading
from flask import Flask, jsonify, request
from flask_socketio import SocketIO
from flask_cors import CORS
import database
import mqtt_subscriber
import os
from flask import send_from_directory


#Set up logging
logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s %(levelname)s %(name)s: %(message)s"
)
logger = logging.getLogger(__name__)

app = Flask(__name__, static_folder=None)
CORS(app)  
socketio = SocketIO(app, cors_allowed_origins="*")


mqtt_subscriber.set_socketio(socketio)


@app.route("/api/status")
def status():
    """Health check endpoint. Returns basic server status."""
    return jsonify({"status": "online", "message": "Sensor backend running"})

@app.route("/api/readings/recent")
def recent_readings():
    """
    Get the most recent sensor readings.
    Query parameter: n (default 100)
    Example: GET /api/readings/recent?n=50
    """
    n = request.args.get("n", 100, type=int)
    n = min(n, 1000)  # Cap at 1000 to prevent huge responses
    readings = database.get_recent(n)
    return jsonify(readings)

@app.route("/api/readings/range")
def range_readings():
    """
    Get readings between two Unix timestamps.
    Query parameters: start, end
    Example: GET /api/readings/range?start=1718000000&end=1718003600
    """
    start = request.args.get("start", type=int)
    end = request.args.get("end", type=int)

    if start is None or end is None:
        return jsonify({"error": "start and end parameters required"}), 400
    if start >= end:
        return jsonify({"error": "start must be before end"}), 400

    readings = database.get_range(start, end)
    return jsonify(readings)

@app.route("/api/stats/today")
def stats_today():
    """Get min, max, average temperature and humidity for today."""
    stats = database.get_stats_today()
    if not stats:
        return jsonify({"error": "No data for today yet"}), 404
    return jsonify(stats)


@socketio.on("connect")
def handle_connect():
    logger.info("Browser client connected via WebSocket")
    recent = database.get_recent(1)
    if recent:
        socketio.emit("new_reading", recent[0])

@socketio.on("disconnect")
def handle_disconnect():
    logger.info("Browser client disconnected")



@app.route("/", defaults={"path": ""})
@app.route("/<path:path>")
def serve_react(path):
    build_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "../frontend/build"))
    full_path = os.path.join(build_dir, path)
    if path and os.path.exists(full_path):
        return send_from_directory(build_dir, path)
    return send_from_directory(build_dir, "index.html")


def start():
    #Initialize database
    database.init_db()

    #Start MQTT subscriber in background thread
    mqtt_subscriber.start_subscriber()
    logger.info("MQTT subscriber started")

    #Start Flask with WebSocket support
    logger.info("Starting web server on port 5000")
    socketio.run(app, host="0.0.0.0", port=5000, debug=False)

if __name__ == "__main__":
    start()
