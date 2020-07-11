
#include <FS.h>
#include "max6675.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
//#include <ArduinoOTA.h>
#include <DNSServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <Ticker.h>
#include <ArduinoJson.h>
#include <ESP8266mDNS.h>
//#include <PubSubClient.h>

#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#include "configuration.h"

//todo
//////screen functions
////cook variables to the file system cookLoad and cookSave
//config html page submission
////autorefresh html page
//graph
//chart
////limit temp output size to 3 chars





Ticker ticker;

// Bounce doorSwitch = Bounce();

char jsonStatusMsg[140];

//configuration properties
int tempDif = TEMPDIF;
int fanMin = FANMIN;
//boolean mqttSsl = false;
char emailUsername[150];
char emailPassword[150];
char emailSendTo[150];
int alertOn = ALERTON;
int emailAfter = EMAILAFTER;

char hostname[20];

//SCL and SDA are used for the screen
//SCL D1
//SDA D2

int thermoDO = 12; //D6
int thermoCS0 = 15; //D8
int thermoCS1 = 16; //D0
int thermoCLK = 14; //D5

int fanPin = 13; //D7

//grill
MAX6675 thermocouple0(thermoCLK, thermoCS0, thermoDO);
//meat
MAX6675 thermocouple1(thermoCLK, thermoCS1, thermoDO);

int my_target_grill_temp = 0;
int my_current_grill_temp = 0;
int my_current_meat_temp = 0;
int my_state = 0;
int my_fan_speed = 0;




///lcd
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
  

void setup() {
  Serial.begin(115200);

  //pinMode(16, OUTPUT);
  //pinMode(fanPin, OUTPUT);  // sets the pin as output
  //digitalWrite(LED_BUILTIN,HIGH)



  sprintf (hostname, "smoker");
  
  //slow ticker when starting up
  //switch to fast tick when in AP mode
  ticker.attach(0.6, tick);

  configLoad();

  lcdSetup();

  wifiSetup();

  mdnsSetup();

  webServerSetup();

  ticker.detach();



  cookLoad();
  
  Serial.println("finished setup");
}

void loop() {


   webServerLoop();
   getTempF();
   if (my_state == 1){
    ticker.attach(0.2, tick);
    controlFan();
   }
   else {
    ticker.detach();
    my_fan_speed = 0;
    //fanspeedhere
    analogWrite(fanPin, 0);
   }

   lcdWrite();


   
   delay(1000);
//   digitalWrite(2, HIGH);  // Turn the LED off by making the voltage HIGH
//   delay(1000);
   
}

//#define tempDif 5
//#define fanMin 6

void controlFan(){

  int my_tempDif = my_target_grill_temp - my_current_grill_temp;
  
  if (my_tempDif > tempDif){
    
    my_fan_speed = (sqrt(my_tempDif) + fanMin) * 100;

    if (my_fan_speed > 1023){
      my_fan_speed = 1023;
    }
  }
  else {
    my_fan_speed = 0;
  }
  Serial.println("Temp Difference is ");
  Serial.println(my_tempDif);
  Serial.println("Fan Speed Is ");
  Serial.println(my_fan_speed);
  //fanspeed here
  analogWrite(fanPin, my_fan_speed);
}



void tick() {
  //int state = digitalRead(16);
  //digitalWrite(16, !state);
}

void getTempF(){
  my_current_grill_temp = thermocouple0.readFahrenheit();
  my_current_meat_temp = thermocouple1.readFahrenheit();
  //my_current_meat_temp = thermocouple0.readFahrenheit();

  if (my_current_grill_temp > 999) {
    my_current_grill_temp = 999;
  }
  
  if (my_current_meat_temp > 999) {
    my_current_meat_temp = 999;
  }
}
//
//void toggleFan() {
//  ticker.attach(0.2, tick);
//  Serial.println("toggling fan");
//}

// void sendCurrentDoorStatus(boolean changed) {
//   int doorState = !doorSwitch.read();
//   sprintf (jsonStatusMsg, "{\"status\":%s, \"changed\":%s}", doorState ? "\"CLOSED\"" : "\"OPEN\"", changed ? "true" : "false");

//   mqttSendMsg(jsonStatusMsg);
// }

