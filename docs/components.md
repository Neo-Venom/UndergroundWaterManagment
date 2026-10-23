# Components: What, Why, and What It Must Do

For each part: **what it is**, **why we chose it**, **what job it does in the system**, and **gotchas**. Prices are rough Indian-market estimates; update with what you actually paid.

---

## Core (must-have)

### 1. ESP32 DevKit (30/38-pin)
- **What:** Dual-core 240 MHz microcontroller with WiFi/Bluetooth, 12-bit ADC, deep sleep.
- **Why:** WiFi built in (no extra module), enough ADC channels, programmable in C++ with the Arduino framework, ~₹450–650.
- **Job:** Reads all sensors, runs the filtering/trend/alert logic, uploads data, sleeps.
- **Gotchas:**
  - **ADC2 pins don't work while WiFi is on.** We use **ADC1 only** (GPIO32–39).
  - ADC is nonlinear → use `analogReadMilliVolts()` + median filter.
  - Max 3.3 V on any GPIO. **5 V signals (ultrasonic echo, turbidity output) need dividers.**
  - Dev board deep-sleep current is ~5–10 mA because of the regulator and USB chip; fine for prototype, replace for production.
- **Alternative:** Arduino Uno has no WiFi, less memory, 5 V logic and no deep sleep worth mentioning → **not used**.

### 2. JSN-SR04T Waterproof Ultrasonic Sensor
- **What:** Ultrasonic distance sensor with a separate waterproof transducer on a ~2.5 m cable.
- **Why:** Non-contact (no corrosion), waterproof probe, range ~25–600 cm, ~₹450–700.
- **Job:** Measures distance from the sensor (mounted at the top of the well) down to the water surface → depth to water.
- **Gotchas:**
  - **Blind zone ≈ 25 cm**, so mount ≥ 30 cm above the highest water level.
  - Beam hits well walls in narrow wells → false echoes. Use a **PVC stilling pipe** or aim straight down; filter with a median.
  - Check the module version: **V3.0 works at 3.3–5 V**; older ones need 5 V. At 5 V the Echo output is 5 V → **divider needed** for the ESP32.
  - Speed of sound changes with temperature (~0.17 %/°C) → compensate.
- **Alternative:** Submersible pressure transducer (more accurate for deep borewells, higher cost). Listed as the upgrade path.

### 3. Gravity-style Analog TDS Sensor + Probe
- **What:** Measures electrical conductivity of water and converts it to an estimated TDS (ppm).
- **Why:** Cheap quality indicator of salinity / dissolved ions, simple analog output (~₹400–1,500 depending on brand).
- **Job:** Gives a quality proxy; triggers "don't drink / get tested" alerts.
- **Gotchas:**
  - Output scales with **temperature (≈ 2 %/°C)** → compensate with the DS18B20.
  - Needs **calibration** against a known solution or reference meter.
  - Probe fouls over time → monthly cleaning.
  - Don't leave it powered continuously (electrolysis wears the probe): we power it from a GPIO only during measurement.
  - TDS ≠ safe or unsafe. It cannot detect specific contaminants.

### 4. DS18B20 Waterproof Temperature Sensor
- **What:** Digital 1-Wire temperature probe, ±0.5 °C.
- **Why:** Digital (no ADC noise), waterproof stainless probe, ~₹100–200. Multiple sensors share one pin.
- **Job:** Water temperature for **TDS compensation**; an optional second probe in air for **speed-of-sound correction**.
- **Gotchas:** Needs a **4.7 kΩ pull-up** between data and 3.3 V. Sensor order on the bus is not guaranteed; label the two probes during the first run.

### 5. Turbidity Sensor (analog, e.g., SEN0189-style)
- **What:** Infrared light transmission through water; clearer water → higher voltage.
- **Why:** Cheap indicator of cloudiness (siltation, surface-water ingress), ~₹400–900.
- **Job:** Reports **clarity %** relative to clean water.
- **Gotchas:**
  - Module works at 5 V; output up to ~4.5 V → **divider** to the ESP32.
  - Absolute NTU values are unreliable; we report **relative clarity** after calibrating to clean water.
  - Light-sensitive, so mount shielded.

