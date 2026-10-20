# Solution Design: Underground Water Management System
 
## 1. Solution Statement

> **Our Underground Water Management System is a low-cost, solar-powered IoT node that continuously measures groundwater level and basic quality, predicts how long the well will last, and automatically alerts the community when action is needed.**

## 2. What Makes It Different

| Typical project | Our System |
|---|---|
| Shows a live level on a dashboard | **Predicts** days-to-dry and depletion rate |
| Level only | Level **and** quality proxies (TDS, turbidity, temperature) |
| Needs mains/USB power | **Solar + deep sleep**, deployable anywhere |
| Dashboard-dependent | **On-device decisions**, alerts even if the dashboard is down |
| "IoT demo" | Thought through for **cost, scale, deployment and maintenance** |

## 3. How It Works, End to End

1. **Sense:** four sensors plus battery voltage are read every 30 minutes.
2. **Correct:** water temperature compensates TDS; air temperature corrects the speed of sound for the ultrasonic distance.
3. **Filter:** median filters reject noise and outliers.
4. **Analyse:** the node keeps ~24 h of readings and fits a trend to estimate cm/day and days-to-dry.
5. **Communicate:** reading goes to ThingSpeak; if a threshold is crossed, a Telegram message goes to registered recipients.
6. **Sleep:** the node deep-sleeps to save power.
7. **Learn:** `analysis/trend.py` analyses long-term data in the cloud (daily minima, longer-window trend).

## 4. Measured Quantities and What They Tell Us

| Quantity | Sensor | Derived value | Decision it supports |
|---|---|---|---|
| Distance to water surface | JSN-SR04T | Depth to water, water column, cm/day, days-to-dry | When to cut pumping, plan alternate supply |
| TDS | Analog TDS probe | ppm (temp-compensated) | Salinity / ingress / "needs lab test" |
| Turbidity | Analog turbidity | Clarity % (relative) | Siltation / contamination warning |
| Water temperature | DS18B20 | °C | Compensation; seasonal context |
| Battery voltage | ADC divider | V | Maintenance trigger |

## 5. Alert Design

| Alert | Trigger | Message intent | Recipient |
|---|---|---|---|
| **Depletion** | Rate above threshold or days-to-dry below threshold | "Reduce pumping / plan alternate source" | Farmer, panchayat |
| **Quality** | TDS above limit or clarity below limit | "Don't drink; get tested" | Households, panchayat |
| **Battery/health** | Battery low or node silent | "Maintenance needed" | Local technician |

Rules: one alert per type per 12 h (cooldown), plain language, includes the numbers, includes a **recommended action**.

## 6. Design Decisions and Rationale

| Decision | Why |
|---|---|
| **ESP32** | WiFi built in, 12-bit ADC, deep sleep, cheap, Arduino framework (C++) |
| **Ultrasonic level** | Non-contact (no corrosion), ~₹500, no calibration drift from submersion |
| **Median filtering** | Cheap, robust against spikes from ADC noise and false echoes |
| **On-device trend** | Alerts survive cloud outages; reduces data traffic |
| **ThingSpeak** | Free, zero server setup, charts out of the box, ideal for a prototype |
| **Telegram** | Free, instant, no extra app for many users; SMS planned for non-smartphone users |
| **Deep sleep, 30 min interval** | Groundwater changes slowly; frequent sampling wastes energy |
| **Solar + 18650** | No wiring, no recurring power cost |

## 7. Scaling the Design

| Stage | Nodes | Connectivity | Backend |
|---|---|---|---|
| Prototype | 1 | WiFi | ThingSpeak |
| Village pilot | 5–10 | WiFi or one LoRa gateway | ThingSpeak or small cloud server |
| Taluka | ~100 | LoRa gateways | Self-hosted DB + dashboard (e.g., MQTT + InfluxDB + Grafana) |
| District/state | 1,000+ | LoRa/NB-IoT/GSM | Multi-tenant dashboard; integration with state water portals |

## 8. Stakeholders and Value

| Stakeholder | Value |
|---|---|
| Farmers | Early warning; avoid failed borewells and wasted drilling |
| Panchayats | Data for water budgeting and recharge planning |
| NGOs/CSR | Measurable outcomes for water programmes |
| Government agencies | Dense, cheap monitoring to complement official networks |

## 9. Ethical and Practical Considerations

- **Data:** share at well/village level; no personal data collected.
- **False alarms:** tune thresholds with local data; show confidence; never claim lab-grade accuracy.
- **Safety messaging:** quality alerts say "get tested", not "this is safe/unsafe" in absolute terms.
- **Inclusion:** alerts in local languages and via SMS for feature-phone users (roadmap).
