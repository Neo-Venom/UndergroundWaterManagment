# 🎯 Goals

## Vision

Every village well in India has an affordable "eye" that tells the community **how much water is left, how fast it is going, and whether it is safe**, early enough to act.

## One-Line Goal

> Build and demonstrate a **sub-₹4,000, solar-powered IoT node** that monitors groundwater depletion and basic quality, predicts when a well will run dry, and alerts users automatically.

## Objectives

| # | Objective | Measurable target |
|---|---|---|
| O1 | Measure depth-to-water reliably | Within **±2 cm** of a ruler/tape reference on the bench |
| O2 | Measure dissolved solids | Within **±10%** of a reference TDS meter after calibration |
| O3 | Measure temperature and compensate readings | DS18B20 within ±0.5 °C; TDS reading compensated to 25 °C |
| O4 | Predict depletion | Report **cm/day** rate and **days-to-pump-intake** from 24 h of data |
| O5 | Alert automatically | Telegram alert within **5 min** of a threshold crossing; max 1 alert per type per 12 h |
| O6 | Run unattended | **≥ 7 days** battery autonomy without sun; indefinite with panel |
| O7 | Be affordable | Prototype BOM **≤ ₹4,000**; production target **≤ ₹2,000** |
| O8 | Be demonstrable live | Drain/refill a water tank on stage and show the dashboard + alert reacting |

## Non-Goals (for the hackathon)

- Replacing lab water testing or detecting specific contaminants (fluoride, arsenic).
- Building a custom PCB (nice to have, not required).
- National-scale backend. We design for it and show the plan, but build a single-node pilot.

## Stretch Goals (only after core works)

- [ ] LoRa link: node → gateway → internet
- [ ] Hindi/Marathi alert text, SMS fallback
- [ ] Rain gauge to correlate recharge with rainfall
- [ ] Multi-node map dashboard
- [ ] Anomaly detection (sudden drop = pump running vs. long-term decline)

## How We Map to Judging

### Round 1: Abstract + 10-minute pitch

| Required section | Where we cover it |
|---|---|
| Team information | `presentation.md` slide 1 |
| Specific problem within theme | `docs/problem-statement.md` |
| Existing solutions and gaps | `docs/problem-statement.md` §4 |
| Proposed solution and technology | `docs/solution.md`, `docs/components.md` |
| Impact and scalability | `docs/cost-and-scalability.md` |

### Round 2: Report + prototype + presentation

| Required section | Where we cover it |
|---|---|
| Solution description | `docs/solution.md` |
| Prototype demonstration | `presentation.md` (demo script), working hardware |
| Cost and market viability | `docs/cost-and-scalability.md` |
| Implementation plan | `implementation.md` |
| Scalability plan | `docs/cost-and-scalability.md` §4 |
| Impact assessment | `docs/cost-and-scalability.md` §5 |

## Definition of Done (Round 2 prototype)

- [ ] Node reads all four sensors and battery, calibrated
- [ ] Runs on solar + battery with deep sleep for ≥ 24 h continuous
- [ ] Data visible on a live dashboard
- [ ] Depletion rate and days-to-dry computed
- [ ] Telegram alerts for depletion, quality, low battery
- [ ] Live demo with drain/refill tank works 3 times in a row
- [ ] Report, slides and repo are complete and consistent
