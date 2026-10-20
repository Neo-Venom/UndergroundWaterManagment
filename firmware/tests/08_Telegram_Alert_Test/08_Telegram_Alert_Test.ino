/*
 * ============================================================================
 * TEST 08: Direct HTTPS Telegram Bot Emergency Alert Test
 * Component: ESP32 WiFi + Secure TLS Client (api.telegram.org)
 *
 * PURPOSE:
 * Tests sending formatted markdown alerts with emojis to your Telegram phone
 * over direct HTTPS, completely eliminating heavy external libraries.
 *
 * HOW TO SET UP YOUR FREE TELEGRAM BOT (Takes 2 minutes):
 * 1. Open the Telegram app, search for "@BotFather", and tap START.
 * 2. Send the message: /newbot
 * 3. Give your bot a name (e.g. "Water Monitor Bot") and username (e.g. "groundwater_alert_bot").
 * 4. BotFather will provide an HTTP API Token (e.g. "123456789:ABCdefGhIJKlmNoPQRstuVWXyz").
 * 5. Search for "@userinfobot" on Telegram, tap START -> it gives your numeric Id (e.g. "987654321").
 * 6. Send any first message (like "hi") to your new bot so it has permission to message you.
 * 7. Paste your BOT_TOKEN and CHAT_ID below.
 *
 * EXPECTED OUTPUT ON SERIAL MONITOR (115200 Baud):
 * ------------------------------------------------
 * [WIFI] Connecting to MyHotspot...
 * [WIFI] Connected! IP: 192.168.1.105
 * [TLS] Connecting to api.telegram.org:443...
 * [TLS] Connected! Sending message payload...
 * [TELEGRAM] Success! Telegram API responded: {"ok":true,...}
 * (Your phone will immediately vibrate with the test alert!)
 * ============================================================================
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

// Enter your WiFi Credentials:
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// Enter your Telegram Bot Credentials:
const char* BOT_TOKEN     = "YOUR_BOT_TOKEN_FROM_BOTFATHER";
const char* CHAT_ID       = "YOUR_NUMERIC_CHAT_ID";

bool sendTelegramAlert(const String& message) {
  Serial.println(F("[TLS] Initializing secure TLS connection to api.telegram.org..."));
  
  WiFiClientSecure client;
  client.setInsecure(); // Fast handshake for prototype without hardcoded CA cert expiration issues
  client.setTimeout(10000);

  if (!client.connect("api.telegram.org", 443)) {
    Serial.println(F("[ERROR] Connection to api.telegram.org failed. Check internet connectivity!"));
    return false;
  }

  // URL-encode string
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

  String url = "/bot" + String(BOT_TOKEN) + "/sendMessage?chat_id=" + String(CHAT_ID) + "&text=" + encodedMsg + "&parse_mode=Markdown";

  Serial.println(F("[TLS] Sending HTTP GET request..."));
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: api.telegram.org\r\n" +
               "Connection: close\r\n\r\n");

  unsigned long timeout = millis();
  while (client.connected() && millis() - timeout < 5000) {
    while (client.available()) {
      String line = client.readStringUntil('\n');
      if (line.indexOf("\"ok\":true") > 0) {
        Serial.println(F("[TELEGRAM] ✅ SUCCESS! Alert received by Telegram servers!"));
        return true;
      } else if (line.indexOf("\"ok\":false") > 0) {
        Serial.printf("[TELEGRAM] ❌ ERROR: Telegram rejected message: %s\n", line.c_str());
        return false;
      }
    }
  }

  Serial.println(F("[TLS] Request finished."));
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println(F("\n======================================================="));
  Serial.println(F("✅ Underground Water Management Test 08: Telegram Alerts"));
  Serial.println(F("======================================================="));

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.printf("[WIFI] Connecting to %s", WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println(F("\n[WIFI] Connected!"));
  Serial.print(F("[WIFI] Local IP Address: "));
  Serial.println(WiFi.localIP());

  // Formulate a demo emergency test message
  String msg = "🚀 *Underground Water Management System Verification*\n";
  msg += "📍 *Site:* Techfest IIT Bombay Lab\n";
  msg += "🌊 *Water Level:* 48.2 cm\n";
  msg += "🧪 *TDS:* 210 ppm (BIS IS 10500 Compliant)\n";
  msg += "⚡ *Solar Battery:* 4.05 V (92%)\n";
  msg += "✅ *Status:* Dual-core ESP32 edge telemetry is 100% active!";

  Serial.println(F("\n[TEST] Dispatching Telegram message..."));
  sendTelegramAlert(msg);
}

void loop() {
  // Idle after sending once
  delay(10000);
}
