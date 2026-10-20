# 🛠️ Implementation Guide

This is the build manual: architecture, algorithms, build order with verification at each step, power budget, testing, and known limitations.

---

## 1. System Architecture

```
┌──────────────────────── FIELD NODE ────────────────────────┐
│  JSN-SR04T ─┐                                               │
│  TDS probe ─┤                                               │
│  DS18B20 ───┼──► ESP32 ──► deep sleep 30 min ──► wake ──┐   │
│  Turbidity ─┤        ▲                                  │   │
│  Battery ADC┘        │                                  │   │
│  Solar → TP4056 → 18650 → 5 V boost ────────────────────┘   │
└───────────────┬──────────────────────────┬──────────────────┘
                │ HTTP (WiFi)              │ HTTPS (WiFi)
                ▼                          ▼
          ThingSpeak channel         Telegram Bot API
                │                          │
                ▼                          ▼
     Dashboard + analysis/trend.py   Farmers / Panchayat phones
```

**Design principle:** all *decisions* (trend, thresholds, alerts) happen **on the node**, so alerts still work if the dashboard is down. The cloud is for storage, visualisation, and longer-term analysis.

## 2. Firmware Structure

`firmware/src/main.cpp` follows a simple "wake → measure → decide → send → sleep" cycle:

```
setup()
 ├─ init pins, ADC attenuation
 ├─ bootCount++
 ├─ takeReading()
 │    ├─ readTemps()              water + (optional) air temperature
 │    ├─ readDistanceCm(airTemp)  median of 7 pings, speed-of-sound corrected
 │    ├─ readTdsPpm(waterTemp)    median of 30 ADC samples, temp-compensated
 │    ├─ readClarityPct()         turbidity voltage → % of clean-water voltage
 │    └─ readBatteryV()
 ├─ pushHistory(distance)         ring buffer in RTC memory (48 samples ≈ 24 h)
 ├─ depletionCmPerDay()           least-squares slope
 ├─ daysToDry()
 ├─ connectWiFi()
 ├─ sendThingSpeak()
 ├─ evaluate alerts → sendTelegram() (with cooldown)
 └─ esp_deep_sleep_start()
```

`BENCH_MODE=1` (in `config.h`) skips WiFi and sleep and just prints readings every 2 s. Use it for all calibration.

## 3. Key Algorithms

### 3.1 Distance (ultrasonic)

```
speed_of_sound (m/s) = 331.3 + 0.606 × T(°C)
distance (cm)        = echo_time (µs) × speed (cm/µs) / 2
```

Take 7 pings 70 ms apart, discard those outside 25–600 cm (JSN-SR04T has a ~25 cm blind zone), and take the **median**. Requires at least 4 valid pings or the reading is flagged invalid.

Depth to water = measured distance + `SENSOR_OFFSET_CM` (height of sensor above ground, if any).

### 3.2 TDS with temperature compensation

```
k          = 1 + 0.02 × (T − 25)        // 2 % per °C
V_comp     = V_measured / k
TDS (ppm)  = (133.42·V³ − 255.86·V² + 857.39·V) × 0.5 × TDS_CAL_FACTOR
```

This is the standard polynomial for the Gravity analog TDS probe. `TDS_CAL_FACTOR` is set during calibration (see `docs/calibration.md`).

### 3.3 Depletion trend

Keep the last 48 distance readings (≈ 24 h at 30-min interval). Fit `distance = a + b·t` using least squares, where `t` is in days.

- `b > 0` → distance to water is growing → **level is falling** at `b` cm/day.
- Need ≥ 6 samples before reporting a rate.

```
days_to_dry = (PUMP_INTAKE_DEPTH_CM − current_distance) / b      (if b > 0.5 cm/day)
```

> **Note:** pumping causes short-term drops that recover. A 24 h window smooths most of this. For the real deployment, compute from **daily minimums/morning readings** over 7+ days (the Python script does this on the cloud side).

