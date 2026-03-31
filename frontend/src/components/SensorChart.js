import { useEffect, useState } from "react";
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, Legend, ResponsiveContainer } from "recharts";
import { API_URL } from "../config";

const RANGES = [
  { label: "1 Hour",  seconds: 3600 },
  { label: "6 Hours", seconds: 21600 },
  { label: "24 Hours",seconds: 86400 },
];

function formatTime(timestamp) {
  return new Date(timestamp * 1000).toLocaleTimeString([], { hour: "2-digit", minute: "2-digit" });
}

export default function SensorChart() {
  const [data, setData] = useState([]);
  const [range, setRange] = useState(RANGES[0]);
  const [loading, setLoading] = useState(false);

  const fetchData = (selectedRange) => {
    setLoading(true);
    const now = Math.floor(Date.now() / 1000);
    const start = now - selectedRange.seconds;

    fetch(`${API_URL}/api/readings/range?start=${start}&end=${now}`)
      .then((r) => r.json())
      .then((readings) => {
        // Format for Recharts
        const formatted = readings.map((r) => ({
          time: formatTime(r.timestamp),
          temperature: parseFloat(r.temperature.toFixed(1)),
          humidity: parseFloat(r.humidity.toFixed(1)),
        }));
        setData(formatted);
        setLoading(false);
      })
      .catch((err) => {
        console.error("Failed to fetch chart data:", err);
        setLoading(false);
      });
  };

  // Fetch data when range changes
  useEffect(() => {
    fetchData(range);
  }, [range]);

  return (
    <div className="chart-container">
      <div className="chart-header">
        <h2>Historical Data</h2>
        <div className="range-buttons">
          {RANGES.map((r) => (
            <button
              key={r.label}
              className={range.label === r.label ? "active" : ""}
              onClick={() => setRange(r)}
            >
              {r.label}
            </button>
          ))}
        </div>
      </div>

      {loading ? (
        <div className="loading">Loading...</div>
      ) : data.length === 0 ? (
        <div className="empty">No data for this time range yet</div>
      ) : (
        <ResponsiveContainer width="100%" height={300}>
          <LineChart data={data}>
            <CartesianGrid strokeDasharray="3 3" />
            <XAxis dataKey="time" tick={{ fontSize: 11 }} interval="preserveStartEnd" />
            <YAxis yAxisId="temp" domain={["auto", "auto"]} label={{ value: "°C", angle: -90, position: "insideLeft" }} />
            <YAxis yAxisId="humidity" orientation="right" domain={[0, 100]} label={{ value: "%", angle: 90, position: "insideRight" }} />
            <Tooltip />
            <Legend />
            <Line yAxisId="temp" type="monotone" dataKey="temperature" stroke="#ef4444" dot={false} name="Temperature (°C)" />
            <Line yAxisId="humidity" type="monotone" dataKey="humidity" stroke="#3b82f6" dot={false} name="Humidity (%)" />
          </LineChart>
        </ResponsiveContainer>
      )}
    </div>
  );
}