### 6. Solar Panel (6 V, 2–5 W)
- **What:** Small photovoltaic panel.
- **Why:** Makes the node self-sufficient, with no wiring or electricity bill, and it's the core of the scalability story. ~₹250–450.
- **Job:** Charges the battery during the day.
- **Gotchas:** Open-circuit voltage of a "6 V" panel can be ~7–8 V. TP4056 input max is ~8 V, so stay within the limit. Monsoon needs margin, so size up if in doubt.

### 7. TP4056 Charger Module **with protection (DW01)**
- **What:** Li-ion charge controller module (1 A).
- **Why:** Dirt cheap (~₹30–60), well-known, protects the cell from over-discharge/short when using the protected version.
- **Job:** Safely charges the 18650 from the solar panel.
- **Gotchas:** Use the version **with protection IC** and connect the load to the **OUT+/OUT−** pads, not directly to BAT. Charging and discharging simultaneously works, but isn't ideal; acceptable for a prototype.

### 8. 18650 Li-ion Cell (~2,000–3,000 mAh) + Holder
- **What:** Rechargeable cell.
- **Why:** Cheap, high capacity, widely available. Buy a reputable cell, not "9,900 mAh" fakes.
- **Job:** Powers the node at night and on cloudy days.
- **Gotchas:** Never short it. Never use unprotected modules.

### 9. MT3608 (or similar) Boost Converter
- **What:** Step-up DC-DC converter.
- **Why:** Battery gives 3.0–4.2 V; ESP32 dev board VIN and the 5 V sensors want ~5 V. ~₹50–80.
- **Job:** Provides a stable **5.0 V rail** (set with a multimeter *before* connecting anything).
- **Gotchas:** Adjust the trimmer to 5.0 V first. **Don't connect USB and the 5 V rail to VIN at the same time** unless the board has a protection diode; disconnect the battery rail while flashing.

### 10. Resistors, Capacitors, Wires, Breadboard/Perfboard
- **4.7 kΩ ×1:** DS18B20 pull-up
- **1 kΩ + 2 kΩ:** Echo divider (5 V → 3.3 V)
- **10 kΩ + 20 kΩ:** Turbidity divider (4.5 V → 3.0 V)
- **100 kΩ ×2:** Battery voltage divider
- **100 nF ×3:** ADC pin noise filters (TDS, turbidity, battery)
- **Job:** Make 5 V sensors and the ADC safe and quiet for a 3.3 V ESP32.

### 11. IP65 Enclosure + Cable Glands
- **What:** Plastic junction box.
- **Why:** Protects electronics from rain, dust, and humidity (~₹250–400).
- **Job:** Houses ESP32, charger, battery; sensor cables exit through glands.
- **Gotchas:** Add silica gel; seal glands; keep the solar panel outside, tilted.

---

## Optional / Stretch

| Component | Why | Job | Notes |
|---|---|---|---|
| **LoRa SX1278 + gateway** | Long-range, no WiFi needed | Node → gateway | Biggest scalability upgrade; needs a second ESP32 as gateway |
| **0.96" OLED (I²C)** | Local readout | Shows level/TDS on site | GPIO21/22; avoid if power is tight |
| **Tipping-bucket rain gauge / rain sensor** | Correlate recharge with rainfall | Count rain events | Reed switch on an RTC-capable GPIO |
| **Soil-moisture sensor** | Show recharge conditions | Context | Corrodes; use capacitive type |
| **Submersible pressure transducer** | Higher accuracy for borewells | Alternative level sensor | Needs ADC/amplifier; higher cost |
| **Low-power ESP32 board (e.g., FireBeetle)** | µA-level deep sleep | Production power saving | Resolves the dev-board sleep-current issue |
| **MOSFET power switch** | Cut power to 5 V sensors during sleep | Saves several mA | P-channel high-side switch driven by a GPIO |

---

## What We Deliberately Don't Use

| Part | Reason |
|---|---|
| Arduino Uno | No WiFi, 5 V logic, higher sleep current, no benefit over ESP32 |
| MQ-135 / gas sensors | Not relevant to the groundwater theme |
| GSM module (SIM800) | Add only for no-WiFi/no-LoRa SMS; cost and power overhead |
| Raspberry Pi | Overkill, high power draw |
