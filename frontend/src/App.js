import CurrentReadings from "./components/CurrentReadings";
import SensorChart from "./components/SensorChart";
import "./App.css";

export default function App() {
  return (
    <div className="app">
      <header className="app-header">
        <h1>Smart Environment Monitor</h1>
        <p>Raspberry Pi 5 · Room 1</p>
      </header>

      <main className="app-main">
        <CurrentReadings />
        <SensorChart />
      </main>
    </div>
  );
}
