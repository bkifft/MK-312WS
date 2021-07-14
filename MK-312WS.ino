#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <Preferences.h>
#include "mypages.h"
#include "mk312.h"

const String default_hostname = "MK312-WS";
const String default_ssid = "asdfg";
const String default_password = "12345678"; //needs to be at least 8 chars long for AP mode
const int retry_limit = 10;

///////////////////////////////////////
WebServer server(80);
Preferences preferences;
String ssid;
String password;
String hostname;
const String preferences_namespace = default_hostname;
char mk312_a = 0;
char mk312_b = 0;
char mk312_ma = 0;

void setup(void) {
  int retry_count = 0;

  Serial.begin(115200);
  Serial2.begin(19200);
  Serial.println("Serials up");
  preferences.begin(preferences_namespace.c_str(), true);
  ssid = preferences.getString("ssid", default_ssid);
  password = preferences.getString("password", default_password);
  hostname = preferences.getString("hostname", default_hostname);
  preferences.end();

  Serial.print("loaded ssid password hostname: ");
  Serial.print(ssid + " ");
  Serial.print(password + " ");
  Serial.println(hostname);

  WiFi.begin(ssid.c_str(), password.c_str());
  Serial.println("connecting to AP");
  while (WiFi.status() != WL_CONNECTED && retry_count++ <= retry_limit)
  {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    WiFi.disconnect();
    WiFi.softAP(ssid.c_str(), password.c_str());
    Serial.println("AP-Mode");
    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());
  }
  /*use mdns for host name resolution*/
  if (!MDNS.begin(hostname.c_str())) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }

  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", rootIndex);
  });
  server.on("/config", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", configIndex);
  });

  server.on("/config_post", HTTP_POST, []() {
    if (server.arg("ssid") != "") ssid = server.arg("ssid");
    if (server.arg("password") != "") password = server.arg("password");
    if (server.arg("hostname") != "") hostname = server.arg("hostname");

    Serial.println("ssid: " + ssid);
    Serial.println("password: " + password);
    Serial.println("hostname: " + hostname);

    preferences.begin(preferences_namespace.c_str(), false);
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
    preferences.putString("hostname", hostname);
    preferences.end();
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", "OK");
  });

  server.on("/reset", HTTP_GET, []() {
    preferences.begin(preferences_namespace.c_str(), false);
    preferences.clear();
    preferences.end();
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", "OK");
  });

  /////////////////update////////////////////////////////////////////////
  server.on("/update", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", updateIndex);
  });

  server.on("/update_post", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  /////////////////////end update////////////////////////////////////

  server.begin();
}

void loop(void) {
  server.handleClient();
  delay(1);
}
