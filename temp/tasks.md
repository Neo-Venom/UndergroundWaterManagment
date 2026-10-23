# ✅ Tasks

Team of up to 4. Suggested roles (swap as needed):

| Tag | Role | Owns |
|---|---|---|
| **FW** | Firmware lead | `firmware/`, sensor code, sleep, alerts |
| **HW** | Hardware lead | wiring, power, enclosure, test rig |
| **CL** | Cloud/data lead | ThingSpeak, dashboard, `analysis/` |
| **DOC** | Research/presentation lead | docs, report, slides, pitch |

> Add real dates next to each phase once you know the zonal-centre date and the Round 2 date. Work **backwards** from those.

Legend: `[ ]` todo · `[~]` in progress · `[x]` done

---

## Phase 0: Setup (Day 1–2)

- [ ] **ALL** Decide team name, role split, communication channel
- [ ] **DOC** Create GitHub repo, push this skeleton, add teammates
- [ ] **HW** Order/collect parts (see `docs/components.md`, `docs/cost-and-scalability.md`)
- [ ] **FW** Install PlatformIO, confirm ESP32 blink + serial works
- [ ] **CL** Create ThingSpeak account + channel (8 fields), create Telegram bot via @BotFather

## Phase 1: Round 1 Abstract and Pitch

- [ ] **DOC** Collect 3–5 verified statistics with sources (CGWB, Atal Bhujal, NITI Aayog, etc.)
- [ ] **DOC** Complete the "existing solutions and gaps" table with real citations
- [ ] **DOC** Finalise problem statement (one sentence + one paragraph)
- [ ] **HW** Draw block diagram and wiring diagram (draw.io/Excalidraw → `docs/img/`)
- [ ] **FW** Record a 30–60 s video of any sensor working (even only the ultrasonic on a bucket)
- [ ] **CL** Screenshot of a live ThingSpeak chart with real data
- [ ] **DOC** Build slides from `presentation.md`
- [ ] **ALL** Rehearse 10-minute pitch 3×, timed
- [ ] **ALL** Prepare answers to the Q&A list in `presentation.md`

## Phase 2: Bench Prototype (Round 2, Week 1)

- [ ] **FW** DS18B20 reading on GPIO4 with 4.7 kΩ pull-up
- [ ] **FW** JSN-SR04T distance with median filter; verify against ruler at 30, 50, 100, 200 cm
- [ ] **FW** TDS reading via `analogReadMilliVolts`; compare with a handheld TDS meter
- [ ] **FW** Turbidity reading through voltage divider
- [ ] **FW** Battery voltage via divider
- [ ] **HW** Build a **test rig**: transparent tank/bucket with a drain valve + a ruler stuck on the side
- [ ] **HW** Mount ultrasonic facing straight down, in a stilling pipe if needed
- [ ] **DOC** Start the report skeleton (copy section headings from `goal.md` judging table)

## Phase 3: Connected Prototype (Week 2)

- [ ] **FW** WiFi connect with timeout + retry
- [ ] **FW** Upload to ThingSpeak (fields 1–8)
- [ ] **CL** Dashboard charts: level, TDS, temp, clarity, battery
- [ ] **FW** Telegram alert function working end-to-end
- [ ] **FW** Alert cooldown logic (no spam)
- [ ] **HW** Calibration session (TDS with known solutions, ultrasonic vs tape, turbidity in clear water)
- [ ] **DOC** Document calibration results in `docs/calibration.md` (real numbers!)

## Phase 4: Field-Ready (Week 3)

- [ ] **HW** Solar panel + TP4056 + 18650 + boost converter wired and tested
- [ ] **HW** Measure real current: active and deep-sleep (multimeter/USB power meter)
- [ ] **FW** Deep sleep cycle every 30 min; RTC history buffer
- [ ] **FW** Depletion rate (cm/day) + days-to-dry
- [ ] **CL** `analysis/trend.py` working against the live channel
- [ ] **HW** Waterproof enclosure (IP65 box + cable glands), conformal coat/hot glue on joints
- [ ] **ALL** 24-hour continuous soak test; log failures

## Phase 5: Polish + Report (Week 4)

- [ ] **ALL** Demo rehearsal: drain/refill the tank, trigger alert on cue (3 clean runs in a row)
- [ ] **DOC** Finish report: solution, prototype, cost, implementation plan, scalability, impact
- [ ] **DOC** Update `docs/cost-and-scalability.md` with **actual** prices paid
- [ ] **CL** Make the dashboard public/read-only for judges (QR code on slide)
- [ ] **FW** Tag `v1.0` release on GitHub, clean up code and comments
- [ ] **HW** Pack: spare sensors, spare ESP32, power bank, extension cord, tools, backup video of demo
- [ ] **ALL** Prepare a fallback plan: pre-recorded demo video if WiFi at the venue fails

## Stretch (only if all above is done)

- [ ] LoRa node → gateway
- [ ] Hindi/Marathi alerts
- [ ] Rain gauge
- [ ] Multi-node map

---

## Demo-Day Checklist

- [ ] Hotspot (phone) configured with the **same SSID/password as in `config.h`**; WiFi at big venues is unreliable
- [ ] Battery fully charged; USB backup power ready
- [ ] Tank filled to start level; marked ruler visible to the judges
- [ ] Telegram chat open on a phone, volume on
- [ ] Dashboard open on a laptop, auto-refresh on
- [ ] Printed one-page summary + report
- [ ] Offline demo video on laptop and phone

---

## Known Risks (watch these)

| Risk | Mitigation |
|---|---|
| Sensor shipping delays | Order **now**; buy a spare ultrasonic and TDS sensor |
| ESP32 ADC noise | Median of 30 samples, use `analogReadMilliVolts`, 100 nF cap at the ADC pin |
| Venue WiFi fails | Phone hotspot + pre-recorded video |
| Ultrasonic false echoes | Stilling pipe/tube, aim straight down, ignore readings < 25 cm |
| Run out of time | Freeze scope after Phase 3; stretch goals are optional |
