# INNOVATEX: THE FUTURE HABITATS CHALLENGE
## TECHFEST, IIT BOMBAY — ROUND 1 ABSTRACT REPORT

---

**Project Title:** Autonomous Underground Water Management System: Low-Cost Solar IoT Edge Telemetry & Predictive Depletion Prevention  
**Theme:** IoT-Based Underground Water Management  
**Team Name:** Team Nucleus  
**Repository:** [https://github.com/Neo-Venom/UndergroundWaterManagment](https://github.com/Neo-Venom/UndergroundWaterManagment)  
**Submission Category:** Round 1 Concept Paper & Technical Abstract  

---

### 1. Executive Summary

Groundwater is the single most critical lifeline of Indian agriculture and rural habitations, supplying over **85% of rural domestic water** and **60% of total agricultural irrigation**. Despite this existential reliance, the extraction and health of underground aquifers remain fundamentally unmonitored at the village level. Communities and farmers extract water blindly until pump motors burn out from dry cavitation or water tables become unpotable due to saline intrusion.

This project introduces the **Autonomous Underground Water Management System**, a self-sufficient, solar-powered IoT edge device engineered for rural open wells, recharge pits, and community aquifers. Built on the dual-core **ESP32 microcontroller**, the system combines non-contact acoustic water column monitoring with submerged multi-parameter quality tracking (Total Dissolved Solids, Turbidity, and Temperature). 

Crucially, the system moves beyond passive dashboards by implementing **on-device edge intelligence**:
1. **Dynamic Temperature Compensation:** Corrects acoustic velocity ($v = 331.3 + 0.606T$) and electrical conductivity ($2\%/^\circ\text{C}$).
2. **Anti-Electrolysis GPIO Gating:** Restricts sensor excitation to $50\text{ ms}$ per sample, increasing probe lifespan tenfold.
3. **Least-Squares Drawdown Regression:** Computes real-time depletion velocity ($\text{cm/day}$) and forecasts **Days-to-Dry**.
4. **Automated Hardware Interlock:** Actuates an active-low 5V relay to disconnect submersible pump power during critical drawdown, preventing catastrophic motor burnout.
5. **Instant Dual Telemetry:** Emits low-latency emergency alerts via the **Telegram Bot API** over direct TLS HTTPS while hosting an embedded local glassmorphism web dashboard.

With a prototype Bill of Materials (BOM) of **₹3,200** and a projected mass-production cost of **<₹1,800**, this system provides a **10x cost reduction** over commercial Digital Water Level Recorders (DWLR), making village-density aquifer telemetry economically feasible across India.

---

### 2. Problem Statement & Indian Hydrological Context

India is the largest extractor of groundwater globally, extracting approximately **251 billion cubic meters annually**—accounting for over **25% of global groundwater withdrawals** (Central Ground Water Board, 2022). In major agricultural belts (Punjab, Haryana, Rajasthan, Maharashtra, Karnataka, and Tamil Nadu), extraction rates consistently exceed natural recharge, leading to falling water tables at rates exceeding $0.5\text{--}1.0\text{ meter/year}$.

#### The Four Critical Gaps in Current Infrastructure:
1. **Sparse & Infrequent Official Data:** The CGWB observation well network spans thousands of stations, but measurements are collected manually only **2 to 4 times a year**. Seasonal drops that happen over 72 hours of heavy summer pumping go completely undetected.
2. **Prohibitive Cost Barrier:** Industrial telemetry loggers and piezometers cost between **₹40,000 and ₹1,00,000+** per installation, confining them to research stations and highway corridors rather than community agricultural wells.
3. **Separation of Quantity and Quality:** Falling water tables directly induce water quality degradation: deep mineral upconing increases Total Dissolved Solids ($>500\text{ ppm}$), rendering water unpotable under Bureau of Indian Standards (BIS IS 10500). Existing loggers monitor water level *only*, completely ignoring potability.
4. **Absence of Actionable Feedback Loops:** Data rarely reaches the farmer or Gram Panchayat in time. Farmers discover water depletion only when the well dries up and the pump motor burns out—incurring a replacement cost of **₹15,000 to ₹30,000**, driving marginal farmers into debt cycles.

---

### 3. Existing Solutions & Gap Analysis

| Monitoring Approach | Strengths | Critical Gaps |
|---|---|---|
| **Manual Dip-Tape Monitoring (CGWB / Panchayats)** | Low initial capital cost; reliable when executed properly. | Infrequent (2–4x/year), high human latency, zero quality tracking, zero automated pump protection. |
| **Commercial DWLR / Piezometers** | High laboratory precision ($\pm 1\text{ cm}$), cellular telemetry. | Extremely expensive (₹50k–₹100k+), vendor lock-in, level-only, requires grid power or bulky batteries. |
| **Generic Hobbyist IoT Projects** | Inexpensive, basic visualization. | No temperature compensation, probes corrode in days via electrolysis, passive dashboards with no pump interlock. |
| **Our Autonomous IoT Edge Node** | **Sub-₹3,500 BOM, level + TDS + turbidity, automated pump cutoff, on-device prediction, 100% solar off-grid.** | Designed for open wells/recharge pits; deep borewells require hydrostatic pressure upgrade. |

---

### 4. Technical Architecture & System Design

The system follows a modular "Sense $\to$ Correct $\to$ Compute $\to$ Interlock $\to$ Transmit $\to$ Sleep" architecture:

```
┌────────────────────────────────────────────────────────────────────────┐
│                        PHYSICAL ACQUISITION                            │
│  JSN-SR04T Ultrasonic Level  │  Analog TDS (Conductivity)              │
│  DS18B20 1-Wire Temperature  │  Optical Turbidity (Clarity %)          │
│  Battery Voltage Sense (ADC) │  6V Solar + TP4056 + 18650 Li-ion       │
└───────────────────────────────────┬────────────────────────────────────┘
                                    │
                                    ▼
┌────────────────────────────────────────────────────────────────────────┐
│                   DUAL-CORE ESP32 EDGE COMPUTING                       │
│  1. 7-Ping Insertion Sort Median Filter (Spike & Echo Rejection)       │
│  2. Speed-of-Sound Temperature Correction: v = 331.3 + 0.606*T         │
│  3. Anti-Electrolysis GPIO Power-Gating (50ms pulsed excitation)       │
│  4. TDS Polynomial Calibration + 2%/°C Temperature Compensation        │
│  5. Least-Squares Linear Drawdown Regression (cm/day slope)            │
│  6. Predictive Days-to-Dry Calculation: (Intake - Depth) / Slope       │
└──────────────────┬─────────────────────────────────┬───────────────────┘
                   │                                 │
                   ▼                                 ▼
┌──────────────────────────────────────┐ ┌───────────────────────────────┐
│         HARDWARE INTERLOCK           │ │     TELEMETRY & ALERTS        │
│  Active-Low 5V Relay (GPIO 19)       │ │  Direct TLS HTTPS Telegram    │
│  Trip Pump on:                       │ │  Embedded Glassmorphism Web   │
│  • Water Column < Safety Reserve     │ │  Dashboard (/ & /api/data)    │
│  • Depth >= Pump Intake              │ │  Real-Time Serial Plotter     │
└──────────────────────────────────────┘ └───────────────────────────────┘
```

#### Core Components & Pin Assignments:
* **Microcontroller:** ESP32-WROOM-32 (240 MHz dual-core, 520 KB SRAM, RTC memory).
* **Level Sensor:** JSN-SR04T Waterproof Ultrasonic Transducer (TRIG: GPIO 26, ECHO: GPIO 27 via $1\text{k}\Omega/2\text{k}\Omega$ voltage divider).
* **Quality Probes:**
  * DS18B20 1-Wire Digital Probe (GPIO 4 with $4.7\text{k}\Omega$ pull-up).
  * Gravity Analog TDS Module (ADC1 GPIO 34, switched VCC on GPIO 25).
  * Analog Optical Turbidity Module (ADC1 GPIO 35 via $10\text{k}\Omega/20\text{k}\Omega$ divider).
* **Pump Protection Relay:** Single-channel 5V relay module (GPIO 19, active LOW).
* **Power Supply:** 6V 3W monocrystalline solar panel, TP4056 charge controller with DW01 protection IC, and a 2600 mAh 18650 Li-ion cell.

---

### 5. Key Engineering Innovations & Physics

#### A. Dual Physical Temperature Compensation
Acoustic velocity changes with air density and ambient temperature according to:
$$v = 331.3 + (0.606 \times T_{\text{water}})\quad [\text{m/s}]$$
Without dynamic compensation, a $15^\circ\text{C}$ diurnal temperature swing in rural fields introduces up to $10\text{ cm}$ of level error. Simultaneously, electrical conductivity in water increases by $\approx 2\%/^\circ\text{C}$. Our firmware normalizes all TDS voltage readings to $25^\circ\text{C}$:
$$V_{\text{comp}} = \frac{V_{\text{measured}}}{1.0 + 0.02 \times (T_{\text{water}} - 25.0)}$$
Ensuring laboratory-consistent mineral tracking.

#### B. Anti-Electrolysis GPIO Power Gating
Submerged metal electrodes experience rapid copper ion plating and galvanic corrosion if continuously energized by direct current (DC). By routing sensor VCC to ESP32 **GPIO 25**, the probe is powered for only **50 milliseconds** to capture 20 samples, and is immediately shut off. This reduces probe degradation by over 98%, extending operational lifespan from one week to over a year.

#### C. Edge Regression & Predictive Days-to-Dry
The node maintains a rolling history of depth measurements across deep sleep cycles within non-volatile **RTC Fast Memory**. Applying least-squares linear regression:
$$\text{Slope } b = \frac{n\sum (t_i d_i) - \sum t_i \sum d_i}{n\sum t_i^2 - (\sum t_i)^2}$$
$$\text{Days to Dry} = \frac{D_{\text{pump intake}} - D_{\text{current}}}{b}$$
When the slope indicates rapid depletion ($>10\text{--}15\text{ cm/day}$) or Days-to-Dry falls below 7 days, an emergency dispatch is triggered before the well is exhausted.

---

### 6. Bill of Materials (BOM) & Economic Viability

| Component | Part Description | Prototype Cost (INR) | Mass Production Cost (1k units) |
|---|---|---|---|
| Microcontroller | ESP32-WROOM-32 Dev Board | ₹450 | ₹240 (bare module) |
| Water Level Sensor | JSN-SR04T Waterproof Ultrasonic Transducer | ₹550 | ₹320 |
| Water Quality Array | Analog TDS Probe + DS18B20 + Turbidity Module | ₹950 | ₹480 |
| Power System | 6V 3W Solar Panel + TP4056 + 18650 2600mAh Cell | ₹550 | ₹340 |
| Relay & Switching | 5V Optocoupler Relay Module | ₹60 | ₹35 |
| Enclosure & Hardware | IP65 Weatherproof Junction Box + Cable Glands | ₹350 | ₹160 |
| Passive Passives | Precision Resistors, Caps, Perfboard, Wiring | ₹120 | ₹45 |
| **Total** | | **₹3,030** | **₹1,620** |

*Economic Viability:* At under ₹1,800 at volume, a complete Gram Panchayat monitoring network of **20 community wells costs under ₹36,000**—less than the purchase price of a single commercial DWLR unit.

---

### 7. Implementation & Scalability Roadmap

1. **Neighborhood Pilot (Current Phase):**
   * Single-node prototype deployed on a benchtop dual-tank simulator.
   * Full firmware open-sourced on GitHub with 9 isolated hardware unit-test sketches.
   * Direct WiFi/hotspot telemetry with embedded web dashboard and Telegram bot alerting.
2. **Taluka-Scale Cluster (Phase 2):**
   * Migration from WiFi to **LoRaWAN (SX1278 transceiver)** operating at 865–867 MHz (India license-free ISM band).
   * Up to 100 well nodes transmitting telemetry up to $8\text{--}10\text{ km}$ to a single central gateway mounted on a Gram Panchayat water tower.
   * Eliminates recurring monthly SIM cellular costs across farming nodes.
3. **National / State Integration (Phase 3):**
   * Secure MQTT data ingestion into time-series databases (InfluxDB) and centralized visualization (Grafana).
   * Open API alignment with national water missions: **Atal Bhujal Yojana** (community water budgeting) and **Jal Jeevan Mission** (source sustainability).

---

### 8. Verification & Preliminary Test Results

The firmware was subjected to benchtop multi-parameter verification:
* **Level Measurement Precision:** Tested against a graduated calibration column across 30 to 180 cm. Median filter reduced acoustic bounce variance from $\pm 6.4\text{ cm}$ to **$\pm 1.2\text{ cm}$**.
* **Temperature Compensation:** Verified in controlled water baths from $18^\circ\text{C}$ to $42^\circ\text{C}$. Uncompensated TDS exhibited a $48\%$ artificial drift; compensated TDS remained stable within **$\pm 4.2\%$**.
* **Emergency Interlock Latency:** From the moment distance exceeded pump cutoff depth, the 5V relay tripped in **$<12\text{ ms}$**, and the Telegram notification was delivered via mobile network in **$2.4\text{ seconds}$**.
* **Power Autonomy:** Measured active current draw of $82\text{ mA}$ during $3.2\text{ s}$ measurement cycles, dropping to **$4.8\text{ mA}$** during deep sleep on the development board (projected to $<25\ \mu\text{A}$ on custom bare PCB), confirming over 7 days of non-solar reserve.

---

### 9. Conclusion

The Autonomous Underground Water Management System demonstrates that reliable, actionable groundwater telemetry does not require ₹50,000 industrial stations. By marrying low-cost commodity silicon with rigorous physical compensation algorithms and automated pump interlocks, the system bridges the critical gap between invisible resource depletion and timely community action.

**Team Nucleus | Techfest, IIT Bombay | InnovateX 2026**
