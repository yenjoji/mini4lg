#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "ESPTemplateProcessor.h"

const char* ssid = "esp8266ap";
const char* password = "esp8266ap";

ESP8266WebServer server(80);

//const int forward = 16;
//const int reverse = 17;
const int forward = 4;
const int reverse = 5;

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
    analogWrite(forward, 0);
    analogWrite(reverse, 0);
  }else if(throttleValue < 0){
    //fwd
    analogWrite(forward, 0);
    analogWrite(reverse, abs(throttleValue));
  }else{
    //rev
    analogWrite(forward, throttleValue);
    analogWrite(reverse, 0);
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
  
  analogWriteRange(255);

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
