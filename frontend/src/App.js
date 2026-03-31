import { BrowserRouter, Routes, Route, NavLink } from "react-router-dom";
import CurrentReadings from "./components/CurrentReadings";
import SensorChart from "./components/SensorChart";
import StatsPage from "./components/StatsPage";
import "./App.css";

export default function App() {
  return (
    <BrowserRouter>
      <div className="app">
        <header className="app-header">
          <h1>Smart Environment Monitor</h1>
          <p>Raspberry Pi 5 · Room 1</p>
          <nav className="nav">
            <NavLink to="/" end className={({ isActive }) => isActive ? "nav-link active" : "nav-link"}>Live View</NavLink>
            <NavLink to="/stats" className={({ isActive }) => isActive ? "nav-link active" : "nav-link"}>Statistics</NavLink>
          </nav>
        </header>

        <main className="app-main">
          <Routes>
            <Route path="/" element={
              <>
                <CurrentReadings />
                <SensorChart />
              </>
            } />
            <Route path="/stats" element={<StatsPage />} />
          </Routes>
        </main>
      </div>
    </BrowserRouter>
  );
}
