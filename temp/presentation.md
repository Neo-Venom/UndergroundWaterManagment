# 🎤 Presentation Guide

Covers **Round 1** (10-minute pitch at the zonal centre) and **Round 2** (final at IIT Bombay: demo + report + panel Q&A).

---

# Part A: Round 1 Pitch (10 minutes)

**Rule of thumb:** 8 slides, one idea each, big visuals, almost no paragraphs. Judges decide in the first 2 minutes whether the problem is real and your solution is practical.

| # | Slide | Time | Content |
|---|---|---|---|
| 1 | Title + Team | 0:30 | Project name **Underground Water Management System**, tagline, team names/colleges, theme chosen |
| 2 | The Problem | 1:30 | Groundwater is depleting **and** contaminated; we find out too late |
| 3 | Existing Solutions & Gaps | 1:30 | Table: manual wells, DWLR, commercial loggers → gap = cost, density, no quality, no alerts |
| 4 | Our Solution | 1:30 | One-line + architecture diagram (sensors → ESP32 → cloud → alerts) |
| 5 | Technology | 1:30 | Sensor table, why each, solar + deep sleep, on-device trend |
| 6 | Demo / Proof | 1:00 | Video or photo of working bench prototype + live dashboard screenshot |
| 7 | Impact & Scalability | 1:30 | Cost per node, per-taluka cost, who benefits, path to national scale |
| 8 | Roadmap & Close | 0:30 | Pilot plan, what's next, one memorable closing line |

## Speaker Script (adapt to your voice)

### Slide 1: Title (0:30)
"We're Team ___. Our project is an **Autonomous Underground Water Management System**, a low-cost solar IoT node that watches both the *quantity* and *quality* of groundwater and warns villagers before a well runs dry or turns unsafe. We chose the Underground Water Management theme."

### Slide 2: Problem (1:30)
- Open with the human story: *"A farmer wakes up and the borewell is dry. The pump ran all night. Nobody warned him."*
- Then 2–3 **verified** statistics (see `docs/problem-statement.md`; put the source on the slide).
- Say the key insight: **"The problem isn't only that groundwater is falling. It's that we don't see it falling in time."**

### Slide 3: Gaps (1:30)
Show the table from `docs/problem-statement.md` §4. Land on:
"Accurate loggers exist, but they're too expensive to put in every village well. Manual measurement is too infrequent. And almost nothing watches water quality *and* level together, with alerts."

### Slide 4: Solution (1:30)
Show the diagram. Walk left to right in one breath:
"Sensors feed an ESP32. It wakes every 30 minutes, measures, computes the depletion trend, uploads to the cloud, and sends a Telegram alert if needed, then sleeps. A solar panel keeps it running forever."

### Slide 5: Technology (1:30)
Show the sensor table (what it measures and why). Emphasise three choices:
1. **Deep sleep + solar** → deployable anywhere, no wiring.
2. **On-device trend and alerts** → works even if the dashboard is down.
3. **Temperature compensation** → TDS and ultrasonic readings stay accurate.

### Slide 6: Proof (1:00)
Play a 20–30 s video: water drains from a tank, distance number rises on the dashboard, phone buzzes.
"This is not a concept; here's the bench prototype."

### Slide 7: Impact & Scale (1:30)
- "₹3–4k per node today; designed to reach ~₹2k at volume."
- "100 nodes can cover a taluka-level monitoring network at a fraction of conventional logger cost."
- Beneficiaries: farmers, gram panchayats, state water departments, programs like Atal Bhujal Yojana.
- Scale path: village → taluka → district → state dashboard; LoRa for no-WiFi areas.

### Slide 8: Close (0:30)
"Our system turns invisible groundwater into visible, actionable data. Cheap enough for every village well. Thank you."

---

# Part B: Round 2 Final

## Report outline (matches the required sections)

