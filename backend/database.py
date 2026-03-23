import sqlite3
import logging
from datetime import datetime

logger = logging.getLogger(__name__)

DB_PATH = "/home/zaid/env-monitor/backend/sensor_data.db"

def get_connection():
    """Get a database connection. Creates the database file if it doesn't exist."""
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row  # Rows behave like dictionaries
    return conn

def init_db():
    """Create the database tables if they don't already exist."""
    conn = get_connection()
    cursor = conn.cursor()

    cursor.execute("""
        CREATE TABLE IF NOT EXISTS readings (
            id          INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp   INTEGER NOT NULL,
            temperature REAL NOT NULL,
            humidity    REAL NOT NULL,
            distance_cm REAL NOT NULL,
            occupied    INTEGER NOT NULL
        )
    """)

    # Index on timestamp makes time-range queries fast
    cursor.execute("""
        CREATE INDEX IF NOT EXISTS idx_timestamp ON readings(timestamp)
    """)

    conn.commit()
    conn.close()
    logger.info("Database initialized at %s", DB_PATH)

def insert_reading(data: dict) -> bool:
    """
    Insert one sensor reading into the database.
    Returns True on success, False on failure.
    """
    try:
        # Validate required fields exist
        required = ["timestamp", "temperature", "humidity", "distance_cm", "occupied"]
        for field in required:
            if field not in data:
                logger.error("Missing field in reading: %s", field)
                return False

        # Validate ranges
        if not (-40 <= data["temperature"] <= 80):
            logger.error("Temperature out of range: %s", data["temperature"])
            return False
        if not (0 <= data["humidity"] <= 100):
            logger.error("Humidity out of range: %s", data["humidity"])
            return False
        if not (0 <= data["distance_cm"] <= 400):
            logger.error("Distance out of range: %s", data["distance_cm"])
            return False

        conn = get_connection()
        conn.execute("""
            INSERT INTO readings (timestamp, temperature, humidity, distance_cm, occupied)
            VALUES (?, ?, ?, ?, ?)
        """, (
            data["timestamp"],
            data["temperature"],
            data["humidity"],
            data["distance_cm"],
            1 if data["occupied"] else 0
        ))
        conn.commit()
        conn.close()
        return True

    except Exception as e:
        logger.error("Failed to insert reading: %s", e)
        return False

def get_recent(n: int = 100) -> list:
    """Get the most recent n readings, newest first."""
    conn = get_connection()
    cursor = conn.execute("""
        SELECT * FROM readings
        ORDER BY timestamp DESC
        LIMIT ?
    """, (n,))
    rows = [dict(row) for row in cursor.fetchall()]
    conn.close()
    return rows

def get_range(start_timestamp: int, end_timestamp: int) -> list:
    """Get all readings between two Unix timestamps."""
    conn = get_connection()
    cursor = conn.execute("""
        SELECT * FROM readings
        WHERE timestamp >= ? AND timestamp <= ?
        ORDER BY timestamp ASC
    """, (start_timestamp, end_timestamp))
    rows = [dict(row) for row in cursor.fetchall()]
    conn.close()
    return rows

def get_stats_today() -> dict:
    """Get min, max, and average temperature and humidity for today."""
    import time
    # Start of today in Unix time
    today_start = int(datetime.now().replace(hour=0, minute=0, second=0, microsecond=0).timestamp())

    conn = get_connection()
    cursor = conn.execute("""
        SELECT
            MIN(temperature) as temp_min,
            MAX(temperature) as temp_max,
            AVG(temperature) as temp_avg,
            MIN(humidity)    as humidity_min,
            MAX(humidity)    as humidity_max,
            AVG(humidity)    as humidity_avg,
            SUM(occupied)    as occupied_count,
            COUNT(*)         as total_count
        FROM readings
        WHERE timestamp >= ?
    """, (today_start,))
    row = cursor.fetchone()
    conn.close()

    if row and row["total_count"] > 0:
        return {
            "temp_min": round(row["temp_min"], 1),
            "temp_max": round(row["temp_max"], 1),
            "temp_avg": round(row["temp_avg"], 1),
            "humidity_min": round(row["humidity_min"], 1),
            "humidity_max": round(row["humidity_max"], 1),
            "humidity_avg": round(row["humidity_avg"], 1),
            "occupied_percent": round((row["occupied_count"] / row["total_count"]) * 100, 1),
            "total_readings": row["total_count"]
        }
    return {}
