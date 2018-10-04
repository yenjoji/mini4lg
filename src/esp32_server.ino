#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "ESPTemplateProcessor.h"

const char* ssid = "esp32ap";
const char* password = "esp32apesp32ap";

WebServer server(80);

const int forward = 16;
const int reverse = 17;

void handleRoot() {
  ESPTemplateProcessor(server).send(String("/index.html"), noopProcessor, '_');
}

String noopProcessor(const String& key) {
  return "";
}

void handleFiles() {
  ESPTemplateProcessor(server).send(String(server.arg("file")), noopProcessor);
}

void handleThrottle() {
  
  int throttleValue = server.arg("value").toInt(); 
  Serial.println(server.arg("value"));
  if( throttleValue == 0 ){
    //brake
    ledcWrite(0, 0);
    ledcWrite(1, 0);
  }else if(throttleValue < 0){
    //fwd
    ledcWrite(0, 0);
    ledcWrite(1, abs(throttleValue));
  }else{
    //rev
    ledcWrite(0, throttleValue);
    ledcWrite(1, 0);
  }
  server.send(200, "text/plain", "OK");
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup(void) {
  SPIFFS.begin();
  
  ledcSetup(0, 5000, 8);
  ledcAttachPin(forward, 0);

  ledcSetup(1, 5000, 8);
  ledcAttachPin(reverse, 1);

  Serial.begin(115200);

  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  Serial.println("");

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/throttle", handleThrottle);

  server.on("/files", handleFiles);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
}
