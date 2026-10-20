# Pinout & Wiring

Target board: **ESP32 DevKit (30/38-pin, ESP32-WROOM-32)**. Pin numbers are **GPIO numbers** (not the silkscreen position).

> ⚠️ **Read before wiring**
> 1. **Max 3.3 V on any ESP32 GPIO.** 5 V signals need a divider.
> 2. **Use ADC1 pins only for analog sensors** (GPIO32–39). ADC2 is unusable while WiFi is active.
> 3. **GPIO34–39 are input-only** and have no internal pull-ups. That's fine for analog reads.
> 4. **Common ground:** every module must share GND.
> 5. Set the boost converter to **5.0 V with a multimeter before** connecting the ESP32.

---

## 1. Pin Map

| Function | ESP32 GPIO | Direction | Connects to | Notes |
|---|---|---|---|---|
| DS18B20 data (1-Wire) | **GPIO4** | I/O | DS18B20 yellow/data | 4.7 kΩ pull-up to 3.3 V |
| Ultrasonic TRIG | **GPIO26** | Output | JSN-SR04T TRIG | 3.3 V output works as trigger |
| Ultrasonic ECHO | **GPIO27** | Input | JSN-SR04T ECHO **via divider** | 1 kΩ / 2 kΩ divider if sensor is powered at 5 V |
| TDS analog out | **GPIO34** (ADC1_CH6) | Analog in | TDS module A-out | Module powered from GPIO25 (3.3 V) |
| TDS power | **GPIO25** | Output | TDS module VCC | HIGH only during measurement (~3–6 mA) |
| Turbidity analog out | **GPIO35** (ADC1_CH7) | Analog in | Turbidity module A-out **via divider** | 10 kΩ / 20 kΩ divider |
| Battery sense | **GPIO32** (ADC1_CH4) | Analog in | Battery+ via 100 kΩ / 100 kΩ divider | Reads half of battery voltage |
| Status LED | **GPIO2** | Output | On-board LED | Optional, blink on send |
| Pump Relay Cutoff | **GPIO19** | Output | 5 V Relay IN | Cutoff pump on dry well (Active LOW) |
| *(opt.)* OLED SDA | GPIO21 | I/O | OLED SDA | I²C |
| *(opt.)* OLED SCL | GPIO22 | I/O | OLED SCL | I²C |
| *(opt.)* Rain gauge | GPIO14 | Input | Reed switch to GND | RTC-capable, can wake from deep sleep |

**Pins deliberately avoided:** GPIO0, 5, 12, 15 (boot/strapping pins), GPIO6–11 (flash), GPIO36/39 (kept free).

The same map lives in code at [`firmware/include/pins.h`](../firmware/include/pins.h). If you change one, change the other.

---

## 2. Per-Sensor Wiring

### DS18B20 (waterproof probe)
```
Red    (VCC)  ──► 3.3 V
Black  (GND)  ──► GND
Yellow (DATA) ──► GPIO4
                   └── 4.7 kΩ ──► 3.3 V   (pull-up between DATA and 3.3 V)
```
Second probe (optional air temp): wire in **parallel on the same three wires**.

### JSN-SR04T (waterproof ultrasonic)
```
VCC  ──► 5 V (boost rail)
GND  ──► GND
TRIG ──► GPIO26
ECHO ──► 1 kΩ ──┬──► GPIO27
                │
                2 kΩ
                │
               GND          (5 V × 2/3 ≈ 3.3 V)
```
If your module is **V3.0 and powered at 3.3 V**, the divider is not needed (and the range may be slightly reduced).

### TDS sensor module
```
VCC  ──► GPIO25   (switched 3.3 V, on only when measuring)
GND  ──► GND
AOUT ──► GPIO34   (add 100 nF from GPIO34 to GND near the ESP32)
```
Probe goes into the module's BNC connector; keep probe cable away from the ultrasonic and power wires.

### Turbidity sensor module
```
VCC  ──► 5 V
GND  ──► GND
AOUT ──► 10 kΩ ──┬──► GPIO35
                 │
                 20 kΩ
                 │
                GND        (4.5 V × 2/3 ≈ 3.0 V; code multiplies by 1.5)
                 + 100 nF from GPIO35 to GND
```

### Battery voltage sense
```
Battery+ ──► 100 kΩ ──┬──► GPIO32
                      │
                      100 kΩ
                      │
                     GND      (reads half the battery voltage; code multiplies by 2)
                      + 100 nF from GPIO32 to GND
```

### Pump Relay Cutoff Module (Active LOW)
```
VCC  ──► 5 V (or 3.3 V depending on relay module coil/logic)
GND  ──► GND
IN   ──► GPIO19   (LOW = Relay energized / Pump ON; HIGH = Relay tripped / Cutoff)
COM  ──► Live / +V of Submersible Pump
NO   ──► Pump Terminal (Normally Open: power flows only when well has sufficient water)
```


---

## 3. Power Path

```
 Solar 6 V ──► TP4056 (with protection) IN+/IN−
                  │
                  ├── B+/B−  ──► 18650 cell
                  │
                  └── OUT+ ──► MT3608 IN+          OUT− ──► common GND
                                   │
                               MT3608 OUT (set to 5.0 V)
                                   │
              ┌────────────────────┼─────────────────────┐
              ▼                    ▼                     ▼
        ESP32 VIN pin        JSN-SR04T VCC        Turbidity VCC
```

ESP32's on-board regulator makes 3.3 V for the ESP32, DS18B20, and the pull-up. The TDS module is fed from GPIO25.

**Battery sense** taps the battery side (TP4056 OUT+ / battery+), *before* the boost converter.

### Power safety
- **Programming over USB:** disconnect the boost converter from VIN first (or confirm your board has a diode on VIN). Otherwise two 5 V sources fight.
- Fuse or at least **double-check polarity** on the 18650 before connecting.
- Don't leave the TP4056 module's bare (unprotected) version on a battery with no protection circuit.

---

## 4. Breadboard Layout Tips

- Keep **analog wires short** and away from the ultrasonic TRIG/ECHO and the boost converter (switching noise).
- Put **100 nF capacitors** physically at the ESP32 end of each analog line.
- Use a **common GND rail**; star-connect if noise appears.
- Label the two DS18B20 probes the first time you power them (print addresses in `BENCH_MODE`).
- Move to perfboard or a PCB before the field test; breadboard contacts fail with vibration and humidity.

---

## 5. Quick Test Checklist

- [ ] Boost converter output is 5.0 V with no load
- [ ] 3.3 V pin reads 3.3 V
- [ ] Echo divider output reads ≈ 3.3 V when the echo line is high (or at least ≤ 3.3 V)
- [ ] Turbidity divider output never exceeds 3.3 V (check with sensor in clean water, it's the max case)
- [ ] Battery divider reads ≈ half battery voltage
- [ ] All grounds are common