// void sendDoorStatusOnChange() {
//   boolean changed = doorSwitch.update();

//   //if the button state has changed, current state is recorded
//   if (changed) {
//     ticker.detach();
//     sendCurrentDoorStatus(changed);
//     int doorState = !doorSwitch.read();
//     digitalWrite(ONBOARD_LED, doorState);
//   }

// }

void factoryReset() {
  Serial.println("Restoring Factory Setting....");
  WiFi.disconnect();
  SPIFFS.format();
  ESP.eraseConfig();
  Serial.println("Restarting....");
  delay(500);
  ESP.restart();
}


//Called to save the configuration data after
//the device goes into AP mode for configuration
void configSave() {
  DynamicJsonDocument jsonDoc(1024);
  JsonObject json = jsonDoc.to<JsonObject>();

  //standard properties, always shown when view config
  json["tempDif"] = tempDif;
  json["fanMin"] = fanMin;
  json["emailUsername"] = emailUsername;
  // json["mqttSsl"] = mqttSsl;
  json["emailPassword"] = emailPassword;
  json["emailSendTo"] = emailSendTo;
  json["emailAfter"] = emailAfter;

  //advanced properties, only show in config if set
  // if (strlen(commandTopic))
  //   json["commandTopic"] = commandTopic;
  // if (strlen(statusTopic))
  //   json["statusTopic"] = statusTopic;

  File configFile = SPIFFS.open("/config.json", "w");
  if (configFile) {
    Serial.println("Saving config data....");
    serializeJson(json, Serial);
    Serial.println();
    serializeJson(json, configFile);
    configFile.close();
  }
}

void cookSave() {
  DynamicJsonDocument jsonDoc(64);
  JsonObject json = jsonDoc.to<JsonObject>();

  //save in case of esp8266 reboot
  json["my_target_grill_temp"] = my_target_grill_temp;
  json["my_state"] = my_state;

  File configFile = SPIFFS.open("/cook.json", "w");
  if (configFile) {
    Serial.println("Saving cook data....");
    serializeJson(json, Serial);
    Serial.println();
    serializeJson(json, configFile);
    configFile.close();
  }
}


//Loads the configuration data on start up
void cookLoad() {
  Serial.println("Loading cook data....");
  if (SPIFFS.begin()) {
    if (SPIFFS.exists("/cook.json")) {
      //file exists, reading and loading
      File cookFile = SPIFFS.open("/cook.json", "r");
      if (cookFile) {
        size_t size = cookFile.size();
        std::unique_ptr<char[]> buf(new char[size]);

        cookFile.readBytes(buf.get(), size);

        DynamicJsonDocument jsonDoc(size);
        DeserializationError error = deserializeJson(jsonDoc, buf.get());
        serializeJsonPretty(jsonDoc, Serial);

        JsonObject json = jsonDoc.as<JsonObject>();
        if (json.containsKey("my_target_grill_temp")) {
          my_target_grill_temp = json["my_target_grill_temp"];
        }

        if (json.containsKey("my_state")) {
          my_state = json["my_state"];
        }
      }
    }
  }
}


//Loads the configuration data on start up
void configLoad() {
  Serial.println("Loading config data....");
  if (SPIFFS.begin()) {
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        size_t size = configFile.size();
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);

        DynamicJsonDocument jsonDoc(size);
        DeserializationError error = deserializeJson(jsonDoc, buf.get());
        serializeJsonPretty(jsonDoc, Serial);

        JsonObject json = jsonDoc.as<JsonObject>();
        if (json.containsKey("tempDif")) {
          tempDif = json["tempDif"];
        }

        if (json.containsKey("fanMin")) {
          fanMin = json["fanMin"];
        }

        if (json.containsKey("emailUsername")) {
          strncpy(emailUsername, json["emailUsername"], 150);
        }

        if (json.containsKey("emailPassword")) {
          strncpy(emailPassword, json["emailPassword"], 150);
        }


        if (json.containsKey("emailsendTo")) {
          strncpy(emailSendTo, json["emailSendTo"], 150);
        }


        if (json.containsKey("emailAfter")) {
          fanMin = json["emailAfter"];
        }

        if (json.containsKey("alertOn")) {
          alertOn = json["alertOn"];
        }


      }
    }
  }
}
