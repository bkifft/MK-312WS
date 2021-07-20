#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <Arduino_JSON.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <AsyncElegantOTA.h>
#include "mk312.h"

//////////change here

//#define FORCE_DEFAULT

const String default_hostname = "MK-312WS";
const String default_ssid = "asdfg";
const String default_password = "12345678"; //needs to be at least 8 chars long for AP mode
const int retry_limit = 10;

/////////


AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

Preferences preferences;
String ssid;
String password;
String hostname;
const String preferences_namespace = default_hostname;
String message = "";
int slider_a = 0;
int slider_b = 0;
int slider_m = 0;

JSONVar slider_values;
String json_string;


void init_fs()
{
  if (!SPIFFS.begin())
  {
    Serial.println("error: fs");
  }
  else
  {
    Serial.println("fs running");
  }
}

void init_preferences()
{
  Serial.println("loading preferences, namespace " + preferences_namespace);
  if (!preferences.begin(preferences_namespace.c_str()))
  {
    Serial.println("error: preferences!");
    ssid = default_ssid;
    password = default_password;
    hostname = default_hostname;
  }
  else
  {
    ssid = preferences.getString("ssid", default_ssid);
    password = preferences.getString("password", default_password);
    hostname = preferences.getString("hostname", default_hostname);
#ifdef FORCE_DEFAULT
    ssid = default_ssid;
    password = default_password;
    hostname = default_hostname;
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
    preferences.putString("hostname", hostname);
#endif //FORCE_DEFAULT
    preferences.end();
    Serial.println("preferences loaded");
  }
  Serial.println("ssid: " + ssid);
  Serial.println("password: " + password);
  Serial.println("hostname: " + hostname);
}

void init_wifi()
{
  int count = 0;
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  Serial.print("Connecting to WiFi ");
  while (++count < retry_limit && WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(1000);
  }
  Serial.println("");
  if (WiFi.status() == WL_CONNECTED)
  {
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
  if (!MDNS.begin(hostname.c_str()))
  {
    Serial.println("error: mdns");
  }
  else
  {
    Serial.println("mdns running: " + hostname + ".local");
  }
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    data[len] = 0;
    message = (char*)data;
    if (message.indexOf("_a?") >= 0)
    {
      slider_a = atoi(message.c_str()+message.indexOf("_a?")+3); //yep, i'm lazy....
    }
    if (message.indexOf("_b?") >= 0)
    {
      slider_b = atoi(message.c_str()+message.indexOf("_b?")+3);
    }

    if (message.indexOf("_m?") >= 0)
    {
      slider_m = atoi(message.c_str()+message.indexOf("_m?")+3);
    }

   
     // dutyCycle1 = map(sliderValue1.toInt(), 0, 100, 0, 255);
      
    slider_values["slider_a"] = slider_a;
    slider_values["slider_b"] = slider_b;
    slider_values["slider_m"] = slider_m;
    json_string = JSON.stringify(slider_values);
    ws.textAll(json_string);

  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type)
  {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void init_ws()
{
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void update_mk312()
{
  //  Serial.println("mk312 stuff):
}

void setup() {
  Serial.begin(115200);
  init_fs();
  init_preferences();
  init_wifi();
  init_ws();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) 
  {
    request->send(SPIFFS, "/index.html", "text/html");
  });
  server.on("/config", HTTP_GET, [](AsyncWebServerRequest * request) 
  {
    request->send(SPIFFS, "/config.html", "text/html");
  });

  server.on("/config_post", HTTP_POST, [](AsyncWebServerRequest * request) 
  {
    if (request->hasParam("ssid",true) && request->getParam("ssid",true)->value() != "" ) ssid = request->getParam("ssid",true)->value();
    if (request->hasParam("password",true) && request->getParam("password",true)->value() != "") password = request->getParam("password",true)->value();
    if (request->hasParam("hostname",true)&& request->getParam("hostname",true)->value() != "") hostname = request->getParam("hostname",true)->value();
    
    Serial.println("ssid: " + ssid);
    Serial.println("password: " + password);
    Serial.println("hostname: " + hostname);

    preferences.begin(preferences_namespace.c_str(), false);
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
    preferences.putString("hostname", hostname);
    preferences.end();
    request->send(200, "text/plain", "CONFIG UPDATED");
  });

  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest * request) 
  {
    preferences.begin(preferences_namespace.c_str(), false);
    preferences.clear();
    preferences.end();
    request->send(200, "text/plain", "RESET DONE");
  });

  server.serveStatic("/", SPIFFS, "/");
  AsyncElegantOTA.begin(&server, ssid.c_str(), password.c_str());
  server.begin();

}

void loop() {
  ws.cleanupClients();
  update_mk312();
}
