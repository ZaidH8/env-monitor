import { useEffect, useState } from "react";
import { API_URL } from "../config";

export default function StatsPage() {
  const [stats, setStats] = useState(null);
  const [error, setError] = useState(null);

  useEffect(() => {
    fetch(`${API_URL}/api/stats/today`)
      .then((r) => {
        if (!r.ok) throw new Error("No data yet");
        return r.json();
      })
      .then(setStats)
      .catch((e) => setError(e.message));
  }, []);

  if (error) return <div className="empty">No statistics available for today yet. Check back after a few hours.</div>;
  if (!stats)  return <div className="loading">Loading stats...</div>;

  return (
    <div className="stats-page">
      <h2>Today's Statistics</h2>
      <div className="readings-grid" style={{ marginTop: 24 }}>
        <div className="reading-card">
          <div className="reading-label">Temp Min / Max</div>
          <div className="reading-value" style={{ fontSize: 22 }}>{stats.temp_min}°C / {stats.temp_max}°C</div>
          <div className="reading-label" style={{ marginTop: 8 }}>Avg: {stats.temp_avg}°C</div>
        </div>
        <div className="reading-card">
          <div className="reading-label">Humidity Min / Max</div>
          <div className="reading-value" style={{ fontSize: 22 }}>{stats.humidity_min}% / {stats.humidity_max}%</div>
          <div className="reading-label" style={{ marginTop: 8 }}>Avg: {stats.humidity_avg}%</div>
        </div>
        <div className="reading-card">
          <div className="reading-label">Occupancy</div>
          <div className="reading-value" style={{ fontSize: 22 }}>{stats.occupied_percent}%</div>
          <div className="reading-label" style={{ marginTop: 8 }}>of the day occupied</div>
        </div>
        <div className="reading-card">
          <div className="reading-label">Total Readings</div>
          <div className="reading-value" style={{ fontSize: 22 }}>{stats.total_readings}</div>
          <div className="reading-label" style={{ marginTop: 8 }}>data points stored</div>
        </div>
      </div>
    </div>
  );
}
