/*
 * ============================================================================
 * TEST 04: Analog TDS Sensor & GPIO Power-Gating Test
 * Component: Gravity Analog TDS Sensor Module & Waterproof Probe
 *
 * PURPOSE:
 * Measures Electrical Conductivity (EC) converted to Total Dissolved Solids (ppm).
 * Demonstrates GPIO Power Gating on GPIO 25: powers the probe only when taking
 * a measurement, preventing continuous galvanic electrolysis & probe decay!
 *
 * HARDWARE CONNECTIONS:
 * - VCC  ---> ESP32 GPIO 25 (Switched 3.3V power gate)
 * - GND  ---> ESP32 GND
 * - AOUT ---> ESP32 GPIO 34 (ADC1_CH6 - Input only)
 * - Optional: 100nF ceramic capacitor between GPIO 34 and GND to suppress ADC ripple.
 *
 * EXPECTED OUTPUT ON SERIAL MONITOR (115200 Baud):
 * ------------------------------------------------
 * In Dry Air:
 * [TDS] Power ON (GPIO25 HIGH) -> Raw: 0.00 V | Comp: 0.00 V | TDS: 0 ppm | Power OFF
 *
 * In Regular Tap / Drinking Water:
 * [TDS] Power ON (GPIO25 HIGH) -> Raw: 0.58 V | Comp: 0.58 V | TDS: 235 ppm | Power OFF
 *
 * In Salt Water (add a pinch of table salt):
 * [TDS] Power ON (GPIO25 HIGH) -> Raw: 1.82 V | Comp: 1.82 V | TDS: 1140 ppm | Power OFF
 * ============================================================================
 */

#include <Arduino.h>

#define PIN_TDS_ADC 34 // Analog read pin (ADC1)
#define PIN_TDS_PWR 25 // Switched power gate pin

const float WATER_TEMP_C = 25.0f; // Assumed 25°C for standalone test
const float TDS_CAL_FACTOR = 1.0f;

float readTdsWithPowerGating() {
  // 1. Energize the probe
  digitalWrite(PIN_TDS_PWR, HIGH);
  delay(50); // Allow analog filter capacitor to settle

  // 2. Take 30 analog samples
  float sumVolts = 0;
  for (int i = 0; i < 30; i++) {
    sumVolts += (float)analogReadMilliVolts(PIN_TDS_ADC) / 1000.0f;
    delay(3);
  }
  float rawVoltage = sumVolts / 30.0f;

  // 3. Immediately cut power to prevent electrolysis
  digitalWrite(PIN_TDS_PWR, LOW);

  // 4. Temperature compensation (standard reference 25°C)
  float k = 1.0f + 0.02f * (WATER_TEMP_C - 25.0f);
  float compVoltage = rawVoltage / k;

  // 5. Standard Gravity 3rd order polynomial conversion
  float ppm = (133.42f * pow(compVoltage, 3) 
              - 255.86f * pow(compVoltage, 2) 
              + 857.39f * compVoltage) * 0.5f * TDS_CAL_FACTOR;

  if (ppm < 0.0f) ppm = 0.0f;

  Serial.printf("[TDS] Power ON -> Raw: %.2f V | Comp: %.2f V | TDS: %.0f ppm | Power OFF (Sleep)\n",
                rawVoltage, compVoltage, ppm);

  return ppm;
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println(F("\n======================================================="));
  Serial.println(F("✅ Underground Water Management Test 04: Analog TDS Sensor"));
  Serial.println(F("======================================================="));
  Serial.println(F("[INFO] Anti-Electrolysis Active: GPIO25 is toggled HIGH only during reading."));
  Serial.println(F("[INFO] Submerge probe in water to see PPM change. (Air = 0 ppm)."));
  Serial.println(F("-------------------------------------------------------"));

  pinMode(PIN_TDS_PWR, OUTPUT);
  digitalWrite(PIN_TDS_PWR, LOW);

  // Set ADC attenuation for 0-3.3V range
  analogSetAttenuation(ADC_11db);
}

void loop() {
  float tds = readTdsWithPowerGating();

  if (tds > 500.0f) {
    Serial.println(F("      ⚠️  WARNING: Exceeds BIS IS 10500 Acceptable Drinking Limit (500 ppm)!"));
  } else if (tds > 50.0f) {
    Serial.println(F("      ✅  PASS: Normal potable mineral range (50 - 500 ppm)."));
  } else {
    Serial.println(F("      ℹ️  INFO: Ultra-pure RO water or probe in open air."));
  }

  delay(2000);
}
