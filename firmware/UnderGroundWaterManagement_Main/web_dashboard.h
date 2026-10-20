#pragma once
#include <Arduino.h>

// ============================================================================
// 💧 Underground Water Management - High-Performance Web Dashboard
// InnovateX: Techfest, IIT Bombay
// Stored in Flash (PROGMEM) to conserve RAM
// ============================================================================

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Underground Water Management | IoT Monitor</title>
  <link rel="preconnect" href="https://fonts.googleapis.com">
  <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
  <link href="https://fonts.googleapis.com/css2?family=Outfit:wght@300;400;600;700;800&family=JetBrains+Mono:wght@400;600&display=swap" rel="stylesheet">
  <style>
    :root {
      --bg: #070d19;
      --card-bg: rgba(15, 23, 42, 0.75);
      --card-border: rgba(56, 189, 248, 0.15);
      --primary: #0284c7;
      --primary-light: #38bdf8;
      --accent: #06b6d4;
      --success: #10b981;
      --warning: #f59e0b;
      --danger: #ef4444;
      --text-main: #f8fafc;
      --text-muted: #94a3b8;
    }

    * { box-sizing: border-box; margin: 0; padding: 0; }

    body {
      font-family: 'Outfit', sans-serif;
      background: radial-gradient(circle at 10% 20%, #0c1b33 0%, var(--bg) 90%);
      color: var(--text-main);
      min-height: 100vh;
      padding: 24px;
      line-height: 1.5;
    }

    .container {
      max-width: 1200px;
      margin: 0 auto;
    }

    header {
      display: flex;
      justify-content: space-between;
      align-items: center;
      flex-wrap: wrap;
      gap: 16px;
      padding-bottom: 24px;
      border-bottom: 1px solid rgba(255, 255, 255, 0.08);
      margin-bottom: 24px;
    }

    .brand {
      display: flex;
      align-items: center;
      gap: 14px;
    }

    .brand-logo {
      width: 48px;
      height: 48px;
      border-radius: 14px;
      background: linear-gradient(135deg, #0284c7, #06b6d4);
      display: flex;
      align-items: center;
      justify-content: center;
      font-size: 26px;
      box-shadow: 0 0 20px rgba(6, 182, 212, 0.4);
    }

    h1 {
      font-size: 26px;
      font-weight: 800;
      letter-spacing: -0.5px;
      background: linear-gradient(90deg, #38bdf8, #818cf8);
      -webkit-background-clip: text;
      -webkit-text-fill-color: transparent;
    }

    .badge-sub {
      font-size: 13px;
      color: var(--text-muted);
      font-weight: 400;
    }

    .techfest-pill {
      background: rgba(56, 189, 248, 0.1);
      border: 1px solid rgba(56, 189, 248, 0.25);
      padding: 6px 14px;
      border-radius: 9999px;
      font-size: 12px;
      font-weight: 600;
      color: var(--primary-light);
      display: flex;
      align-items: center;
      gap: 8px;
    }

    .pulse {
      width: 8px;
      height: 8px;
      border-radius: 50%;
      background: var(--success);
      box-shadow: 0 0 10px var(--success);
      animation: pulseAnim 2s infinite;
    }

    @keyframes pulseAnim {
      0%, 100% { opacity: 1; transform: scale(1); }
      50% { opacity: 0.4; transform: scale(1.2); }
    }

    /* GRID LAYOUT */
    .dashboard-grid {
      display: grid;
      grid-template-columns: 340px 1fr;
      gap: 24px;
      margin-bottom: 24px;
    }

    @media (max-width: 900px) {
      .dashboard-grid { grid-template-columns: 1fr; }
    }

    .card {
      background: var(--card-bg);
      backdrop-filter: blur(16px);
      -webkit-backdrop-filter: blur(16px);
      border: 1px solid var(--card-border);
      border-radius: 20px;
      padding: 24px;
      box-shadow: 0 8px 32px rgba(0, 0, 0, 0.35);
      transition: transform 0.2s ease, border-color 0.2s ease;
    }

    .card:hover {
      border-color: rgba(56, 189, 248, 0.35);
    }

    /* RESERVOIR VISUALIZER */
    .reservoir-box {
      display: flex;
      flex-direction: column;
      align-items: center;
    }

    .reservoir-title {
      font-size: 16px;
      font-weight: 700;
      margin-bottom: 16px;
      color: var(--primary-light);
      align-self: flex-start;
      display: flex;
      align-items: center;
      gap: 8px;
    }

    .tank-graphic {
      width: 100%;
      height: 320px;
      background: rgba(15, 23, 42, 0.9);
      border: 3px solid #334155;
      border-radius: 16px;
      position: relative;
      overflow: hidden;
      margin-bottom: 16px;
      display: flex;
      flex-direction: column-reverse;
    }

    .water-fill {
      width: 100%;
      background: linear-gradient(180deg, #38bdf8 0%, #0369a1 100%);
      transition: height 0.6s cubic-bezier(0.4, 0, 0.2, 1);
      position: relative;
      box-shadow: 0 0 30px rgba(56, 189, 248, 0.3) inset;
    }

    .water-surface-line {
      position: absolute;
      top: 0;
      left: 0;
      right: 0;
      height: 4px;
      background: #bae6fd;
      box-shadow: 0 0 10px #38bdf8;
    }

    .pump-intake-marker {
      position: absolute;
      left: 0;
      right: 0;
      border-top: 2px dashed #f87171;
      z-index: 10;
      pointer-events: none;
    }

    .marker-label {
      position: absolute;
      right: 8px;
      bottom: 2px;
      font-size: 11px;
      font-weight: 600;
      color: #f87171;
      background: rgba(0, 0, 0, 0.6);
      padding: 1px 6px;
      border-radius: 4px;
    }

    /* METRICS TILES */
    .metrics-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
      gap: 16px;
    }

    .metric-tile {
      background: rgba(30, 41, 59, 0.45);
      border: 1px solid rgba(255, 255, 255, 0.05);
      border-radius: 16px;
      padding: 18px;
      display: flex;
      flex-direction: column;
      justify-content: space-between;
    }

    .metric-label {
      font-size: 13px;
      color: var(--text-muted);
      font-weight: 600;
      text-transform: uppercase;
      letter-spacing: 0.5px;
      display: flex;
      align-items: center;
      gap: 6px;
    }

    .metric-val {
      font-family: 'JetBrains Mono', monospace;
      font-size: 30px;
      font-weight: 700;
      color: var(--text-main);
      margin: 10px 0 4px 0;
    }

    .metric-unit {
      font-size: 14px;
      font-weight: 400;
      color: var(--text-muted);
      margin-left: 4px;
    }

    .status-tag {
      display: inline-block;
      font-size: 11px;
      font-weight: 700;
      padding: 4px 10px;
      border-radius: 6px;
      align-self: flex-start;
    }

    .tag-safe { background: rgba(16, 185, 129, 0.2); color: #34d399; border: 1px solid rgba(16, 185, 129, 0.3); }
    .tag-warn { background: rgba(245, 158, 11, 0.2); color: #fbbf24; border: 1px solid rgba(245, 158, 11, 0.3); }
    .tag-danger { background: rgba(239, 68, 68, 0.2); color: #f87171; border: 1px solid rgba(239, 68, 68, 0.3); }

    /* ACTION & CONTROLS */
    .controls-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
      gap: 20px;
    }

    .btn {
      cursor: pointer;
      font-family: 'Outfit', sans-serif;
      font-weight: 700;
      font-size: 14px;
      padding: 12px 20px;
      border-radius: 12px;
      border: none;
      display: flex;
      align-items: center;
      justify-content: center;
      gap: 8px;
      transition: all 0.2s ease;
    }

    .btn-primary {
      background: linear-gradient(135deg, #0284c7, #06b6d4);
      color: white;
      box-shadow: 0 4px 15px rgba(2, 132, 199, 0.3);
    }
    .btn-primary:hover {
      transform: translateY(-2px);
      box-shadow: 0 6px 20px rgba(2, 132, 199, 0.5);
    }

    .btn-danger {
      background: linear-gradient(135deg, #dc2626, #ef4444);
      color: white;
    }

    .btn-success {
      background: linear-gradient(135deg, #059669, #10b981);
      color: white;
    }

    .relay-toggle-box {
      display: flex;
      align-items: center;
      justify-content: space-between;
      padding: 12px 18px;
      background: rgba(15, 23, 42, 0.6);
      border-radius: 12px;
      border: 1px solid rgba(255, 255, 255, 0.05);
    }

    footer {
      text-align: center;
      margin-top: 36px;
      color: var(--text-muted);
      font-size: 13px;
    }
  </style>
</head>
<body>
  <div class="container">
    <header>
      <div class="brand">
        <div class="brand-logo">💧</div>
        <div>
          <h1>Groundwater Monitor</h1>
          <div class="badge-sub">Autonomous Groundwater Depletion & Quality Predictor</div>
        </div>
      </div>
      <div class="techfest-pill">
        <div class="pulse"></div>
        Techfest, IIT Bombay — InnovateX Habitat
      </div>
    </header>

    <div class="dashboard-grid">
      <!-- RESERVOIR VISUALIZER -->
      <div class="card reservoir-box">
        <div class="reservoir-title">
          <span>📊</span> Live Well Reservoir
        </div>
        <div class="tank-graphic" id="tankGraphic">
          <div class="water-fill" id="waterFill" style="height: 60%;">
            <div class="water-surface-line"></div>
          </div>
          <div class="pump-intake-marker" id="pumpMarker" style="bottom: 25%;">
            <div class="marker-label">Pump Intake Cutoff</div>
          </div>
        </div>
        <div style="width: 100%; display: flex; justify-content: space-between; font-size: 13px; color: var(--text-muted);">
          <span>Water Column: <b id="lblWaterCol" style="color:var(--text-main);">-- cm</b></span>
          <span>Depth to Surface: <b id="lblDist" style="color:var(--text-main);">-- cm</b></span>
        </div>
      </div>

      <!-- MAIN METRICS TILES -->
      <div class="card">
        <div style="display:flex; justify-content:space-between; align-items:center; margin-bottom:18px;">
          <h2 style="font-size: 18px; font-weight:700;">Groundwater Analytics</h2>
          <span id="lastUpdated" style="font-size:12px; color:var(--text-muted);">Syncing...</span>
        </div>
        
        <div class="metrics-grid">
          <!-- 1. Depletion Rate -->
          <div class="metric-tile">
            <div class="metric-label">📉 Depletion Rate</div>
            <div class="metric-val" id="valDepletion">0.0 <span class="metric-unit">cm/day</span></div>
            <div class="status-tag tag-safe" id="tagDepletion">STABLE</div>
          </div>

          <!-- 2. Days to Dry -->
          <div class="metric-tile">
            <div class="metric-label">⏳ Days to Dry-Run</div>
            <div class="metric-val" id="valDaysToDry">-- <span class="metric-unit">days</span></div>
            <div class="status-tag tag-safe" id="tagDaysToDry">SECURE</div>
          </div>

          <!-- 3. TDS (Salinity) -->
          <div class="metric-tile">
            <div class="metric-label">🧪 Salinity (TDS)</div>
            <div class="metric-val" id="valTDS">-- <span class="metric-unit">ppm</span></div>
            <div class="status-tag tag-safe" id="tagTDS">BIS IS 10500 PASS</div>
          </div>

          <!-- 4. Water Clarity -->
          <div class="metric-tile">
            <div class="metric-label">💎 Clarity / Siltation</div>
            <div class="metric-val" id="valClarity">-- <span class="metric-unit">%</span></div>
            <div class="status-tag tag-safe" id="tagClarity">CLEAR</div>
          </div>

          <!-- 5. Temperature -->
          <div class="metric-tile">
            <div class="metric-label">🌡️ Water Temp</div>
            <div class="metric-val" id="valTemp">-- <span class="metric-unit">°C</span></div>
            <div style="font-size: 11px; color: var(--text-muted);">DS18B20 1-Wire Calibrated</div>
          </div>

          <!-- 6. Battery Status -->
          <div class="metric-tile">
            <div class="metric-label">⚡ Solar Li-ion Battery</div>
            <div class="metric-val" id="valBatt">-- <span class="metric-unit">V</span></div>
            <div class="status-tag tag-safe" id="tagBatt">HEALTHY</div>
          </div>
        </div>
      </div>
    </div>

    <!-- CONTROL PANEL & ACTIONS -->
    <div class="card">
      <h2 style="font-size: 18px; font-weight:700; margin-bottom: 16px;">Intelligent Well Interlock & Hardware Control</h2>
      <div class="controls-grid">
        <div class="relay-toggle-box">
          <div>
            <div style="font-weight: 700; font-size: 15px;">Submersible Pump Safety Relay</div>
            <div style="font-size: 12px; color: var(--text-muted);" id="pumpStatusDesc">Auto-Cutoff Active: Safe to pump</div>
          </div>
          <button class="btn btn-danger" id="btnPumpToggle" onclick="togglePump()">OVERRIDE CUTOFF</button>
        </div>

        <div style="display:flex; align-items:center; gap: 12px;">
          <button class="btn btn-primary" style="flex:1;" onclick="triggerTestAlert()">
            📲 Send Telegram Demo Alert
          </button>
        </div>
      </div>
    </div>

    <footer>
      Underground Water Management Node &bull; Techfest IIT Bombay &bull; Dual-Core ESP32 Edge Predictor
    </footer>
  </div>

  <script>
    let pumpOverridden = false;

    async function fetchData() {
      try {
        const res = await fetch('/api/data');
        if (!res.ok) throw new Error("HTTP error " + res.status);
        const d = await res.json();

        // Update Reservoir Graphic
        const totalDepth = d.total_depth || 100;
        const pumpIntake = d.pump_intake || 85;
        const waterCol = Math.max(0, d.water_column);
        const fillPct = Math.min(100, Math.max(0, (waterCol / totalDepth) * 100));
        
        document.getElementById('waterFill').style.height = fillPct.toFixed(1) + '%';
        const intakePct = Math.min(100, Math.max(0, ((totalDepth - pumpIntake) / totalDepth) * 100));
        document.getElementById('pumpMarker').style.bottom = intakePct.toFixed(1) + '%';

        // Labels
        document.getElementById('lblWaterCol').textContent = waterCol.toFixed(1) + ' cm';
        document.getElementById('lblDist').textContent = d.distance.toFixed(1) + ' cm';

        // Metrics
        document.getElementById('valDepletion').innerHTML = d.depletion_rate.toFixed(1) + ' <span class="metric-unit">cm/day</span>';
        const tagDepl = document.getElementById('tagDepletion');
        if (d.depletion_rate > 15) {
          tagDepl.className = 'status-tag tag-danger'; tagDepl.textContent = 'RAPID DRAWDOWN';
        } else if (d.depletion_rate > 8) {
          tagDepl.className = 'status-tag tag-warn'; tagDepl.textContent = 'MODERATE';
        } else {
          tagDepl.className = 'status-tag tag-safe'; tagDepl.textContent = 'STABLE';
        }

        // Days to Dry
        const valDays = document.getElementById('valDaysToDry');
        const tagDays = document.getElementById('tagDaysToDry');
        if (d.days_to_dry >= 999 || d.days_to_dry < 0) {
          valDays.innerHTML = '&infin; <span class="metric-unit">days</span>';
          tagDays.className = 'status-tag tag-safe'; tagDays.textContent = 'RECHARGE/STABLE';
        } else {
          valDays.innerHTML = d.days_to_dry.toFixed(0) + ' <span class="metric-unit">days</span>';
          if (d.days_to_dry < 7) {
            tagDays.className = 'status-tag tag-danger'; tagDays.textContent = 'CRITICAL RISK';
          } else if (d.days_to_dry < 15) {
            tagDays.className = 'status-tag tag-warn'; tagDays.textContent = 'LOW RESERVE';
          } else {
            tagDays.className = 'status-tag tag-safe'; tagDays.textContent = 'SUFFICIENT';
          }
        }

        // TDS
        document.getElementById('valTDS').innerHTML = d.tds.toFixed(0) + ' <span class="metric-unit">ppm</span>';
        const tagTDS = document.getElementById('tagTDS');
        if (d.tds > 1000) {
          tagTDS.className = 'status-tag tag-danger'; tagTDS.textContent = 'UNFIT (>1000 PPM)';
        } else if (d.tds > 500) {
          tagTDS.className = 'status-tag tag-warn'; tagTDS.textContent = 'PERMISSIBLE (>500)';
        } else {
          tagTDS.className = 'status-tag tag-safe'; tagTDS.textContent = 'POTABLE (<500)';
        }

        // Clarity
        document.getElementById('valClarity').innerHTML = d.clarity.toFixed(0) + ' <span class="metric-unit">%</span>';
        const tagClar = document.getElementById('tagClarity');
        if (d.clarity < 60) {
          tagClar.className = 'status-tag tag-danger'; tagClar.textContent = 'HEAVY SILTATION';
        } else if (d.clarity < 75) {
          tagClar.className = 'status-tag tag-warn'; tagClar.textContent = 'TURBID';
        } else {
          tagClar.className = 'status-tag tag-safe'; tagClar.textContent = 'CRYSTAL CLEAR';
        }

        // Temp & Battery
        document.getElementById('valTemp').innerHTML = d.temp.toFixed(1) + ' <span class="metric-unit">°C</span>';
        document.getElementById('valBatt').innerHTML = d.battery.toFixed(2) + ' <span class="metric-unit">V</span>';
        const tagBatt = document.getElementById('tagBatt');
        if (d.battery < 3.5) {
          tagBatt.className = 'status-tag tag-danger'; tagBatt.textContent = 'LOW CHARGE';
        } else {
          tagBatt.className = 'status-tag tag-safe'; tagBatt.textContent = 'CHARGED (' + d.batt_pct + '%)';
        }

        // Pump State
        const btnPump = document.getElementById('btnPumpToggle');
        const descPump = document.getElementById('pumpStatusDesc');
        if (d.pump_relay_on) {
          btnPump.className = 'btn btn-danger';
          btnPump.textContent = 'EMERGENCY CUTOFF';
          descPump.textContent = 'Pump Running: Water level safe';
        } else {
          btnPump.className = 'btn btn-success';
          btnPump.textContent = 'ENABLE PUMP';
          descPump.textContent = 'Pump Cutoff: Dry-run protection triggered or stopped';
        }

        document.getElementById('lastUpdated').textContent = 'Last ping: ' + new Date().toLocaleTimeString();
      } catch (err) {
        document.getElementById('lastUpdated').textContent = 'Node Offline';
      }
    }

    async function togglePump() {
      await fetch('/api/pump_toggle');
      fetchData();
    }

    async function triggerTestAlert() {
      alert('Triggering Telegram test message...');
      const res = await fetch('/api/test_alert');
      const text = await res.text();
      alert('Response: ' + text);
    }

    setInterval(fetchData, 1800);
    fetchData();
  </script>
</body>
</html>
)rawliteral";
