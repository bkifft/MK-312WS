#include <Arduino.h>
#include "BluetoothSerial.h"
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

const String default_hostname = "MK-312WS";
const String default_ssid = WiFi.macAddress();
const String default_password = "12345678"; //needs to be at least 8 chars long for AP mode
const bool default_bt_mode = false;
const int retry_limit = 10;
const String fw_version = "0.4";
/////////stop changing

const char* hex_table = "0123456789abcdef";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncWebSocket ws_bytes("/ws_bytes");

BluetoothSerial SerialBT;
bool bt_mode;

Preferences preferences;
String ssid;
String password;
String hostname;
String preferences_namespace;

JSONVar values;
String json_string;

void init_bt()
{
 Serial2.begin(19200);  
 SerialBT.begin("MK312");
}

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
  preferences_namespace = WiFi.macAddress();
  preferences_namespace.replace(":", "");
  Serial.println("loading preferences, namespace " + preferences_namespace);
  if (!preferences.begin(preferences_namespace.c_str()))
  {
    Serial.println("error: preferences!");
    ssid = default_ssid;
    password = default_password;
    hostname = default_hostname;
    bt_mode = default_bt_mode;
  }
  else
  {
    ssid = preferences.getString("ssid", default_ssid);
    password = preferences.getString("password", default_password);
    hostname = preferences.getString("hostname", default_hostname);
    bt_mode = preferences.getBool("bt_mode", default_bt_mode);

    preferences.end();
    Serial.println("preferences loaded");
  }

  Serial.print("ssid: ");Serial.println( ssid);
  Serial.print("hostname: ");Serial.println( hostname);
    Serial.print("bt_mode: ");Serial.println( bt_mode);

}


String template_processor(const String& var)
{
  if (var == "HOSTNAME")
  {
    return hostname;
  }
  if (var == "FWVERSION")
  {
    return fw_version;
  }
  
  return String();
}

void init_wifi()
{
  int count = 0;
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
//  Serial.println("Connecting to WiFi ");
  while (++count < retry_limit && WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(1000);
  }
  Serial.println("");
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("Connected to " + ssid);
    Serial.println("IP address: " + WiFi.localIP().toString());
  }
  else
  {
    WiFi.disconnect();
    ssid = ssid.substring(0, 30) + "MK"; //ssid is specced at max 32 characters. neded in case wrong pw to join existing wifi
    WiFi.softAP(ssid.c_str(), default_password.c_str());
    Serial.println("AP-Mode");
    Serial.println("IP address: " + WiFi.softAPIP().toString());
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

void update_knobs()
{
  values["slider_a"] = mk312_get_a();
  values["slider_b"] = mk312_get_b();
  values["slider_m"] = mk312_get_ma();

}
/*
void handleWebSocketMessage_ws_bytes(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo*)arg;

  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {

  }

}
*/
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
    Serial.println("ws-command: " + String(message));
    char c[16];

    switch (message[0])
    {
      case 'r': //ramp
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

      case 'e': //emergency stop
         mk312_all_off();
         values["slider_a"] = mk312_get_a();
         values["slider_b"] = mk312_get_b();
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

      case 'i'://inc
        switch (message[4])
        {
          case 'a':
            mk312_inc_a();
            values["slider_a"] = mk312_get_a();
            break;

          case 'b':
            mk312_inc_b();
            values["slider_b"] = mk312_get_b();
            break;

          case 'm':
            mk312_inc_ma();
            values["slider_m"] = mk312_get_ma();
            break;
        }
        break;

      case 'd'://dec
        switch (message[4])
        {
          case 'a':
            mk312_dec_a();
            values["slider_a"] = mk312_get_a();
            break;

          case 'b':
            mk312_dec_b();
            values["slider_b"] = mk312_get_b();
            break;

          case 'm':
            mk312_dec_ma();
            values["slider_m"] = mk312_get_ma();
            break;
        }
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

/*void onEvent_ws_bytes(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
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
*/

void init_ws()
{
  ws.onEvent(onEvent_ws);
  server.addHandler(&ws);
 // ws_bytes.onEvent(onEvent_ws_bytes);
 // server.addHandler(&ws_bytes);
}


void setup() {
  values["slider_a"] = 0;
  values["slider_b"] = 0;
  values["slider_m"] = 0;
  values["mode"] = "0x76";
  values["adc"] = "on";
  values["battery"] = 0;
  //values["ramp_level"] = 5;
  //values["ramp_time"] = 5;
  json_string = JSON.stringify(values);

  Serial.begin(115200);
  pinMode(0, OUTPUT);
  pinMode(2, OUTPUT);
  init_fs();
  init_preferences();
  init_wifi();

if (bt_mode)
{
  init_bt();
   server.on("/", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send(SPIFFS, "/btmode.html", "text/html", false, template_processor);
  });
}
else
{
  delay(1000);
  init_mk312_easy();
  init_ws();


  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send(SPIFFS, "/index.html", "text/html", false, template_processor);
  });
}
  server.on("/config", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send(SPIFFS, "/config.html", "text/html", false, template_processor);
  });

  server.on("/config_post", HTTP_POST, [](AsyncWebServerRequest * request)
  {
    if (request->hasParam("ssid", true)) ssid = request->getParam("ssid", true)->value();
    if (request->hasParam("password", true)) password = request->getParam("password", true)->value();
    if (request->hasParam("hostname", true)) hostname = request->getParam("hostname", true)->value();
    if (request->hasParam("bt_mode", true)) //only triggers if checked
    {
      bt_mode = true;
    }
    else
    {
      bt_mode = false;
    }
/*int args = request->args();
for(int i=0;i<args;i++){
  Serial.printf("ARG[%s]: %s\n", request->argName(i).c_str(), request->arg(i).c_str());
  
}*/
    Serial.println("new ssid: " + ssid);
    Serial.println("new hostname: " + hostname);

    preferences.begin(preferences_namespace.c_str());
    if (ssid.length() > 0 ) preferences.putString("ssid", ssid);
    if (password.length() > 0 ) preferences.putString("password", password);
    if (hostname.length() > 0 ) preferences.putString("hostname", hostname);
    preferences.putBool("bt_mode", bt_mode);

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
  AsyncElegantOTA.begin(&server);//, ssid.c_str(), password.c_str());//FIXME: reenable update login
  server.begin();
  digitalWrite(0, HIGH);
  digitalWrite(2, HIGH);
}


void loop() {
  char c;
  if (bt_mode)
  //no locking required, bt mode is 1:1 for now
  {
    if (Serial2.available()) {
      c = Serial2.read();
      SerialBT.write(c);
     // Serial.print("in  ");
     // Serial.println(c, HEX);
      
    }
    if (SerialBT.available()) {
      c = SerialBT.read();
      Serial2.write(c);
    //  Serial.print("out ");
     // Serial.println(c, HEX);
      
    }
  }
  else
  {
      ws.cleanupClients();
  }
}
