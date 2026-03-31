import { useEffect, useState } from "react";
import { io } from "socket.io-client";
import { SOCKET_URL, API_URL } from "../config";

export default function CurrentReadings() {
  const [reading, setReading] = useState(null);
  const [connected, setConnected] = useState(false);

  useEffect(() => {
    // Fetch the most recent reading when the page loads
    fetch(`${API_URL}/api/readings/recent?n=1`)
      .then((r) => r.json())
      .then((data) => {
        if (data.length > 0) setReading(data[0]);
      })
      .catch((err) => console.error("Failed to fetch reading:", err));

    // Connect to WebSocket for live updates
    const socket = io(SOCKET_URL);

    socket.on("connect", () => setConnected(true));
    socket.on("disconnect", () => setConnected(false));

    // Every time a new reading arrives, update the display
    socket.on("new_reading", (data) => {
      setReading(data);
    });

    // Cleanup: disconnect WebSocket when component unmounts
    return () => socket.disconnect();
  }, []);

  if (!reading) {
    return <div className="card">Waiting for sensor data...</div>;
  }

  return (
    <div className="current-readings">
      <div className="status-bar">
        <span className={connected ? "dot green" : "dot grey"}></span>
        {connected ? "Live" : "Disconnected"}
      </div>

      <div className="readings-grid">
        <div className="reading-card">
          <div className="reading-label">Temperature</div>
          <div className="reading-value">{reading.temperature.toFixed(1)}°C</div>
        </div>

        <div className="reading-card">
          <div className="reading-label">Humidity</div>
          <div className="reading-value">{reading.humidity_percent !== undefined
            ? reading.humidity_percent.toFixed(1)
            : reading.humidity?.toFixed(1)}%</div>
        </div>

        <div className="reading-card">
          <div className="reading-label">Distance</div>
          <div className="reading-value">{reading.distance_cm.toFixed(0)} cm</div>
        </div>

        <div className={`reading-card occupancy ${reading.occupied ? "occupied" : "empty"}`}>
          <div className="reading-label">Room Status</div>
          <div className="reading-value">{reading.occupied ? "🟢 Occupied" : "⚪ Empty"}</div>
        </div>
      </div>
    </div>
  );
}
