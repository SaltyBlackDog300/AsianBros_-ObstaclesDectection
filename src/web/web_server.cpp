#include "web_server.h"
#include <WiFiS3.h>
#include "dashboard.h"
#include "../../config.h"
#include "../modes/modes.h"

static WiFiServer server(80);

static void sendResponse(WiFiClient& client, const char* type, const char* body) {
  client.print("HTTP/1.1 200 OK\r\nContent-Type: ");
  client.print(type);
  client.print("\r\nCache-Control: no-store\r\nConnection: close\r\n\r\n");
  client.print(body);
}

static String queryValue(const String& path, const char* key) {
  String needle = String(key) + "=";
  int start = path.indexOf(needle);
  if (start < 0) return "";
  start += needle.length();
  int end = path.indexOf('&', start);
  return end < 0 ? path.substring(start) : path.substring(start, end);
}

static String statusJson() {
  String json = "{\"mode\":\"";
  json += MODE_NAMES[mode];
  json += "\",\"dist\":";
  json += distanceCm;
  json += ",\"neck\":";
  json += neckAngle;
  json += ",\"speed\":";
  json += (long)(speedCms < 0 ? -1 : speedCms + 0.5f);
  json += ",\"peak\":";
  json += (long)(peakSpeedCms + 0.5f);
  json += ",\"turn\":";
  json += turnPercent;
  json += ",\"drive\":";
  json += drivePercent;
  json += ",\"stop\":";
  json += stopCm;
  json += ",\"t90\":";
  json += turn90Ms;
  json += ",\"motion\":\"";
  json += motion;
  json += "\",\"radar\":[";
  for (int i = 0; i < 19; i++) {
    if (i) json += ',';
    json += radar[i];
  }
  json += "]}";
  return json;
}

static String mapJson() {
  String json = "{\"id\":";
  json += mapId;
  json += ",\"step\":";
  json += mapScanStep;
  json += ",\"reads\":";
  json += mapReads;
  json += ",\"route\":";
  json += mapRoute;
  json += ",\"aim\":";
  json += mapAim;
  json += ",\"pts\":[";
  for (int i = 0; i < mapPoints; i++) {
    if (i) json += ',';
    json += mapDist[i];
  }
  json += "]}";
  return json;
}

void webBegin() {
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Wi-Fi module not found");
    return;
  }
  Serial.print("Joining ");
  Serial.println(WIFI_SSID);
  WiFi.config(IPAddress(ROVER_IP), IPAddress(ROVER_GATEWAY), IPAddress(ROVER_GATEWAY), IPAddress(ROVER_SUBNET));
  for (int attempt = 0; attempt < 3 && WiFi.status() != WL_CONNECTED; attempt++) {
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    delay(2000);
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Hotspot not found, starting own network instead");
    WiFi.config(IPAddress(192, 168, 4, 1));
    WiFi.beginAP(AP_SSID, AP_PASS);
  }
  for (int i = 0; i < 20 && WiFi.localIP() == IPAddress(0, 0, 0, 0); i++) delay(250);
  server.begin();
  Serial.print("Control page: http://");
  Serial.println(WiFi.localIP());
}

void webUpdate() {
  WiFiClient client = server.available();
  if (!client) return;
  client.setTimeout(100);

  String request = client.readStringUntil('\n');
  while (client.available()) {
    String header = client.readStringUntil('\n');
    if (header.length() <= 1) break;
  }
  int start = request.indexOf(' ');
  int end = request.indexOf(' ', start + 1);
  String path = (start >= 0 && end > start) ? request.substring(start + 1, end) : "/";

  if (path.startsWith("/mode")) {
    Mode m;
    if (modeFromName(queryValue(path, "m"), m)) setMode(m);
    sendResponse(client, "application/json", statusJson().c_str());
  } else if (path.startsWith("/drive")) {
    String d = queryValue(path, "d");
    if (mode == MODE_MANUAL && d.length() == 1) manualModeCommand(d[0]);
    sendResponse(client, "application/json", statusJson().c_str());
  } else if (path.startsWith("/map")) {
    sendResponse(client, "application/json", mapJson().c_str());
  } else if (path.startsWith("/tune")) {
    String stepValue = queryValue(path, "step");
    String readsValue = queryValue(path, "reads");
    String turnValue = queryValue(path, "turn");
    if (turnValue.length()) turnPercent = constrain(turnValue.toInt(), 10, 100);
    String driveValue = queryValue(path, "drive");
    if (driveValue.length()) drivePercent = constrain(driveValue.toInt(), 10, 100);
    String stopValue = queryValue(path, "stop");
    if (stopValue.length()) stopCm = constrain(stopValue.toInt(), 6, 40);
    String t90Value = queryValue(path, "t90");
    if (t90Value.length()) turn90Ms = constrain(t90Value.toInt(), 200, 6000);
    if (stepValue.length()) mapStep = constrain(stepValue.toInt(), 1, 30);
    if (readsValue.length()) mapReads = constrain(readsValue.toInt(), 1, 5);
    sendResponse(client, "application/json", mapJson().c_str());
  } else if (path.startsWith("/status")) {
    sendResponse(client, "application/json", statusJson().c_str());
  } else if (path.startsWith("/dashboard.css")) {
    sendResponse(client, "text/css", DASHBOARD_CSS);
  } else if (path.startsWith("/dashboard.js")) {
    sendResponse(client, "application/javascript", DASHBOARD_JS);
  } else {
    sendResponse(client, "text/html", DASHBOARD_HTML);
  }
  client.stop();
}
