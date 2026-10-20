#pragma once
#include <Arduino.h>

// ============================================================================
// 💧 Underground Water Management - System Configuration
// InnovateX: Techfest, IIT Bombay
// ============================================================================

// ----------------------------------------------------------------------------
// 1. OPERATION MODE
// ----------------------------------------------------------------------------
// Set DEMO_MODE to 'true' for Hackathon Stage Presentation:
//   - Continuous execution (no deep sleep)
//   - Live Embedded Web Dashboard hosted at ESP32 IP address
//   - Real-time Serial Plotter streaming
//   - Fast response to water level changes (samples every 3 seconds)
//
// Set DEMO_MODE to 'false' for Field Deployment:
//   - Reads sensors -> checks thresholds -> sends cloud/telegram -> deep sleeps
//   - Wakes every SLEEP_INTERVAL_MINUTES to maximize solar battery life
#define DEMO_MODE true
#define FIELD_SLEEP_INTERVAL_MIN 30
#define DEMO_SAMPLE_INTERVAL_SEC  3

// ----------------------------------------------------------------------------
// 2. NETWORK & TELEGRAM CREDENTIALS
// ----------------------------------------------------------------------------
// Connect to your home router, college lab WiFi, or mobile hotspot for demos
#define WIFI_SSID     "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// Telegram Bot credentials:
// 1. Create a bot using @BotFather on Telegram to get your BOT_TOKEN
// 2. Get your Chat ID using @userinfobot or @raw_data_bot on Telegram
#define TELEGRAM_BOT_TOKEN "YOUR_BOT_TOKEN_FROM_BOTFATHER"
#define TELEGRAM_CHAT_ID   "YOUR_NUMERIC_CHAT_ID"

// Optional ThingSpeak Channel (Set to "" if not using)
#define THINGSPEAK_API_KEY ""

// ----------------------------------------------------------------------------
// 3. GPIO PIN DEFINITIONS (Strictly follows docs/pinout.md)
// ----------------------------------------------------------------------------
#define PIN_DS18B20    4   // 1-Wire Temperature Data (4.7k pull-up to 3.3V)
#define PIN_TRIG       26  // JSN-SR04T Ultrasonic Trigger
#define PIN_ECHO       27  // JSN-SR04T Ultrasonic Echo (via 1k/2k divider)
#define PIN_TDS_ADC    34  // Analog TDS Probe Out (ADC1_CH6)
#define PIN_TDS_PWR    25  // Switched 3.3V power gate for TDS (protects from electrolysis)
#define PIN_TURB_ADC   35  // Analog Turbidity Out (ADC1_CH7, via 10k/20k divider)
#define PIN_BATT_ADC   32  // Battery sense (ADC1_CH4, via 100k/100k divider)
#define PIN_STATUS_LED 2   // Onboard Blue LED
#define PIN_PUMP_RELAY 19  // Pump Dry-Run Cutoff Relay (Active LOW)

// ----------------------------------------------------------------------------
// 4. PHYSICAL WELL GEOMETRY & RESERVOIR PARAMETERS (in cm)
// ----------------------------------------------------------------------------
// For hackathon bench demo with a bucket/acrylic cylinder, adjust these numbers:
#define TOTAL_WELL_DEPTH_CM       100.0f  // Total depth from sensor mount to well bottom
#define PUMP_INTAKE_DEPTH_CM      85.0f   // Depth where pump intake sits (critical cutoff level)
#define SENSOR_MOUNT_OFFSET_CM    15.0f   // Height of sensor above max water brim (blind zone margin)
#define MIN_RESERVE_COLUMN_CM     15.0f   // Minimum water height above pump intake for safe pumping

// ----------------------------------------------------------------------------
// 5. THRESHOLDS & ALARM LIMITS (BIS IS 10500 standards for India)
// ----------------------------------------------------------------------------
#define THRESHOLD_MAX_TDS_PPM      500.0f  // BIS IS 10500 acceptable limit for drinking water
#define THRESHOLD_MIN_CLARITY_PCT  70.0f   // Turbidity drop indicating silt or contamination
#define THRESHOLD_CRITICAL_DAYS    7.0f    // Days-to-dry warning threshold
#define THRESHOLD_DEPLETION_RATE   10.0f   // Fast depletion warning (cm/day)
#define THRESHOLD_LOW_BATTERY_V    3.50f   // Low Li-ion battery warning voltage

// Alert cooldown in milliseconds (prevents Telegram spamming)
#define ALERT_COOLDOWN_MS (DEMO_MODE ? 60000UL : 43200000UL) // 1 min in demo, 12 hrs in field

// ----------------------------------------------------------------------------
// 6. CALIBRATION CONSTANTS
// ----------------------------------------------------------------------------
// TDS Calibration factor: adjust if you have a reference commercial TDS meter
#define TDS_CAL_FACTOR            1.0f

// Turbidity Sensor: voltage when submerged in crystal clean drinking water
// (Note: after 10k/20k divider, 4.2V becomes 2.8V at the ESP32 pin)
#define TURBIDITY_DIVIDER_RATIO   1.5f    // (10k + 20k) / 20k = 1.5
#define TURBIDITY_CLEAN_VOLTAGE   4.10f   // Sensor module output voltage in clean water

// Battery Divider: two 100k resistors divide voltage by 2
#define BATTERY_DIVIDER_RATIO     2.0f
