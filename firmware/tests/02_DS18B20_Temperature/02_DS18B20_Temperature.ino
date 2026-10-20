/*
 * ============================================================================
 * TEST 02: DS18B20 Waterproof Temperature Sensor Test
 * Component: DS18B20 1-Wire Digital Probe
 *
 * PURPOSE:
 * Verifies 1-Wire communication, checks the 64-bit unique ROM address,
 * and reads water/ambient temperature in Celsius and Fahrenheit.
 *
 * REQUIRED LIBRARIES (Install via Arduino Library Manager):
 * 1. "OneWire" by Jim Studt, Tom Pollard, Robin Bliss, Paul Stoffregen
 * 2. "DallasTemperature" by Miles Burton
 *
 * HARDWARE CONNECTIONS:
 * - Red Wire   (VCC)  ---> ESP32 3.3V
 * - Black Wire (GND)  ---> ESP32 GND
 * - Yellow Wire (DATA)---> ESP32 GPIO 4
 * - PULL-UP RESISTOR: Connect a 4.7kΩ resistor between Yellow (DATA) and 3.3V!
 *   (Crucial: Without this 4.7k resistor, the sensor will NOT be found).
 *
 * EXPECTED OUTPUT ON SERIAL MONITOR (115200 Baud):
 * ------------------------------------------------
 * [SETUP] Locating 1-Wire devices...
 * [SETUP] Found 1 device(s) on the bus!
 * [SETUP] Device 0 ROM Address: 28 FF 64 1E 82 16 03 A1
 * [READ] Water Temperature: 26.50 °C (79.70 °F) | Status: OK
 * [READ] Water Temperature: 26.56 °C (79.81 °F) | Status: OK
 *
 * If disconnected or missing pull-up resistor:
 * [READ] Error: Temperature reading invalid (-127.00 °C). Check wiring & 4.7k pull-up!
 * ============================================================================
 */

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define PIN_ONE_WIRE_BUS 4

OneWire oneWire(PIN_ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

DeviceAddress tempDeviceAddress;

void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
    if (i < 7) Serial.print(" ");
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println(F("\n======================================================="));
  Serial.println(F("✅ Underground Water Management Test 02: DS18B20 Temperature"));
  Serial.println(F("======================================================="));

  sensors.begin();
  int deviceCount = sensors.getDeviceCount();
  Serial.printf("[SETUP] Found %d 1-Wire device(s) on GPIO %d\n", deviceCount, PIN_ONE_WIRE_BUS);

  if (deviceCount == 0) {
    Serial.println(F("[ERROR] No DS18B20 found!"));
    Serial.println(F("        1. Did you connect a 4.7kΩ pull-up between GPIO4 and 3.3V?"));
    Serial.println(F("        2. Are Red (3.3V), Black (GND), and Yellow (GPIO4) correct?"));
  } else {
    for (int i = 0; i < deviceCount; i++) {
      if (sensors.getAddress(tempDeviceAddress, i)) {
        Serial.printf("[SETUP] Device %d ROM Address: ", i);
        printAddress(tempDeviceAddress);
        sensors.setResolution(tempDeviceAddress, 11); // 11-bit resolution (~0.125 °C precision, 375ms conversion)
      }
    }
  }
  Serial.println(F("-------------------------------------------------------"));
}

void loop() {
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);

  if (tempC == DEVICE_DISCONNECTED_C || tempC < -50.0f) {
    Serial.println(F("[READ] Error: Temperature reading invalid (-127.00 °C). Check wiring & 4.7k pull-up!"));
  } else {
    float tempF = DallasTemperature::toFahrenheit(tempC);
    Serial.printf("[READ] Water Temperature: %.2f °C (%.2f °F) | Status: OK\n", tempC, tempF);
  }

  delay(1500);
}
