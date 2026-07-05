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

// Resolusi host AI lokal: kalau berformat mDNS ("xxx.local"), query IP-nya dulu.
// Kalau IP statis atau hostname biasa, dipakai langsung (HTTPClient akan resolve sendiri).
// Return "" kalau gagal resolve.
static String resolveBaseUrl() {
  String host = LOCAL_AI_HOST;
  if (endsWith(LOCAL_AI_HOST, ".local")) {
    String nameOnly = host.substring(0, host.length() - 6); // buang ".local"
    IPAddress resolvedIp = MDNS.queryHost(nameOnly);
    if (resolvedIp == IPAddress(0, 0, 0, 0)) {
#if DEBUG_PRINT
      Serial.printf("[AiClient] Gagal resolve mDNS host: %s\n", LOCAL_AI_HOST);
#endif
      return "";
    }
    host = resolvedIp.toString();
  }
  return String(LOCAL_AI_USE_HTTPS ? "https://" : "http://") + host + ":" + String(LOCAL_AI_PORT);
}

// Parse respons JSON bersama {"text":..., "audio_url":...} dipakai oleh
// sendGesture() dan sendVoice().
static bool parseAiReply(const String &respBody, const String &baseUrl, AiReply &outReply) {
  StaticJsonDocument<512> respDoc;
  DeserializationError err = deserializeJson(respDoc, respBody);
  if (err) return false;

  const char *text = respDoc["text"] | "";
  outReply.text = String(text);
  outReply.text.trim();

  const char *audioUrl = respDoc["audio_url"] | "";
  outReply.audioUrl = String(audioUrl);
  if (outReply.audioUrl.length() > 0 && !outReply.audioUrl.startsWith("http")) {
    // server balas path relatif ("/audio/xxx.mp3") -> gabung dengan base url
    outReply.audioUrl = baseUrl + outReply.audioUrl;
  }

  return outReply.text.length() > 0;
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

bool AiClient::sendGesture(const char *gesture, AiReply &outReply) {
  if (!isWifiConnected()) return false;

  String baseUrl = resolveBaseUrl();
  if (baseUrl.length() == 0) return false;
  String url = baseUrl + LOCAL_AI_PATH;

  HTTPClient http;
  if (!http.begin(url)) return false;
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(10000); // AI lokal bisa butuh waktu proses lebih lama

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

  return parseAiReply(respBody, baseUrl, outReply);
}

bool AiClient::sendVoice(const uint8_t *wavData, size_t wavSize, AiReply &outReply) {
  if (!isWifiConnected()) return false;
  if (!wavData || wavSize == 0) return false;

  String baseUrl = resolveBaseUrl();
  if (baseUrl.length() == 0) return false;
  String url = baseUrl + "/voice"; // endpoint khusus speech-to-text di server

  HTTPClient http;
  if (!http.begin(url)) return false;
  http.addHeader("Content-Type", "audio/wav");
  http.setTimeout(20000); // STT + generate AI + TTS bisa makan waktu lebih lama dari sekadar gesture

  int httpCode = http.POST((uint8_t *)wavData, wavSize);
  if (httpCode != HTTP_CODE_OK) {
#if DEBUG_PRINT
    Serial.printf("[AiClient] HTTP error (voice): %d (url=%s)\n", httpCode, url.c_str());
#endif
    http.end();
    return false;
  }

  String respBody = http.getString();
  http.end();

  return parseAiReply(respBody, baseUrl, outReply);
}
