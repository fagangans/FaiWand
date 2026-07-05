// ai_client.cpp
#include "ai_client.h"
#include "config.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

static bool endsWith(const char *str, const char *suffix) {
  size_t lenStr = strlen(str);
  size_t lenSuf = strlen(suffix);
  if (lenSuf > lenStr) return false;
  return strcmp(str + (lenStr - lenSuf), suffix) == 0;
}

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
  if (WiFi.status() != WL_CONNECTED) return false;

  MDNS.begin("magicwand"); // supaya resolusi ".local" hostname AI lokal bisa jalan
  return true;
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

  // Resolusi host AI lokal: kalau berformat mDNS ("xxx.local"), query IP-nya dulu.
  // Kalau IP statis atau hostname biasa, dipakai langsung (HTTPClient akan resolve sendiri).
  String host = LOCAL_AI_HOST;
  if (endsWith(LOCAL_AI_HOST, ".local")) {
    String nameOnly = host.substring(0, host.length() - 6); // buang ".local"
    IPAddress resolvedIp = MDNS.queryHost(nameOnly);
    if (resolvedIp == IPAddress(0, 0, 0, 0)) {
#if DEBUG_PRINT
      Serial.printf("[AiClient] Gagal resolve mDNS host: %s\n", LOCAL_AI_HOST);
#endif
      return false;
    }
    host = resolvedIp.toString();
  }

  String url = String(LOCAL_AI_USE_HTTPS ? "https://" : "http://") + host + ":" +
               String(LOCAL_AI_PORT) + LOCAL_AI_PATH;

  HTTPClient http;
  if (!http.begin(url)) {
    return false;
  }
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(10000); // AI lokal (speech-to-speech) bisa butuh waktu proses lebih lama

  // Payload minimal: nama gesture yang terdeteksi. Sesuaikan key JSON ini dengan
  // kontrak API server AI lokal kamu kalau berbeda.
  StaticJsonDocument<128> reqDoc;
  reqDoc["gesture"] = gesture;
  String reqBody;
  serializeJson(reqDoc, reqBody);

  int httpCode = http.POST(reqBody);
  if (httpCode != HTTP_CODE_OK) {
#if DEBUG_PRINT
    Serial.printf("[AiClient] HTTP error: %d (url=%s)\n", httpCode, url.c_str());
#endif
    http.end();
    return false;
  }

  String respBody = http.getString();
  http.end();

  // Respons diharapkan: {"text": "..."} — sesuaikan key ini dengan output server AI lokal kamu
  // kalau berbeda (misal server memisahkan field "reply" atau "message").
  StaticJsonDocument<512> respDoc;
  DeserializationError err = deserializeJson(respDoc, respBody);
  if (err) {
    return false;
  }

  const char *text = respDoc["text"] | "";
  outResponse = String(text);
  outResponse.trim();
  return outResponse.length() > 0;
}
