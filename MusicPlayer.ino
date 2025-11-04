#include <WiFiManager.h>
#include "persistence.h"
#include "connectBT.h"

const char *htmlroot = "<!DOCTYPE html><html><head><title>Music Player</title></head><body>Hello World!</body></html>";
const char *appName = "MusicPlayer";
WebServer server(80);

String selectedDevice ="";
String selectedVolume = "";
void log(const char *msg, float atTime) {
  Serial.printf("%s at %1.2f\n", msg, atTime);
}
void restServerRouting() {
  server.on("/", HTTP_GET, []() {
    server.send(200, F("text/html"), htmlroot);
  });
}
void handleNotFound() {
  String message = server.uri() + " args ";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ":" + server.arg(i);
    if (server.argName(i) == "device") {
      selectedDevice = server.arg(i);
    }
    if (server.argName(i) == "volume") {
      selectedVolume = server.arg(i);
    }
  }
  Serial.println(message);
  server.send(200, F("text/html") , htmlroot);
}

void setup() {
  WiFi.mode(WIFI_STA);
  Serial.begin(115200);
  initFileSystem();
  WiFiManager wm;
  wm.setHostname(appName);
  if (wm.autoConnect(appName)) {
    const char *indexHtml = readData("/index.html");
    if (indexHtml != NULL) {
      htmlroot = indexHtml;
    }
    restServerRouting();
    server.onNotFound(handleNotFound);
    server.begin();
    Serial.println("connected.");
  }
}
String startedDevice = "";

bool isStarted() {
  return startedDevice == selectedDevice;
}

void loop() {
  server.handleClient();
  if (!selectedDevice.isEmpty() && !selectedVolume.isEmpty() && !isStarted()) {
    startedDevice = selectedDevice;
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    connectBT(startedDevice, selectedVolume);
  }
}
