# Problem Statement

> **Before you cite anything below:** the statistics marked **[verify]** are widely quoted figures written from memory. Find the original source (CGWB, Ministry of Jal Shakti, NITI Aayog, World Bank), confirm the number and year, and put the citation on your slide and in your report. Judges notice unsourced numbers.

## 1. Context

Groundwater is India's most important water source.

- India is the **largest user of groundwater in the world**, extracting roughly a quarter of the global total **[verify]**.
- Groundwater supplies the large majority of rural drinking water and a major share of irrigation **[verify exact %]**.
- In many regions, extraction exceeds natural recharge, and water tables are falling year after year (Punjab, Haryana, Rajasthan, parts of Gujarat, Karnataka, Maharashtra, Tamil Nadu) **[verify with latest CGWB Dynamic Ground Water Resources assessment]**.
- Quality problems compound quantity problems: salinity, fluoride, nitrate, arsenic and iron are documented in many districts **[verify with CGWB quality reports]**.

## 2. The Specific Problem We Address

> **Communities and authorities lack timely, affordable, village-level visibility into how fast groundwater is depleting and whether it is becoming unsafe, so they act after the damage is done.**

Broken into four sub-problems:

1. **Sparse and infrequent data.** Official observation wells are few relative to the number of wells people actually use, and many are measured manually a few times a year **[verify]**.
2. **Cost barrier.** Professional telemetry-based water-level loggers are accurate but expensive, so they cannot be deployed at village density.
3. **Quality and quantity are siloed.** Level and quality are rarely measured by the same device at the same time.
4. **No actionable loop.** Even when data exists, it seldom reaches the farmer or panchayat as a clear warning ("X days left" / "don't drink").

## 3. Who Is Affected

| Group | Pain |
|---|---|
| Small and marginal farmers | Borewell fails mid-season; crop loss; deeper drilling debt |
| Rural households | Drinking water gets saltier or unsafe without notice |
| Gram panchayats | No data to plan water use, tanker needs, or recharge works |
| State/district water authorities | Coarse data; slow response; hard to verify programme outcomes |
| Programmes like Atal Bhujal Yojana | Community-led water budgeting needs cheap, frequent monitoring **[verify programme details]** |

## 4. Existing Solutions and Their Gaps

| Existing approach | What it does well | Gap |
|---|---|---|
| **Manual dip-well measurement** (CGWB network, panchayat volunteers) | Cheap, simple, accurate when done carefully | Infrequent, human error, delayed reporting, no quality data, no alerts |
| **Digital Water Level Recorders (DWLR) / piezometer loggers** (e.g., under national hydrology programmes) **[verify]** | High accuracy, telemetry | Expensive, limited number of stations, level only, not designed for every village well |
| **Commercial IoT water loggers** | Polished products, multi-parameter options | High cost per node, vendor lock-in, not affordable at scale for communities |
| **Generic IoT dashboards** (typical hackathon/academic projects) | Show live data | Only visualise; no depletion prediction, no actionable alerts, little power/field design |
| **Government schemes and community water budgeting** | Awareness, governance, funding | Need **cheap, continuous data** to work well |

### Our gap statement
There is no widely deployed, **low-cost, solar-powered** node that combines **level + basic quality**, **predicts** depletion, and **alerts** the people who can act, at a price low enough for dense village-level coverage.

## 5. Why Now

- ESP32-class microcontrollers and low-cost sensors make sub-₹4k nodes feasible.
- Telegram/SMS/WhatsApp-style channels reach users without new apps.
- Government programmes are already investing in community-led groundwater management and need monitoring tools.

## 6. Scope

**In scope (hackathon prototype)**
- Single node measuring depth to water, TDS, temperature, turbidity, battery
- Depletion rate + days-to-dry estimate
- Cloud logging, dashboard, Telegram alerts
- Solar + deep-sleep operation

**Out of scope (documented as future work)**
- Identifying specific contaminants (fluoride, arsenic, nitrate)
- Custom PCB and certified enclosure
- Full multi-village backend and mobile app

## 7. Success Criteria

See [`../goal.md`](../goal.md) for targets (accuracy, autonomy, cost, alert latency).

## 8. Research To-Do (for DOC lead)

- [ ] Latest CGWB groundwater assessment: % of blocks over-exploited/critical/semi-critical
- [ ] Atal Bhujal Yojana: states covered, budget, objectives
- [ ] NITI Aayog Composite Water Management Index findings
- [ ] CGWB observation well counts and measurement frequency
- [ ] Cost of a DWLR/commercial logger (get 1–2 quotes or published tender values)
- [ ] BIS IS 10500 drinking water limits for TDS and turbidity
- [ ] One local example (a district or village near your college with a documented groundwater problem)
- [ ] Add a footnote/citation list at the end of this file