### 3.4 Alert logic

| Alert | Trigger (defaults in `config.h`) | Cooldown |
|---|---|---|
| Depletion | rate > 15 cm/day **or** days-to-dry < 14 | 12 h |
| Quality | TDS > 500 ppm **or** clarity < 70 % | 12 h |
| Battery | battery < 3.5 V | 12 h |

TDS 500 ppm corresponds to the "acceptable" limit for drinking water in the Indian standard IS 10500 (up to 2,000 ppm "permissible" in the absence of an alternative). **Verify current values before citing in your report.**

Cooldown prevents alert spam; state lives in RTC memory.

## 4. ThingSpeak Channel Layout

| Field | Name | Unit |
|---|---|---|
| field1 | distance_to_water | cm |
| field2 | water_column | cm (well depth − distance) |
| field3 | tds | ppm |
| field4 | water_temp | °C |
| field5 | clarity | % |
| field6 | depletion_rate | cm/day |
| field7 | battery | V |
| field8 | days_to_dry | days |

Free ThingSpeak accepts one update per 15 s per channel; we send one per 30 min, so there is plenty of headroom.

## 5. Build Order (verify at each step before moving on)

### Step 1: Blink + serial
Confirm the toolchain works. **Pass:** serial prints `boot 1`.

### Step 2: DS18B20
Wire per `docs/pinout.md` (4.7 kΩ pull-up between data and 3.3 V). **Pass:** room temp ±1 °C; hold in hand → rises.

### Step 3: JSN-SR04T
Mount facing down on a stand above a bucket. **Pass:** matches ruler within ±2 cm at 30, 50, 100 cm. Lower the water level → distance grows.

### Step 4: TDS
Test with tap water, distilled water (≈ 0–20 ppm), and a known solution (see calibration doc). **Pass:** distilled ≪ tap, and within ±10% of a handheld meter after calibration.

### Step 5: Turbidity
Clean water = reference voltage (`TURB_V_CLEAR`). Stir in a pinch of soil/flour. **Pass:** clarity % drops clearly.

### Step 6: Combine all sensors
Run in `BENCH_MODE`. **Pass:** all values stable simultaneously for 10 min (no crosstalk; ADC readings not jumping).

### Step 7: Cloud
Enable WiFi, send to ThingSpeak. **Pass:** chart updates; data matches serial output.

### Step 8: Telegram
Use a test threshold (e.g., TDS > 50) to trigger. **Pass:** phone receives the message; second trigger within cooldown is suppressed.

### Step 9: Deep sleep
Set `BENCH_MODE 0`, `SLEEP_MINUTES 1` for testing. **Pass:** node wakes every minute, `bootCount` increments (check ThingSpeak timestamps), history persists.

### Step 10: Trend
Drain the test tank at a known rate (e.g., mark and time it). **Pass:** reported cm/day ≈ actual (extrapolate); days-to-dry sensible.

### Step 11: Solar + battery
Wire TP4056 + 18650 + boost converter. **Pass:** runs 24 h, battery voltage trace on dashboard rises in sun, falls at night.

### Step 12: Enclosure + soak test
Seal, mount, run ≥ 24 h on the real setup. Log failures.

## 6. Power Budget (estimates, **measure your own**)

| State | Current (approx.) | Duration per cycle |
|---|---|---|
| Active (WiFi + sensors) | 100–200 mA average | ~25 s |
| Deep sleep, ESP32 dev board | ~5–10 mA (regulator + USB chip dominate) | ~30 min |
| Deep sleep, low-power board (e.g., FireBeetle ESP32) | ~0.01–0.1 mA | ~30 min |

Rough per-day charge on a standard dev board: active ≈ 48 cycles × 25 s × 150 mA ≈ **~50 mAh**; sleep ≈ 24 h × 7 mA ≈ **~170 mAh** → ~220 mAh/day at battery level (before boost-converter losses).

