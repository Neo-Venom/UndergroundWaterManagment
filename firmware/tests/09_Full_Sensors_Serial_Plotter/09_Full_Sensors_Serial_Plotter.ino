/*
 * ============================================================================
 * TEST 09: Full Multi-Sensor Bench Test with Arduino Serial Plotter
 * Components: Ultrasonic + TDS + Turbidity + DS18B20 + Battery Divider
 *
 * PURPOSE:
 * Reads ALL sensors simultaneously without requiring WiFi or Telegram.
 * Formatted directly for the Arduino IDE Serial Plotter (Tools -> Serial Plotter)
 * to visually inspect sensor signals, noise, and response in real time!
 *
 * HOW TO VIEW LIVE GRAPHS:
 * 1. Upload this sketch to your ESP32.
 * 2. In Arduino IDE, go to: Tools -> Serial Plotter (Ctrl+Shift+L).
 * 3. Set the baud rate in the bottom right corner to 115200.
 * 4. You will see 6 live colorful curves:
 *    - Blue:   Distance (cm)
 *    - Orange: Water_Column (cm)
 *    - Green:  TDS (ppm)
 *    - Red:    Clarity (%)
 *    - Purple: Temperature (°C)
 *    - Brown:  Battery (V x 10 for plot scale)
 * ============================================================================
 */

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Pin Definitions
#define PIN_DS18B20   4
#define PIN_TRIG      26
#define PIN_ECHO      27
#define PIN_TDS_ADC   34
#define PIN_TDS_PWR   25
#define PIN_TURB_ADC  35
#define PIN_BATT_ADC  32

const float TOTAL_DEPTH_CM = 100.0f;
const float SENSOR_OFFSET_CM = 15.0f;

OneWire oneWire(PIN_DS18B20);
DallasTemperature tempSensor(&oneWire);

// Filter helper
float getMedian(float arr[], int n) {
  float temp[16];
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

  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_TDS_PWR, OUTPUT);
  digitalWrite(PIN_TRIG, LOW);
  digitalWrite(PIN_TDS_PWR, LOW);

  analogSetAttenuation(ADC_11db);
  tempSensor.begin();

  Serial.println(F("\n[INIT] Ready. Open Tools -> Serial Plotter (115200 Baud) to view live graph curves!"));
}

void loop() {
  // 1. Temperature
  tempSensor.requestTemperatures();
  float tempC = tempSensor.getTempCByIndex(0);
  if (tempC < -20.0f || tempC > 80.0f) tempC = 25.0f;

  // 2. Ultrasonic
  float speedSound = 331.3f + 0.606f * tempC;
  float speedCmPerUs = (speedSound * 100.0f) / 1000000.0f;
  float distSamples[5];
  int validDist = 0;

  for (int i = 0; i < 5; i++) {
    digitalWrite(PIN_TRIG, LOW);
    delayMicroseconds(4);
    digitalWrite(PIN_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_TRIG, LOW);

    unsigned long dur = pulseIn(PIN_ECHO, HIGH, 35000);
    if (dur > 0) {
      float d = (dur * speedCmPerUs) / 2.0f;
      if (d >= 20.0f && d <= 600.0f) {
        distSamples[validDist++] = d;
      }
    }
    delay(30);
  }
  float distance = (validDist > 0) ? getMedian(distSamples, validDist) : 50.0f;
  float waterColumn = TOTAL_DEPTH_CM - (distance + SENSOR_OFFSET_CM);
  if (waterColumn < 0) waterColumn = 0;

  // 3. TDS (Power-gated)
  digitalWrite(PIN_TDS_PWR, HIGH);
  delay(40);
  float sumTds = 0;
  for (int i = 0; i < 15; i++) {
    sumTds += (float)analogReadMilliVolts(PIN_TDS_ADC) / 1000.0f;
    delay(2);
  }
  digitalWrite(PIN_TDS_PWR, LOW);
  float vTdsRaw = sumTds / 15.0f;
  float k = 1.0f + 0.02f * (tempC - 25.0f);
  float vTdsComp = vTdsRaw / k;
  float tdsPpm = (133.42f * pow(vTdsComp, 3) - 255.86f * pow(vTdsComp, 2) + 857.39f * vTdsComp) * 0.5f;
  if (tdsPpm < 0) tdsPpm = 0;

  // 4. Turbidity
  float sumTurb = 0;
  for (int i = 0; i < 15; i++) {
    sumTurb += (float)analogReadMilliVolts(PIN_TURB_ADC) / 1000.0f;
    delay(2);
  }
  float vTurbModule = (sumTurb / 15.0f) * 1.5f; // x1.5 divider
  float clarityPct = (vTurbModule / 4.10f) * 100.0f;
  if (clarityPct > 100) clarityPct = 100;
  if (clarityPct < 0) clarityPct = 0;

  // 5. Battery
  float sumBatt = 0;
  for (int i = 0; i < 15; i++) {
    sumBatt += (float)analogReadMilliVolts(PIN_BATT_ADC) / 1000.0f;
    delay(2);
  }
  float battVolts = (sumBatt / 15.0f) * 2.0f; // x2 divider

  // STREAM TO ARDUINO SERIAL PLOTTER:
  // Note: Battery is scaled by 10 (e.g. 3.9V -> 39.0) so it's easily visible alongside 0-100 values!
  Serial.printf("Distance_cm:%.1f,Water_Column_cm:%.1f,TDS_ppm:%.1f,Clarity_pct:%.1f,Temp_C:%.1f,Battery_Vx10:%.1f\n",
                distance, waterColumn, tdsPpm, clarityPct, tempC, battVolts * 10.0f);

  delay(400);
}