1. **Executive summary** (½ page)
2. **Problem and background** (from `docs/problem-statement.md`)
3. **Solution description** (from `docs/solution.md`)
4. **Prototype demonstration**: photos, wiring diagram, firmware overview, calibration results, test results
5. **Cost and market viability**: BOM with *actual* prices, per-node vs commercial, who pays, business model
6. **Implementation plan**: pilot steps, timeline, partners, maintenance
7. **Scalability plan**: LoRa, gateways, backend, cost at scale
8. **Impact assessment**: environmental, social, economic (with a stated methodology and assumptions)
9. **Risks and limitations** (be honest, it builds credibility)
10. **Appendix**: schematics, code repo link, data samples

## Live Demo Script (3–4 minutes)

**Setup:** transparent tank with a ruler, valve for draining, node mounted above, laptop with dashboard, phone with Telegram.

| Time | Action | What to say |
|---|---|---|
| 0:00 | Point at the node | "This is our Groundwater Monitoring Node: ESP32, ultrasonic level sensor, TDS probe, temperature, turbidity, solar-powered." |
| 0:30 | Show dashboard | "Here's live data: level, TDS, temperature, clarity, battery." |
| 1:00 | Open the valve; water drops | "I'm now simulating over-extraction. Watch the distance number rise." |
| 1:45 | Show depletion rate/days-to-dry | "The node computes the rate and predicts when water reaches the pump." |
| 2:15 | Dip probe in salty/murky water (pre-prepared cup) | "Now a contamination event. TDS and clarity change." |
| 2:45 | Phone buzzes with Telegram | "And the village gets the warning automatically." |
| 3:15 | Wrap up | "Total cost of this node: ₹___. Built for every village well." |

**Tips**
- Use a **demo-mode threshold** (e.g., `SLEEP_MINUTES 1` and a low alert threshold) so alerts fire within seconds. Say clearly that it's a demo configuration; real deployment uses 30 min.
- Pre-warm: do a full run-through 10 minutes before.
- Have a **backup video** ready.
- Bring a phone hotspot configured with the same SSID/password as in `config.h`.

---

# Part C: Judge Q&A Prep

| Question | Strong answer |
|---|---|
| **Why ultrasonic, not a pressure sensor?** | Non-contact, no corrosion, very cheap, easy to install on open wells. Pressure transducers are more accurate for deep/narrow borewells; that's our upgrade path (and we say so). |
| **TDS isn't a contamination test.** | Correct, and we say so. TDS and turbidity are low-cost *early-warning proxies* for salinity, ingress, siltation. They tell villagers "get this tested," not "this is arsenic." Future work: ion-selective probes. |
| **How accurate is it?** | Show your calibration table: ±2 cm level, ±10% TDS after calibration. Compare with a reference. |
| **What if there's no WiFi in the village?** | LoRa node-to-gateway (stretch goal); one gateway serves many nodes. Designed in; WiFi is the prototype transport. |
| **How does it run without power?** | Solar + 18650 + deep sleep (25 s awake per 30 min). Show measured current. |
| **Pumping causes level swings, so is the trend valid?** | We use a windowed fit locally and daily-minimum analysis in the cloud; it's a trend tool, not a snapshot. |
| **What's the business model?** | Sold or leased to panchayats/NGOs/state agencies; subscription for alerts/dashboard; fits government programs like Atal Bhujal Yojana that already fund monitoring. |
| **Cost vs. existing loggers?** | Show BOM; commercial telemetry loggers cost many times more (cite quotes). We trade some accuracy for 10× density. |
| **What about theft or damage?** | Low unit cost, tamper-evident box, community ownership of the data. |
| **Data privacy / who owns data?** | Village-level open data; node IDs map to wells, not households. |
| **Why is this better than just a dashboard?** | Prediction ("9 days left") and **action** (alerts to people who can act). |
| **What's your most important next step?** | A real-world pilot on 5–10 wells for one season. |
| **What did you learn?** | Be honest: calibration, ADC noise, sleep-current dominance, sensor fouling. |

---

# Part D: Presentation Tips

- **Open with a person, not a technology.**
- **Cite sources on slides** (small footer text). Verified facts win trust.
- Show **real data from your own prototype**, even if messy.
- **Own your limitations** before judges point them out.
- One number per slide beats ten.
- Practise out loud with a timer; cut anything that doesn't fit.
- Everyone on the team should speak at least once and know the whole system.
