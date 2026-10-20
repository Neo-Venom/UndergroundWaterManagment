/*
 * ============================================================================
 * TEST 05: Analog Turbidity (Relative Clarity) Sensor Test
 * Component: SEN0189 / Optical Turbidity Sensor Module & Probe
 *
 * PURPOSE:
 * Measures light transmission through liquid to detect suspended particles,
 * siltation, or sediment intrusion into the well.
 *
 * HARDWARE CONNECTIONS:
 * - VCC  ---> 5V (Sensor operates at 5V)
 * - GND  ---> ESP32 GND
 * - AOUT ---> Voltage Divider to ESP32 GPIO 35:
 *             AOUT ---> 10kΩ Resistor ---> GPIO 35
 *                                     ---> 20kΩ Resistor ---> GND
 *             (Converts 4.5V module signal down to safe ~3.0V)
 * - Switch on adapter module: Set to "A" (Analog mode), NOT "D" (Digital mode).
 *
 * EXPECTED OUTPUT ON SERIAL MONITOR (115200 Baud):
 * ------------------------------------------------
 * In Clean Drinking Water:
 * [TURBIDITY] Pin: 2.73 V | Module Out: 4.10 V | Clarity: 100 % | Status: CRYSTAL CLEAR
 *
 * In Slightly Turbid Water (stir with a drop of milk or silt):
 * [TURBIDITY] Pin: 2.15 V | Module Out: 3.22 V | Clarity: 78 % | Status: ACCEPTABLE
 *
 * In Heavy Muddy/Dirty Water:
 * [TURBIDITY] Pin: 1.20 V | Module Out: 1.80 V | Clarity: 44 % | Status: HEAVY SILTATION ALERT!
 * ============================================================================
 */

#include <Arduino.h>

#define PIN_TURB_ADC 35 // ADC1_CH7

// Divider ratio: (10k + 20k) / 20k = 1.5
const float VOLTAGE_DIVIDER_RATIO = 1.5f;

// Reference voltage produced by your specific module in clean water
// (Calibrate this with clean tap water on your desk)
const float CLEAN_WATER_VOLTAGE = 4.10f;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println(F("\n======================================================="));
  Serial.println(F("✅ Underground Water Management Test 05: Analog Turbidity Sensor"));
  Serial.println(F("======================================================="));
  Serial.println(F("[INFO] Ensure the toggle switch on the small adapter board is set to 'A' (Analog)."));
  Serial.println(F("[INFO] Shield the probe from direct overhead ambient room lighting for stable reads."));
  Serial.println(F("-------------------------------------------------------"));

  analogSetAttenuation(ADC_11db);
}

void loop() {
  // Average 20 analog reads to eliminate ripple
  float sumVolts = 0;
  for (int i = 0; i < 20; i++) {
    sumVolts += (float)analogReadMilliVolts(PIN_TURB_ADC) / 1000.0f;
    delay(5);
  }
  float pinVoltage = sumVolts / 20.0f;

  // Calculate actual module output before voltage divider
  float moduleVoltage = pinVoltage * VOLTAGE_DIVIDER_RATIO;

  // Calculate relative clarity percentage
  float clarityPct = (moduleVoltage / CLEAN_WATER_VOLTAGE) * 100.0f;
  if (clarityPct > 100.0f) clarityPct = 100.0f;
  if (clarityPct < 0.0f)   clarityPct = 0.0f;

  Serial.printf("[TURBIDITY] Pin: %.2f V | Sensor Out: %.2f V | Clarity: %.1f %% | ",
                pinVoltage, moduleVoltage, clarityPct);

  if (clarityPct >= 85.0f) {
    Serial.println(F("Status: CRYSTAL CLEAR"));
  } else if (clarityPct >= 70.0f) {
    Serial.println(F("Status: NORMAL POTABLE"));
  } else if (clarityPct >= 50.0f) {
    Serial.println(F("Status: ⚠️  MODERATE TURBIDITY"));
  } else {
    Serial.println(F("Status: 🚨 HEAVY SILTATION / CONTAMINATED"));
  }

  delay(1500);
}
