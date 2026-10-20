/*
 * ============================================================================
 * TEST 01: ESP32 Hardware Verification & Serial Communication
 * Component: ESP32 DevKit (Onboard LED on GPIO 2)
 *
 * PURPOSE:
 * Verifies that your computer can compile code, upload to the ESP32,
 * open the Serial Monitor at 115200 baud, and toggle GPIO pins.
 *
 * HARDWARE CONNECTIONS:
 * - Just connect your ESP32 to your PC using a micro-USB / Type-C cable.
 *
 * EXPECTED OUTPUT ON SERIAL MONITOR (115200 Baud):
 * ------------------------------------------------
 * [BOOT] ESP32 Chip Model: ESP32-D0WDQ6 (Revision 1)
 * [BOOT] CPU Frequency: 240 MHz
 * [BOOT] Flash Chip Size: 4194304 bytes (4MB)
 * [LOOP] Cycle: 1 | Onboard LED: ON
 * [LOOP] Cycle: 1 | Onboard LED: OFF
 * [LOOP] Cycle: 2 | Onboard LED: ON
 * ...
 * ============================================================================
 */

#include <Arduino.h>

#define PIN_LED 2 // Built-in blue LED on most ESP32 DevKit boards

unsigned long cycleCount = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(PIN_LED, OUTPUT);

  Serial.println(F("\n======================================================="));
  Serial.println(F("✅ Underground Water Management Test 01: ESP32 Core & Serial"));
  Serial.println(F("======================================================="));
  Serial.printf("[BOOT] ESP32 Chip Model: %s (Rev %d)\n", ESP.getChipModel(), ESP.getChipRevision());
  Serial.printf("[BOOT] CPU Frequency: %d MHz\n", ESP.getCpuFreqMHz());
  Serial.printf("[BOOT] Flash Chip Size: %u bytes\n", ESP.getFlashChipSize());
  Serial.printf("[BOOT] Free Heap Memory: %u bytes\n", ESP.getFreeHeap());
  Serial.println(F("[BOOT] Ready! Starting LED blink cycle...\n"));
}

void loop() {
  cycleCount++;

  // LED ON
  digitalWrite(PIN_LED, HIGH);
  Serial.printf("[LOOP] Cycle: %lu | Onboard LED: ON\n", cycleCount);
  delay(1000);

  // LED OFF
  digitalWrite(PIN_LED, LOW);
  Serial.printf("[LOOP] Cycle: %lu | Onboard LED: OFF\n", cycleCount);
  delay(1000);
}
