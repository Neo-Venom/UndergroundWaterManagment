/*
 * ============================================================================
 * TEST 06: Solar Li-ion Battery Voltage Divider Test
 * Component: 18650 Li-ion Cell + 100kΩ/100kΩ Voltage Divider
 *
 * PURPOSE:
 * Measures remaining solar battery charge to predict power autonomy,
 * diagnose low charge, and prevent battery over-discharge.
 *
 * HARDWARE CONNECTIONS:
 * - Battery+ (or TP4056 OUT+) ---> 100kΩ Resistor ---> GPIO 32
 *                                                   ---> 100kΩ Resistor ---> GND
 * - Battery- / GND            ---> ESP32 GND (Common Ground)
 * - Capacitor: Put a 100nF capacitor between GPIO 32 and GND to smooth ADC noise.
 *
 * VOLTAGE REFERENCE GUIDE (18650 Li-ion):
 * - 4.20 V : 100% (Fully Charged from Solar Panel)
 * - 3.85 V : ~65% (Nominal Voltage)
 * - 3.60 V : ~25% (Low Power Alert threshold)
 * - 3.30 V : ~0%  (Cutoff threshold)
 *
 * EXPECTED OUTPUT ON SERIAL MONITOR (115200 Baud):
 * ------------------------------------------------
 * [BATTERY] ADC Pin: 1.95 V | Battery: 3.90 V | Charge: 67% | Status: HEALTHY
 * [BATTERY] ADC Pin: 2.05 V | Battery: 4.10 V | Charge: 89% | Status: CHARGED (SOLAR ACTIVE)
 *
 * If battery is very low:
 * [BATTERY] ADC Pin: 1.70 V | Battery: 3.40 V | Charge: 11% | Status: ⚠️ LOW BATTERY WARNING
 * ============================================================================
 */

#include <Arduino.h>

#define PIN_BATT_ADC 32 // ADC1_CH4

// 100k + 100k divider divides voltage by 2:
const float DIVIDER_RATIO = 2.0f;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println(F("\n======================================================="));
  Serial.println(F("✅ Underground Water Management Test 06: Battery Voltage Sense"));
  Serial.println(F("======================================================="));
  Serial.println(F("[INFO] Reads Battery+ via two 100kΩ resistors."));
  Serial.println(F("[INFO] Never connect battery directly to ESP32 without the divider!"));
  Serial.println(F("-------------------------------------------------------"));

  analogSetAttenuation(ADC_11db);
}

void loop() {
  float sumVolts = 0;
  for (int i = 0; i < 20; i++) {
    sumVolts += (float)analogReadMilliVolts(PIN_BATT_ADC) / 1000.0f;
    delay(4);
  }
  float pinVoltage = sumVolts / 20.0f;
  float batteryVoltage = pinVoltage * DIVIDER_RATIO;

  // Approximate Li-ion state of charge: 3.3V (0%) to 4.2V (100%)
  float pct = ((batteryVoltage - 3.30f) / (4.20f - 3.30f)) * 100.0f;
  if (pct > 100.0f) pct = 100.0f;
  if (pct < 0.0f)   pct = 0.0f;

  Serial.printf("[BATTERY] ADC Pin: %.2f V | Battery: %.2f V | Charge: %3.0f%% | ",
                pinVoltage, batteryVoltage, pct);

  if (batteryVoltage >= 4.05f) {
    Serial.println(F("Status: CHARGED (SOLAR ACTIVE)"));
  } else if (batteryVoltage >= 3.65f) {
    Serial.println(F("Status: HEALTHY AUTONOMY"));
  } else if (batteryVoltage >= 3.45f) {
    Serial.println(F("Status: ⚠️  LOW BATTERY WARNING"));
  } else {
    Serial.println(F("Status: 🚨 CRITICAL DISCHARGE - SLEEP NOW"));
  }

  delay(2000);
}
