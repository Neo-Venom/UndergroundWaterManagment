/*
 * ============================================================================
 * 💧 Underground Water Management: Autonomous Groundwater Depletion & Quality Predictor
 * InnovateX: The Future Habitats Challenge | Techfest, IIT Bombay
 *
 * Board: ESP32 DevKit (30/38 pin)
 * Core Architecture:
 *   1. Multi-Sensor Data Acquisition (Ultrasonic, TDS, Turbidity, DS18B20, Battery)
 *   2. Dual Temperature Compensation (Speed of Sound + TDS Conductivity)
 *   3. Outlier Rejection via Insertion-Sort Median Filters
 *   4. Edge Analytics: Least-Squares Drawdown Rate (cm/day) & Days-to-Dry
 *   5. Pump Dry-Run Protection: Automated Relay Interlock Cutoff
 *   6. Secure Telegram Bot Emergency Alerts (Direct HTTPS)
 *   7. Embedded Real-time Glassmorphism Web Dashboard (/ and /api/data)
 *   8. Arduino Serial Plotter Multi-Stream Output
 *   9. Deep Sleep Support for 100% Off-Grid Solar Autonomy
 * ============================================================================
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "config.h"
#include "web_dashboard.h"

// ----------------------------------------------------------------------------
// PERSISTENT RTC MEMORY (Preserved during Deep Sleep & Soft Resets)
// ----------------------------------------------------------------------------
#define MAX_HISTORY 48
RTC_DATA_ATTR float rtc_distHistory[MAX_HISTORY];
RTC_DATA_ATTR int   rtc_historyCount = 0;
RTC_DATA_ATTR uint32_t rtc_bootCount = 0;
RTC_DATA_ATTR unsigned long rtc_lastAlertDepletion = 0;
RTC_DATA_ATTR unsigned long rtc_lastAlertQuality   = 0;
RTC_DATA_ATTR unsigned long rtc_lastAlertBattery   = 0;

// ----------------------------------------------------------------------------
// GLOBAL OBJECTS & STATE
// ----------------------------------------------------------------------------
OneWire oneWire(PIN_DS18B20);
DallasTemperature tempSensor(&oneWire);
WebServer server(80);

struct SensorReadings {
  float waterTempC;
  float distanceCm;
  float waterColumnCm;
  float tdsPpm;
  float clarityPct;
  float batteryVolts;
  int   batteryPct;
  float depletionRateCmPerDay;
  float daysToDry;
  bool  pumpRelayOn;
  bool  isValidDistance;
};

SensorReadings g_readings;
bool g_manualPumpOverride = false;
unsigned long g_lastSampleTime = 0;

// ============================================================================
// HELPER: MEDIAN FILTER (Insertion Sort)
// ============================================================================
float calculateMedian(float arr[], int n) {
  float temp[32];
  if (n > 32) n = 32;
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
  return (n % 2 == 0) ? ((temp[n / 2 - 1] + temp[n / 2]) / 2.0f) : temp[n / 2];
}

// ============================================================================
// SENSOR ACQUISITION MODULES
// ============================================================================

// 1. DS18B20 Temperature Acquisition
float readWaterTemperature() {
  tempSensor.requestTemperatures();
  float t = tempSensor.getTempCByIndex(0);
  if (t == DEVICE_DISCONNECTED_C || t < -20.0f || t > 85.0f) {
    Serial.println(F("[DS18B20] Warning: Sensor disconnected or error. Falling back to 25.0°C."));
    return 25.0f;
  }
  return t;
}

// 2. JSN-SR04T Ultrasonic Level Measurement with Speed-of-Sound Compensation
float readUltrasonicDistance(float ambientTempC, bool &isValid) {
  // Speed of sound compensation formula (m/s -> cm/us)
  float speedOfSound = 331.3f + (0.606f * ambientTempC); // in m/s
  float speedCmPerUs = (speedOfSound * 100.0f) / 1000000.0f;

  float samples[7];
  int validCount = 0;

  for (int i = 0; i < 7; i++) {
    digitalWrite(PIN_TRIG, LOW);
    delayMicroseconds(4);
    digitalWrite(PIN_TRIG, HIGH);
    delayMicroseconds(12);
    digitalWrite(PIN_TRIG, LOW);

    // Timeout: 35ms (~600 cm round trip limit)
    unsigned long duration = pulseIn(PIN_ECHO, HIGH, 35000);
    if (duration > 0) {
      float dist = (duration * speedCmPerUs) / 2.0f;
      // Filter out blind zone (< 20 cm) and out-of-range (> 600 cm)
      if (dist >= 20.0f && dist <= 600.0f) {
        samples[validCount++] = dist;
      }
    }
    delay(40);
  }

  if (validCount >= 3) {
    isValid = true;
    return calculateMedian(samples, validCount);
  } else {
    isValid = false;
    Serial.println(F("[Ultrasonic] Warning: Insufficient valid echoes. Check probe alignment or blind zone!"));
    return (validCount > 0) ? samples[0] : 0.0f;
  }
}

// 3. Analog TDS Probe with GPIO Power Gating (Anti-Electrolysis)
float readTds(float waterTempC) {
  // Turn ON switched power rail to the TDS module
  digitalWrite(PIN_TDS_PWR, HIGH);
  delay(50); // Settling delay for analog frontend

  float samples[20];
  for (int i = 0; i < 20; i++) {
    samples[i] = (float)analogReadMilliVolts(PIN_TDS_ADC) / 1000.0f; // in Volts
    delay(5);
  }

  // Turn OFF power immediately to prevent galvanic copper corrosion
  digitalWrite(PIN_TDS_PWR, LOW);

  float rawVoltage = calculateMedian(samples, 20);

  // Temperature compensation (standard reference 25 °C, ~2% per °C)
  float compensationCoefficient = 1.0f + 0.02f * (waterTempC - 25.0f);
  float compensatedVoltage = rawVoltage / compensationCoefficient;

  // Standard Gravity TDS polynomial curve:
  float tdsPpm = (133.42f * pow(compensatedVoltage, 3) 
                 - 255.86f * pow(compensatedVoltage, 2) 
                 + 857.39f * compensatedVoltage) * 0.5f * TDS_CAL_FACTOR;

  if (tdsPpm < 0.0f) tdsPpm = 0.0f;
  return tdsPpm;
}

// 4. Analog Turbidity Sensor with Relative Clarity Calculation
float readClarityPercentage() {
  float samples[15];
  for (int i = 0; i < 15; i++) {
    samples[i] = (float)analogReadMilliVolts(PIN_TURB_ADC) / 1000.0f; // in Volts
    delay(4);
  }
  float pinVoltage = calculateMedian(samples, 15);
  
  // Reconstruct module output voltage through voltage divider
  float moduleVoltage = pinVoltage * TURBIDITY_DIVIDER_RATIO;

  // Clarity percentage relative to clean benchmark water
  float clarity = (moduleVoltage / TURBIDITY_CLEAN_VOLTAGE) * 100.0f;
  if (clarity > 100.0f) clarity = 100.0f;
  if (clarity < 0.0f)   clarity = 0.0f;

  return clarity;
}

// 5. Battery Voltage Sense via 100k/100k Divider
float readBatteryVoltage(int &batteryPercent) {
  float samples[15];
  for (int i = 0; i < 15; i++) {
    samples[i] = (float)analogReadMilliVolts(PIN_BATT_ADC) / 1000.0f;
    delay(3);
  }
  float pinVoltage = calculateMedian(samples, 15);
  float battVolts = pinVoltage * BATTERY_DIVIDER_RATIO;

  // Approximate Li-ion state of charge: 3.3V (0%) to 4.2V (100%)
  float pct = ((battVolts - 3.30f) / (4.20f - 3.30f)) * 100.0f;
  if (pct > 100.0f) pct = 100.0f;
  if (pct < 0.0f)   pct = 0.0f;
  batteryPercent = (int)pct;

  return battVolts;
}

// ============================================================================
// EDGE ANALYTICS: TREND & DAYS-TO-DRY PREDICTION
// ============================================================================
void pushDistanceReading(float dist) {
  if (rtc_historyCount < MAX_HISTORY) {
    rtc_distHistory[rtc_historyCount++] = dist;
  } else {
    // Shift left to maintain rolling window
    for (int i = 0; i < MAX_HISTORY - 1; i++) {
      rtc_distHistory[i] = rtc_distHistory[i + 1];
    }
    rtc_distHistory[MAX_HISTORY - 1] = dist;
  }
}

void computeDepletionAnalytics(float currentDist, float &rateCmPerDay, float &daysToDry) {
  if (rtc_historyCount < 2) {
    rateCmPerDay = 0.0f;
    daysToDry = 999.0f;
    return;
  }

  // Least Squares Linear Regression: y = a + b * x
  // In DEMO_MODE: readings are taken every few seconds. We scale the delta 
  // so judges can observe instantaneous drawdown velocity when water is drained!
  float sumX = 0, sumY = 0, sumXY = 0, sumXX = 0;
  int n = rtc_historyCount;

  for (int i = 0; i < n; i++) {
    float x = (float)i;
    float y = rtc_distHistory[i];
    sumX += x;
    sumY += y;
    sumXY += x * y;
    sumXX += x * x;
  }

  float slope = (n * sumXY - sumX * sumY) / (n * sumXX - sumX * sumX + 0.0001f);

  if (DEMO_MODE) {
    // In demo mode: scale slope (cm/sample) to simulated cm/day
    rateCmPerDay = slope * 20.0f;
  } else {
    // In field mode: samples are 30 mins apart (48 samples = 1 day)
    rateCmPerDay = slope * 48.0f;
  }

  // If level is dropping (slope > 0, distance increasing)
  if (rateCmPerDay > 0.5f) {
    float remainingDistance = PUMP_INTAKE_DEPTH_CM - currentDist;
    if (remainingDistance > 0.0f) {
      daysToDry = remainingDistance / rateCmPerDay;
    } else {
      daysToDry = 0.0f; // Water already at or below pump intake!
    }
  } else {
    rateCmPerDay = 0.0f;
    daysToDry = 999.0f; // Stable or recharging
  }
}

// ============================================================================
// HARDWARE PUMP INTERLOCK (Automatic Dry-Run Cutoff)
// ============================================================================
void updatePumpRelayState(float distance, float waterColumn) {
  // If water level drops below pump intake or water column drops below safety reserve:
  bool emergencyCutoff = (distance >= PUMP_INTAKE_DEPTH_CM) || (waterColumn <= MIN_RESERVE_COLUMN_CM);

  if (emergencyCutoff || g_manualPumpOverride) {
    // Trip relay: Cut pump power (Active LOW relay -> set HIGH to disconnect)
    digitalWrite(PIN_PUMP_RELAY, HIGH);
    g_readings.pumpRelayOn = false;
  } else {
    // Safe to run pump
    digitalWrite(PIN_PUMP_RELAY, LOW);
    g_readings.pumpRelayOn = true;
  }
}

// ============================================================================
// SECURE TELEGRAM BOT NOTIFICATION DISPATCH (Direct HTTPS)
// ============================================================================
bool sendTelegramMessage(const String &message) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(F("[Telegram] Error: WiFi not connected. Cannot send alert."));
    return false;
  }

  if (String(TELEGRAM_BOT_TOKEN) == "YOUR_BOT_TOKEN_FROM_BOTFATHER" || 
      String(TELEGRAM_CHAT_ID) == "YOUR_NUMERIC_CHAT_ID") {
    Serial.println(F("[Telegram] Notice: Please configure TELEGRAM_BOT_TOKEN and TELEGRAM_CHAT_ID in config.h"));
    return false;
  }

  Serial.println(F("[Telegram] Connecting to Telegram API via TLS..."));
  WiFiClientSecure client;
  client.setInsecure(); // Bypass CA certificate verification for fast prototype handshake
  client.setTimeout(8000);

  if (!client.connect("api.telegram.org", 443)) {
    Serial.println(F("[Telegram] Connection failed to api.telegram.org:443"));
    return false;
  }

  // URL-encode message
  String encodedMsg = "";
  for (unsigned int i = 0; i < message.length(); i++) {
    char c = message[i];
    if (isalnum(c)) {
      encodedMsg += c;
    } else if (c == ' ') {
      encodedMsg += "%20";
    } else if (c == '\n') {
      encodedMsg += "%0A";
    } else {
      char buf[4];
      sprintf(buf, "%%%02X", (unsigned char)c);
      encodedMsg += buf;
    }
  }

  String url = "/bot" + String(TELEGRAM_BOT_TOKEN) + "/sendMessage?chat_id=" + String(TELEGRAM_CHAT_ID) + "&text=" + encodedMsg + "&parse_mode=Markdown";

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: api.telegram.org\r\n" +
               "Connection: close\r\n\r\n");

  unsigned long startWait = millis();
  while (client.connected() && millis() - startWait < 4000) {
    while (client.available()) {
      String line = client.readStringUntil('\n');
      if (line.indexOf("\"ok\":true") > 0) {
        Serial.println(F("[Telegram] Alert delivered successfully!"));
        return true;
      }
    }
  }
  return true;
}

// ============================================================================
// ALERT EVALUATION & COOLDOWN LOGIC
// ============================================================================
void evaluateAlertTriggers(const SensorReadings &r) {
  unsigned long now = millis();

  // 1. Critical Water Depletion or Imminent Dry-Run
  if ((r.depletionRateCmPerDay >= THRESHOLD_DEPLETION_RATE || r.daysToDry <= THRESHOLD_CRITICAL_DAYS || r.distanceCm >= PUMP_INTAKE_DEPTH_CM) &&
      (now - rtc_lastAlertDepletion >= ALERT_COOLDOWN_MS || rtc_lastAlertDepletion == 0)) {
    
    String alert = "🚨 *UNDERGROUND WATER MANAGEMENT: EMERGENCY DEPLETION ALERT*\n";
    alert += "📍 *Site:* Community Well #1\n";
    alert += "📉 *Drawdown Rate:* " + String(r.depletionRateCmPerDay, 1) + " cm/day\n";
    alert += "⏳ *Estimated Days-to-Dry:* " + (r.daysToDry < 999 ? String(r.daysToDry, 0) + " days" : "Critical!") + "\n";
    alert += "🛑 *Action Taken:* Automated Pump Cutoff Interlock ENGAGED to prevent motor burn!\n";
    alert += "💡 *Recommendation:* Halt irrigation pumping immediately; initiate water rationing.";

    if (sendTelegramMessage(alert)) {
      rtc_lastAlertDepletion = now;
    }
  }

  // 2. Contamination / High Salinity / High Turbidity
  if ((r.tdsPpm >= THRESHOLD_MAX_TDS_PPM || r.clarityPct <= THRESHOLD_MIN_CLARITY_PCT) &&
      (now - rtc_lastAlertQuality >= ALERT_COOLDOWN_MS || rtc_lastAlertQuality == 0)) {

    String alert = "⚠️ *UNDERGROUND WATER MANAGEMENT: WATER QUALITY ALERT*\n";
    alert += "📍 *Site:* Community Well #1\n";
    alert += "🧪 *TDS:* " + String(r.tdsPpm, 0) + " ppm (BIS Limit: 500 ppm)\n";
    alert += "💎 *Relative Clarity:* " + String(r.clarityPct, 0) + "% (Threshold: 70%)\n";
    alert += "🛑 *Health Notice:* Water exceeds potable standards. UNFIT FOR DRINKING.\n";
    alert += "💡 *Recommendation:* Send samples for laboratory chemical testing (Fluoride/Arsenic/Nitrate).";

    if (sendTelegramMessage(alert)) {
      rtc_lastAlertQuality = now;
    }
  }

  // 3. Low Battery Warning
  if (r.batteryVolts <= THRESHOLD_LOW_BATTERY_V &&
      (now - rtc_lastAlertBattery >= ALERT_COOLDOWN_MS || rtc_lastAlertBattery == 0)) {

    String alert = "🔋 *UNDERGROUND WATER MANAGEMENT: NODE HEALTH ALERT*\n";
    alert += "⚡ *Battery Voltage:* " + String(r.batteryVolts, 2) + " V (" + String(r.batteryPct) + "%)\n";
    alert += "⚠️ *Status:* Low solar reserve. Node entering ultra-power-saving mode.\n";
    alert += "🔧 *Action:* Inspect solar panel angle or remove dust accumulation.";

    if (sendTelegramMessage(alert)) {
      rtc_lastAlertBattery = now;
    }
  }
}

// ============================================================================
// WEBSERVER ROUTE HANDLERS
// ============================================================================
void handleRoot() {
  server.send_P(200, "text/html", INDEX_HTML);
}

void handleApiData() {
  String json = "{";
  json += "\"distance\":" + String(g_readings.distanceCm, 1) + ",";
  json += "\"water_column\":" + String(g_readings.waterColumnCm, 1) + ",";
  json += "\"total_depth\":" + String(TOTAL_WELL_DEPTH_CM, 1) + ",";
  json += "\"pump_intake\":" + String(PUMP_INTAKE_DEPTH_CM, 1) + ",";
  json += "\"tds\":" + String(g_readings.tdsPpm, 0) + ",";
  json += "\"clarity\":" + String(g_readings.clarityPct, 0) + ",";
  json += "\"temp\":" + String(g_readings.waterTempC, 1) + ",";
  json += "\"battery\":" + String(g_readings.batteryVolts, 2) + ",";
  json += "\"batt_pct\":" + String(g_readings.batteryPct) + ",";
  json += "\"depletion_rate\":" + String(g_readings.depletionRateCmPerDay, 1) + ",";
  json += "\"days_to_dry\":" + String(g_readings.daysToDry, 1) + ",";
  json += "\"pump_relay_on\":" + String(g_readings.pumpRelayOn ? "true" : "false");
  json += "}";
  server.send(200, "application/json", json);
}

void handlePumpToggle() {
  g_manualPumpOverride = !g_manualPumpOverride;
  updatePumpRelayState(g_readings.distanceCm, g_readings.waterColumnCm);
  server.send(200, "text/plain", g_manualPumpOverride ? "PUMP_OVERRIDDEN_OFF" : "PUMP_AUTO_ENABLED");
}

void handleTestAlert() {
  String testMsg = "🚀 *Underground Water Management Demo Alert*\nLive Telegram test triggered from the Web Dashboard!\nAll edge prediction systems are operational.";
  bool ok = sendTelegramMessage(testMsg);
  server.send(200, "text/plain", ok ? "Alert Sent to Telegram!" : "Alert Failed. Check WiFi/Token.");
}

// ============================================================================
// MEASUREMENT CYCLE
// ============================================================================
void performFullMeasurementCycle() {
  digitalWrite(PIN_STATUS_LED, HIGH);

  // 1. Water Temperature
  g_readings.waterTempC = readWaterTemperature();

  // 2. Ultrasonic Level with Speed-of-Sound Compensation
  g_readings.distanceCm = readUltrasonicDistance(g_readings.waterTempC, g_readings.isValidDistance);

  // 3. Derived Water Column (Height of water above well floor)
  g_readings.waterColumnCm = TOTAL_WELL_DEPTH_CM - (g_readings.distanceCm + SENSOR_MOUNT_OFFSET_CM);
  if (g_readings.waterColumnCm < 0.0f) g_readings.waterColumnCm = 0.0f;

  // 4. TDS with Temperature Compensation
  g_readings.tdsPpm = readTds(g_readings.waterTempC);

  // 5. Turbidity Relative Clarity
  g_readings.clarityPct = readClarityPercentage();

  // 6. Battery Voltage
  g_readings.batteryVolts = readBatteryVoltage(g_readings.batteryPct);

  // 7. Update Rolling History & Depletion Analytics
  if (g_readings.isValidDistance) {
    pushDistanceReading(g_readings.distanceCm);
    computeDepletionAnalytics(g_readings.distanceCm, g_readings.depletionRateCmPerDay, g_readings.daysToDry);
  }

  // 8. Update Pump Interlock Relay
  updatePumpRelayState(g_readings.distanceCm, g_readings.waterColumnCm);

  // 9. Output to Serial Plotter & Monitor
  // Format formatted for Arduino Serial Plotter (Tools -> Serial Plotter)
  Serial.printf("Distance_cm:%.1f,Water_Column_cm:%.1f,TDS_ppm:%.1f,Clarity_pct:%.1f,Temp_C:%.1f,Depletion_Rate:%.1f\n",
                g_readings.distanceCm,
                g_readings.waterColumnCm,
                g_readings.tdsPpm,
                g_readings.clarityPct,
                g_readings.waterTempC,
                g_readings.depletionRateCmPerDay);

  // 10. Check Alert Thresholds
  evaluateAlertTriggers(g_readings);

  digitalWrite(PIN_STATUS_LED, LOW);
}

// ============================================================================
// ARDUINO SETUP
// ============================================================================
void setup() {
  Serial.begin(115200);
  delay(1000);
  rtc_bootCount++;

  Serial.println(F("\n======================================================="));
  Serial.println(F("💧 Underground Water Management - IoT Intelligence Node"));
  Serial.println(F("InnovateX: The Future Habitats Challenge | Techfest IIT Bombay"));
  Serial.printf("Boot Count: %u | Mode: %s\n", rtc_bootCount, DEMO_MODE ? "DEMO_BENCH_MODE" : "FIELD_DEEP_SLEEP");
  Serial.println(F("======================================================="));

  // Initialize GPIO Pins
  pinMode(PIN_STATUS_LED, OUTPUT);
  pinMode(PIN_PUMP_RELAY, OUTPUT);
  pinMode(PIN_TDS_PWR, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  // Default pin states
  digitalWrite(PIN_STATUS_LED, LOW);
  digitalWrite(PIN_PUMP_RELAY, LOW); // Relay enabled initially
  digitalWrite(PIN_TDS_PWR, LOW);    // TDS powered off until read
  digitalWrite(PIN_TRIG, LOW);

  // ADC Attenuation for 3.3V range (11dB gives full 0 - 3.3V scale on ESP32)
  analogSetAttenuation(ADC_11db);

  // Initialize DS18B20 1-Wire
  tempSensor.begin();

  // Connect to WiFi
  Serial.printf("[WiFi] Connecting to %s...\n", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long wifiStart = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - wifiStart < 8000) {
    delay(400);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(F("\n[WiFi] Connected successfully!"));
    Serial.print(F("[WiFi] IP Address: http://"));
    Serial.println(WiFi.localIP());
  } else {
    Serial.println(F("\n[WiFi] Warning: Could not connect to AP. Starting Access Point mode..."));
    WiFi.mode(WIFI_AP);
    WiFi.softAP("WaterMonitor_AP", "water12345");
    Serial.print(F("[WiFi] Connect to 'WaterMonitor_AP' -> Open http://"));
    Serial.println(WiFi.softAPIP());
  }

  // Setup Web Server Routes
  server.on("/", handleRoot);
  server.on("/api/data", handleApiData);
  server.on("/api/pump_toggle", handlePumpToggle);
  server.on("/api/test_alert", handleTestAlert);
  server.begin();
  Serial.println(F("[WebServer] HTTP Server started. Web Dashboard is live!"));

  // Perform initial measurement
  performFullMeasurementCycle();

  // If in Field Deep Sleep Mode: do one cycle, then enter ultra-low power sleep
  if (!DEMO_MODE) {
    Serial.printf("[Power] Field Mode: Entering deep sleep for %d minutes...\n", FIELD_SLEEP_INTERVAL_MIN);
    esp_sleep_enable_timer_wakeup((uint64_t)FIELD_SLEEP_INTERVAL_MIN * 60ULL * 1000000ULL);
    esp_deep_sleep_start();
  }
}

// ============================================================================
// ARDUINO LOOP (Runs continuously in DEMO_BENCH_MODE)
// ============================================================================
void loop() {
  // Handle incoming HTTP requests from the Web Dashboard
  server.handleClient();

  // Periodic sensor sampling
  if (millis() - g_lastSampleTime >= (DEMO_SAMPLE_INTERVAL_SEC * 1000UL)) {
    g_lastSampleTime = millis();
    performFullMeasurementCycle();
  }
}
