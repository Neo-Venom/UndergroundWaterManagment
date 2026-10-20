/*
 * ============================================================================
 * TEST 07: Submersible Pump Cutoff Relay Module Test
 * Component: 5V Single Channel Relay Module (Active LOW)
 *
 * PURPOSE:
 * Tests the electromechanical relay that cuts power to the farmer's
 * submersible pump when groundwater reaches the critical dry-run threshold.
 *
 * HARDWARE CONNECTIONS:
 * - VCC ---> 5V (from MT3608 or 5V rail)
 * - GND ---> ESP32 GND
 * - IN  ---> ESP32 GPIO 19
 *
 * HIGH VOLTAGE / LOAD WIRING (For live pump / demo bulb):
 * - COM terminal ---> Live power wire from pump power supply
 * - NO (Normally Open) ---> To Submersible Pump motor
 *
 * (When relay is ENERGIZED, COM connects to NO, allowing current to flow to pump.
 *  When water drops too low, relay TRIPS OFF to NO disconnect, cutting power!)
 *
 * EXPECTED OUTPUT ON SERIAL MONITOR (115200 Baud):
 * ------------------------------------------------
 * [RELAY] State: ENERGIZED (PUMP RUNNING)  | GPIO 19: LOW  | Audible 'CLICK' heard!
 * ... wait 3 seconds ...
 * [RELAY] State: TRIPPED (DRY-RUN CUTOFF) | GPIO 19: HIGH | Audible 'CLICK' heard!
 * ============================================================================
 */

#include <Arduino.h>

#define PIN_PUMP_RELAY 19
#define PIN_STATUS_LED 2

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println(F("\n======================================================="));
  Serial.println(F("✅ Underground Water Management Test 07: Pump Cutoff Relay"));
  Serial.println(F("======================================================="));
  Serial.println(F("[INFO] Most relay modules are ACTIVE LOW:"));
  Serial.println(F("       GPIO LOW  = Relay ON  (LED on module lights, contacts close)"));
  Serial.println(F("       GPIO HIGH = Relay OFF (Cutoff active, pump disconnected)"));
  Serial.println(F("[INFO] Listen for the mechanical CLICK every 3 seconds!"));
  Serial.println(F("-------------------------------------------------------"));

  pinMode(PIN_PUMP_RELAY, OUTPUT);
  pinMode(PIN_STATUS_LED, OUTPUT);
}

void loop() {
  // 1. Energize Relay (Pump Allowed to Run)
  digitalWrite(PIN_PUMP_RELAY, LOW); // Active LOW
  digitalWrite(PIN_STATUS_LED, HIGH);
  Serial.println(F("[RELAY] State: ENERGIZED (PUMP RUNNING)  | GPIO19: LOW  [Click!]"));
  delay(3000);

  // 2. Trip Relay (Dry-run Emergency Cutoff)
  digitalWrite(PIN_PUMP_RELAY, HIGH);
  digitalWrite(PIN_STATUS_LED, LOW);
  Serial.println(F("[RELAY] State: TRIPPED (DRY-RUN CUTOFF) | GPIO19: HIGH [Click!]"));
  delay(3000);
}