- A single 18650 (~2,000 mAh) → roughly **7–9 days** without sun. Matches target O6 (≥ 7 days).
- A 6 V / 2–3 W panel gives enough margin in normal sun. Monsoon needs a bigger panel or a low-power board.

**Takeaway for the pitch:** the dev board's sleep current dominates the budget; moving to a low-quiescent board cuts consumption by a large factor. This is a known, quantified optimisation, not a hidden flaw.

## 7. Testing Plan

| Test | Method | Pass criterion |
|---|---|---|
| Level accuracy | Compare with tape at 5 distances | ±2 cm |
| Level repeatability | 20 readings at a fixed level | σ < 0.5 cm |
| TDS accuracy | Handheld meter, 3 solutions | ±10% |
| Temp compensation | Same sample at 15 °C and 35 °C | Readings agree within ~10% |
| Alert latency | Trigger and time to phone | < 5 min |
| Alert cooldown | Trigger twice in 1 h | 1 message |
| Power autonomy | Battery only, no sun | ≥ 7 days (or extrapolated from measured mA) |
| Soak | 24 h continuous | No crashes, no missed uploads |
| Failure: no WiFi | Turn off AP | Node sleeps cleanly; recovers next cycle |
| Failure: sensor disconnected | Unplug JSN-SR04T | Node reports NaN/invalid, doesn't crash |

Record **actual measurements** in `docs/calibration.md`. Real numbers in the report beat claims.

## 8. Deployment Plan (for the report)

1. **Pilot (1 village, 5–10 wells):** choose wells with contrasting use (irrigation vs drinking). Partner with the gram panchayat / local NGO / an Atal Bhujal Yojana implementing agency.
2. **Mount:** sensor bracket on the well rim, ultrasonic pointing down through a PVC stilling pipe if the well is narrow; enclosure and solar panel at the top, locked/secured.
3. **Connectivity:** WiFi where available, otherwise add a LoRa gateway on a rooftop (one gateway can serve many nodes within several km line-of-sight).
4. **Calibration at install:** record tape-measured depth at day 0.
5. **Alert recipients:** panchayat + registered farmers (Telegram now, SMS later).
6. **Maintenance:** clean TDS/turbidity probes monthly; check battery/panel quarterly.
7. **Data ownership:** open data per village, visible to the community.

## 9. Known Limitations & Mitigations

| Limitation | Why it matters | Mitigation |
|---|---|---|
| TDS/turbidity are **proxies** | Can't identify fluoride/arsenic/nitrate | Position as an early-warning flag; trigger "get it lab-tested"; future: ion-selective sensors or lab partner |
| Ultrasonic in narrow borewells | Pipe walls cause false echoes | Stilling pipe, narrow-beam mounting, median filtering; upgrade path: submersible pressure transducer |
| Temperature affects speed of sound | Error ~0.17 %/°C if ignored | Compensated using DS18B20 (air temp ideally) |
| TDS probe fouling | Drift over weeks | Monthly cleaning, periodic recalibration |
| Cheap ESP32 ADC is nonlinear | Reading errors | `analogReadMilliVolts` + median filter + calibration |
| WiFi not available in villages | Can't upload | LoRa gateway (stretch); store-and-forward buffer (future) |
| Pumping dips distort trend | False alarms | Use longer windows / daily minimum in cloud analysis |
| Theft/vandalism | Field hardware | Low cost per node, tamper-evident enclosure, community ownership |
| Moisture/condensation | Corrodes electronics | IP65 box, cable glands, silica gel, conformal coating |
| Telegram `setInsecure()` in demo | No cert validation | Pin the root CA in production |

## 10. Future Work

- LoRa + gateway for off-grid areas
- Submersible pressure sensor for deep borewells
- Local-language voice/SMS alerts
- Ion-selective probes (nitrate, fluoride) or periodic lab-sample integration
- ML-based anomaly detection and recharge forecasting (combine with rainfall)
- OTA firmware updates
