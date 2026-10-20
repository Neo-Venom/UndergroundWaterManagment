/*
 * ============================================================================
 * TEST 03: JSN-SR04T Waterproof Ultrasonic Level Sensor Test
 * Component: JSN-SR04T Waterproof Transducer & Driver Board
 *
 * PURPOSE:
 * Measures distance from the sensor down to an obstacle/water surface.
 * Demonstrates 10-ping median filtering to eliminate acoustic bounce spikes.
 *
 * HARDWARE CONNECTIONS:
 * - VCC  ---> 5V (from MT3608 or USB 5V; or 3.3V if using JSN-SR04T V3.0)
 * - GND  ---> ESP32 GND
 * - TRIG ---> ESP32 GPIO 26
 * - ECHO ---> Voltage Divider to ESP32 GPIO 27:
 *             ECHO pin ---> 1kΩ Resistor ---> GPIO 27
 *                                         ---> 2kΩ Resistor ---> GND
 *             (Converts 5V Echo pulse down to safe ~3.3V)
 *
 * CRITICAL PHYSICS & GOTCHAS:
 * 1. BLIND ZONE: JSN-SR04T has a blind zone of approx 20–25 cm.
 *    Any object placed closer than 25 cm will read either 0 or ~25 cm!
 * 2. BEAM ANGLE: The ultrasonic acoustic cone is ~45°–70°.
 *    Ensure the probe points directly down at water without hitting close pipe walls.
 *
 * EXPECTED OUTPUT ON SERIAL MONITOR (115200 Baud):
 * ------------------------------------------------
 * [PING] Echo: 2912 µs | Raw Dist: 50.1 cm (19.7 in) | Median (10 pings): 50.2 cm
 * [PING] Echo: 2915 µs | Raw Dist: 50.2 cm (19.8 in) | Median (10 pings): 50.2 cm
 * [PING] Echo: 2908 µs | Raw Dist: 50.0 cm (19.7 in) | Median (10 pings): 50.1 cm
 *
 * If too close (<20 cm) or pointing into open air:
 * [PING] Warning: Echo timeout or object inside blind zone (<25cm)!
 * ============================================================================
 */

#include <Arduino.h>

#define PIN_TRIG 26
#define PIN_ECHO 27

// Speed of sound at standard 25°C in cm/us:
// (331.3 + 0.606 * 25) * 100 / 1000000 = 0.034645 cm/us
const float SPEED_OF_SOUND_CM_US = 0.034645f;

float samples[10];

float getMedian(float arr[], int n) {
  float temp[10];
  for (int i = 0; i < n; i++) temp[i] = arr[i];
  for (int i = 1; i < n; i++) {
    float key = temp[i];
    int j = i - 1;
    while (j >= 0 && temp[j] > key) {
      temp[j + 1] = temp[j];
      j--;
    }
    temp[j + 1] = key;
  }
  return temp[n / 2];
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println(F("\n======================================================="));
  Serial.println(F("✅ Underground Water Management Test 03: JSN-SR04T Ultrasonic"));
  Serial.println(F("======================================================="));
  Serial.println(F("[INFO] Ultrasonic Blind Zone: ~20-25 cm. Keep targets > 25 cm away!"));
  Serial.println(F("[INFO] ECHO pin MUST be routed through 1k/2k voltage divider to GPIO27."));
  Serial.println(F("-------------------------------------------------------"));

  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  digitalWrite(PIN_TRIG, LOW);
}

void loop() {
  int validCount = 0;
  float latestRawDist = 0.0f;
  unsigned long latestDuration = 0;

  for (int i = 0; i < 10; i++) {
    // 10 microsecond trigger pulse
    digitalWrite(PIN_TRIG, LOW);
    delayMicroseconds(4);
    digitalWrite(PIN_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_TRIG, LOW);

    // Read echo pulse with 35ms timeout (~600 cm max range)
    unsigned long duration = pulseIn(PIN_ECHO, HIGH, 35000);

    if (duration > 0) {
      float dist = (duration * SPEED_OF_SOUND_CM_US) / 2.0f;
      if (dist >= 20.0f && dist <= 600.0f) {
        samples[validCount++] = dist;
        latestRawDist = dist;
        latestDuration = duration;
      }
    }
    delay(50); // Inter-ping settling delay
  }

  if (validCount >= 4) {
    float medianDist = getMedian(samples, validCount);
    float distInches = medianDist / 2.54f;
    Serial.printf("[PING] Echo: %lu µs | Raw: %.1f cm | Median (%d pings): %.1f cm (%.1f in)\n",
                  latestDuration, latestRawDist, validCount, medianDist, distInches);
  } else {
    Serial.println(F("[PING] Warning: Out of range, no echo, or object inside blind zone (<25cm)!"));
  }

  delay(600);
}
