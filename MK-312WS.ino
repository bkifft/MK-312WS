#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <Arduino_JSON.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <AsyncElegantOTA.h>
//#include <BluetoothSerial.h>
#include "logger.h"


#include "mk312.h"

//////////change here

const String default_hostname = "MK-312WS";
const String default_ssid = WiFi.macAddress();
const String default_password = "12345678"; //needs to be at least 8 chars long for AP mode
const int retry_limit = 10;
const int task_delay_ms = 500;

/////////sgtop changing

const char* hex_table = "0123456789abcdef";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncWebSocket ws_bytes("/ws_bytes");


Preferences preferences;
String ssid;
String password;
String hostname;
String preferences_namespace;


char log_out[LOG_SIZE + 1];


JSONVar values;
String json_string;


void init_fs()
{
  if (!SPIFFS.begin())
  {
    log(String("error: fs"));
  }
  else
  {
    log(String("fs running"));
  }
}

void init_preferences()
{
  preferences_namespace = WiFi.macAddress();
  preferences_namespace.replace(":","");
  log(String("loading preferences, namespace ") + preferences_namespace);
  if (!preferences.begin(preferences_namespace.c_str(),true))
  {
    //serial.println("error: preferences!");
    ssid = default_ssid;
    password = default_password;
    hostname = default_hostname;
  }
  else
  {
    ssid = preferences.getString("ssid", default_ssid);
    password = preferences.getString("password", default_password);
    hostname = preferences.getString("hostname", default_hostname);
    preferences.end();
    log(String("preferences loaded"));
  }
  log(String("ssid: ") + ssid);
  log(String("hostname: ") + hostname);
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
  log(String("Connecting to WiFi "));
  while (++count < retry_limit && WiFi.status() != WL_CONNECTED)
  {
    log(String('.'));
    delay(1000);
  }
  //serial.println("");
  if (WiFi.status() == WL_CONNECTED)
  {
    log(String("Connected to ") + ssid);
    log(String("IP address: ") + WiFi.localIP());
  }
  else
  {
    WiFi.disconnect();
    WiFi.softAP(ssid.c_str(), default_password.c_str());
    log(String("AP-Mode"));
    log(String("IP address: ") + WiFi.softAPIP());
  }
  /*use mdns for host name resolution*/
  if (!MDNS.begin(hostname.c_str()))
  {
    //serial.println("error: mdns");
  }
  else
  {
    //serial.println("mdns running: " + hostname + ".local");
  }
}

void update_knobs()
{
  values["slider_a"] = mk312_get_a();
  values["slider_b"] = mk312_get_b();
  values["slider_m"] = mk312_get_ma();

}

void handleWebSocketMessage_ws_bytes(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo*)arg; 

  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    
  }

}
void handleWebSocketMessage_ws(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  int slider;
  char* message;
  byte newmode;

  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    data[len] = 0;
    message = (char*)data;
    //serial.println(message);
    log(String("ws-command: ") + String(message));
    char c[16];

    switch (message[0])
    {
      case 'r':
        switch (message[5])
        {
          case 'l':
       //     values["ramp_level"] = mk312_get_ramp_level();

            break;

          case 't':
         //   values["ramp_time"] = mk312_get_ramp_time();

            break;
          case 's':
            mk312_ramp_start();
            break;
        }
        break;

      case 's': //slider
        slider = atoi(message + 9);
        switch (message[7])
        {
          case 'a':
            mk312_set_a(slider);
            values["slider_a"] = mk312_get_a();
            break;

          case 'b':
            mk312_set_b(slider);
            values["slider_b"] = mk312_get_b();
            break;

          case 'm':
            mk312_set_ma(slider);
            values["slider_m"] = mk312_get_ma();
            break;
        }
        break;

      case 'm'://mode
        newmode = strtol(message + 5, NULL, 16);
        mk312_set_mode(newmode);

        //this puts the mode char as human readable 0x notation into c[]
        c[0] = mk312_get_mode();
        c[1] = 'x';
        c[2] = hex_table[c[0] >> 4];
        c[3] = hex_table[c[0] & 0xf];
        c[0] = '0';
        c[4] = '\0';
        values["mode"] = c;
        break;

      case 'a'://adc
        if (message[5] == 'n')//on
        {
          mk312_enable_adc();
        }
        else
        {
          mk312_disable_adc();
        }
        values["adc"] = mk312_get_adc_disabled() ? "off" : "on";
        update_knobs();
        break;

        case 'b'://bytes
        
        break;
        
    }

    values["battery"] = mk312_get_battery_level();
    json_string = JSON.stringify(values);
    ws.textAll(json_string);
  }
}

void onEvent_ws(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type)
  {
    case WS_EVT_CONNECT:
      //serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      //serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage_ws(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}
void onEvent_ws_bytes(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type)
  {
    case WS_EVT_CONNECT:
      //serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      //serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage_ws_bytes(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void init_ws()
{
  ws.onEvent(onEvent_ws);
  server.addHandler(&ws);
  ws_bytes.onEvent(onEvent_ws_bytes);
server.addHandler(&ws_bytes);
  }


void setup() {
  Serial.end();
  //pinMode(LED_BUILTIN, OUTPUT);
  init_fs();
  init_preferences();
  init_wifi();
  init_ws();
  init_mk312_easy();

  values["slider_a"] = 0;
  values["slider_b"] = 0;
  values["slider_m"] = 0;
  values["mode"] = "0x76";
  values["adc"] = "on";
  values["battery"] = 0;
  //values["ramp_level"] = 5;
  //values["ramp_time"] = 5;

  json_string = JSON.stringify(values);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send(SPIFFS, "/index.html", "text/html", false, template_processor);
  });

  server.on("/config", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send(SPIFFS, "/config.html", "text/html");
  });

  server.on("/debug", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    dump_log(log_out, LOG_SIZE);
    request->send(200, "text/plain", log_out);
  });

  server.on("/config_post", HTTP_POST, [](AsyncWebServerRequest * request)
  {
    if (request->hasParam("ssid", true) && request->getParam("ssid", true)->value() != "" ) ssid = request->getParam("ssid", true)->value();
    if (request->hasParam("password", true) && request->getParam("password", true)->value() != "") password = request->getParam("password", true)->value();
    if (request->hasParam("hostname", true) && request->getParam("hostname", true)->value() != "") hostname = request->getParam("hostname", true)->value();

    //serial.println("ssid: " + ssid);
    //serial.println("hostname: " + hostname);

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
