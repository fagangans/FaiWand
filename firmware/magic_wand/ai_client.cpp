// ai_client.cpp
#include "ai_client.h"
#include "config.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

bool AiClient::connectWifi(uint32_t timeoutMs) {
  if (WiFi.status() == WL_CONNECTED) return true;

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < timeoutMs) {
    delay(250);
#if DEBUG_PRINT
    Serial.print(".");
#endif
  }
  return WiFi.status() == WL_CONNECTED;
}

void AiClient::disconnectWifi() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

bool AiClient::isWifiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

bool AiClient::sendGesture(const char *gesture, String &outResponse) {
  if (!isWifiConnected()) return false;

  // Pilih endpoint/model/key sesuai AI_PROVIDER di config.h
  const char *url;
  const char *model;
  const char *apiKey;

  if (strcmp(AI_PROVIDER, "grok") == 0) {
    url = GROK_API_URL;
    model = GROK_MODEL;
    apiKey = GROK_API_KEY;
  } else {
    url = OPENAI_API_URL;
    model = OPENAI_MODEL;
    apiKey = OPENAI_API_KEY;
  }

  WiFiClientSecure client;
  client.setInsecure(); // MVP: skip cert validation. Untuk produksi, pin root CA yang sesuai.

  HTTPClient https;
  if (!https.begin(client, url)) {
    return false;
  }
  https.addHeader("Content-Type", "application/json");
  https.addHeader("Authorization", String("Bearer ") + apiKey);

  // Bangun payload chat-completions: prompt singkat berisi nama gesture yang terdeteksi
  StaticJsonDocument<512> reqDoc;
  reqDoc["model"] = model;
  JsonArray messages = reqDoc.createNestedArray("messages");

  JsonObject sys = messages.createNestedObject();
  sys["role"] = "system";
  sys["content"] =
      "Kamu adalah AI di dalam tongkat sihir (magic wand). "
      "User baru saja melakukan gestur bernama tertentu. "
      "Balas singkat (maks 1-2 kalimat) dengan gaya seperti mantra/respons sihir yang sesuai gestur itu.";

  JsonObject user = messages.createNestedObject();
  user["role"] = "user";
  user["content"] = String("Gestur terdeteksi: ") + gesture;

  reqDoc["max_tokens"] = 60;

  String reqBody;
  serializeJson(reqDoc, reqBody);

  int httpCode = https.POST(reqBody);
  if (httpCode != HTTP_CODE_OK) {
#if DEBUG_PRINT
    Serial.printf("[AiClient] HTTP error: %d\n", httpCode);
#endif
    https.end();
    return false;
  }

  String respBody = https.getString();
  https.end();

  StaticJsonDocument<1024> respDoc;
  DeserializationError err = deserializeJson(respDoc, respBody);
  if (err) {
    return false;
  }

  const char *content = respDoc["choices"][0]["message"]["content"] | "";
  outResponse = String(content);
  outResponse.trim();
  return outResponse.length() > 0;
}
