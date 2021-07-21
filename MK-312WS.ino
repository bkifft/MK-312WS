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

JSONVar values;
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

String template_processor(const String& var)
{
  if (var == "HOSTNAME")
    return hostname;
  return String();
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
    Serial.println("Connected to " + ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    WiFi.disconnect();
    WiFi.softAP(default_ssid.c_str(), default_password.c_str());
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


/*
   rewrite this.
   switch on first letter, split at ?, switch sliders for ?-1, switch mode for first and last letter
*/
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    data[len] = 0;
    message = (char*)data;


    switch (message[0])
    {

      case 's': //slider
        switch (message[7])
        {
          case 'a':
            slider_a = atoi(message.c_str() + 9);
            values["slider_a"] = slider_a;
            mk312_set_a(slider_a);
            break;

          case 'b':
            slider_b = atoi(message.c_str() + 9);
            values["slider_b"] = slider_b;
            mk312_set_b(slider_b);
            break;

          case 'm':
            slider_m = atoi(message.c_str() + 9);
            values["slider_m"] = slider_m;
            mk312_set_ma(slider_m);
            break;
        }
        break;

      case 'm'://mode
        byte newmode;
        switch (message[7]) //first letter of mode
        {
          case 'a'://audio1 to audio3
            switch (message[12])
            {
              case '1':
                newmode = MODE_AUDIO1;
                break;
              case '2':
                newmode = MODE_AUDIO2;
                break;
              case '3':
                newmode = MODE_AUDIO3;
                break;
            }
            break;

          case 'c'://climb, combo
            break;

          case 'i': //intense
            break;

          case 'o': //orgasm
            break;

          case 'p': //phase1 to 3
            break;

          case 'r': //random1 random2 rythm
            break;

          case 's': //stroke split
            break;

          case 't': // toggle torment
            break;

          case 'u'://user1 to user6
            break;

            mk312_set_mode(newmode);
        }
        break;
    }

    /*#define MODE_AUDIO1 0x7c
      #define MODE_AUDIO2 0x7d
      #define MODE_AUDIO3 0x7e
      #define MODE_CLIMB 0x78
      #define MODE_COMBO 0x79
      #define MODE_INTENSE 0x7a
      #define MODE_ORGASM 0x83
      #define MODE_PHASE1 0x85
      #define MODE_PHASE2 0x86
      #define MODE_PHASE3 0x87
      #define MODE_RANDOM1 0x80
      #define MODE_RANDOM2 0x81
      #define MODE_RYTHM 0x7b
      #define MODE_SPLIT 0x7f
      #define MODE_STROKE 0x77
      #define MODE_TOGGLE 0x82
      #define MODE_TORMENT 0x84
      #define MODE_USER1 0x88
      #define MODE_USER2 0x89
      #define MODE_USER3 0x8a
      #define MODE_USER4 0x8b
      #define MODE_USER5 0x8c
      #define MODE_USER6 0x8d
      #define MODE_USER7 0x8e
      #define MODE_WAVES 0x76
    */
    // dutyCycle1 = map(sliderValue1.toInt(), 0, 100, 0, 255);

    json_string = JSON.stringify(values);
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
  /*  mk312_sync();
    mk312_set_a(percent);
    mk312_set_b(percent);
    mk312_set_ma(percent);
  */
}

void setup() {
  Serial.begin(115200);
  init_fs();
  init_preferences();
  init_wifi();
  init_ws();
  init_mk312();

  values["slider_a"] = slider_a;
  values["slider_b"] = slider_b;
  values["slider_m"] = slider_m;
  json_string = JSON.stringify(values);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send(SPIFFS, "/index.html", "text/html", false, template_processor);
  });

  server.on("/config", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send(SPIFFS, "/config.html", "text/html");
  });

  server.on("/config_post", HTTP_POST, [](AsyncWebServerRequest * request)
  {
    if (request->hasParam("ssid", true) && request->getParam("ssid", true)->value() != "" ) ssid = request->getParam("ssid", true)->value();
    if (request->hasParam("password", true) && request->getParam("password", true)->value() != "") password = request->getParam("password", true)->value();
    if (request->hasParam("hostname", true) && request->getParam("hostname", true)->value() != "") hostname = request->getParam("hostname", true)->value();

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
}
