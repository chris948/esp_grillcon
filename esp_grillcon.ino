

extern "C" 
{
#include "user_interface.h"
} 

#include <FS.h>
#include "max6675.h"
#include <SPI.h>
#include <Wire.h>
//#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
//#include <ArduinoOTA.h>
#include <DNSServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>
#include <ESP8266mDNS.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h> 
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#include "configuration.h"

//todo
//email/text alerts?
//break up html file size


#define ALLOCATED_RAM 33520

unsigned long timer = 0;
unsigned long writeTimer = 0;

WiFiUDP ntpUDP;

// You can specify the time server pool and the offset, (in seconds)
// additionaly you can specify the update interval (in milliseconds).
// NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);
NTPClient timeClient(ntpUDP, "north-america.pool.ntp.org", -18000, 3600);

//char jsonStatusMsg[140];

//configuration properties
int tempDif = TEMPDIF;
int fanMin = FANMIN;
//char emailUsername[150];
//char emailPassword[150];
//char emailSendTo[150];
//int alertOn = ALERTON;
//int emailAfter = EMAILAFTER;

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

//graph data
int count = 0;
String *timeStamp;
int *grillTemp;
int *meatTemp;

///lcd
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
  

void setup() {
  Serial.begin(115200);

  sprintf (hostname, "smoker");
  //load config file
  configLoad();
  //start lcd screen
  lcdSetup();
  //start wifi
  wifiSetup();
  mdnsSetup();
  webServerSetup();
  //get ntp time
  timeClient.begin();
  //timers for the loop
  timer = millis();
  writeTimer = millis();
  //determine free space and initialize arrays
  allocateRam();
  //load previous cook data if possible
  cookLoad();
  
  Serial.println("finished setup");
}//finish setup

void loop() {

   
   webServerLoop();

   //get temps and update variables for LCD and website based on timer value
   if (millis()>=timer){
    timer = millis() + 3000;

    getTempF();
     if (my_state == 1){
      controlFan();
     }
     else {
      my_fan_speed = 0;
      analogWrite(fanPin, 0);
     }

     lcdWrite();
   } 
}//finish loop

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

  Serial.print("Max Temp Difference is ");  
  Serial.println(tempDif);
  Serial.print("Temp Difference is ");
  Serial.println(my_tempDif);
  Serial.print("Fan Speed Is ");
  Serial.println(my_fan_speed);
  //fanspeed here
  analogWrite(fanPin, my_fan_speed);
}


void getTempF(){

  my_current_grill_temp = thermocouple0.readFahrenheit();
  my_current_meat_temp = thermocouple1.readFahrenheit();
  

  if (my_current_grill_temp > 999) {
    my_current_grill_temp = 999;
  }
  if (my_current_meat_temp > 999) {
    my_current_meat_temp = 999;
  }

  if (millis() >= writeTimer) {
    
    //ntp time
    timeClient.update();
    
    Serial.println(timeClient.getFormattedTime());
  
    writeTimer = millis() + 600000; // 5 mins = 300000 millis
    timeStamp[count] = timeClient.getFormattedTime();  //Current time
    if (my_current_grill_temp < 999) {
      grillTemp[count] = my_current_grill_temp;
    }
    if (my_current_meat_temp < 999) {
      meatTemp[count] = my_current_meat_temp;
    }

    Serial.print(" writing to memory - current time ");
    Serial.println(timeStamp[count]);
    Serial.print("grill temp ");
    Serial.println(grillTemp[count]);
    Serial.print("meat temp ");
    Serial.println(meatTemp[count]);
    Serial.print("counter is ");
    Serial.println(count);

    count++;
  }
}


void allocateRam(){
  //Get free RAM in bytes
  uint32_t free=system_get_free_heap_size() - ALLOCATED_RAM;
  unsigned long numberOfRows=0;

  
  //Divide the free RAM by the size of the variables used to store the data. 
  //This will allow us to work out the maximum number of records we can store. 
  //All while keeping some RAM free which is specified in ALLOCATED_RAM
  numberOfRows = free / (sizeof(int)*2+ sizeof(unsigned long)); // 2 x int for temp and pressure.  Long for time. 

  Serial.print("Space for ");
  Serial.print(numberOfRows);
  Serial.println(" rows");

  //re-declare the arrays with the number of elements
  grillTemp = new int [numberOfRows];
  meatTemp = new int [numberOfRows];
  timeStamp = new String [numberOfRows];

}

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
//  json["emailUsername"] = emailUsername;
//  json["mqttSsl"] = mqttSsl;
//  json["emailPassword"] = emailPassword;
//  json["emailSendTo"] = emailSendTo;
//  json["emailAfter"] = emailAfter;

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
//
//        if (json.containsKey("emailUsername")) {
//          strncpy(emailUsername, json["emailUsername"], 150);
//        }
//
//        if (json.containsKey("emailPassword")) {
//          strncpy(emailPassword, json["emailPassword"], 150);
//        }
//
//
//        if (json.containsKey("emailsendTo")) {
//          strncpy(emailSendTo, json["emailSendTo"], 150);
//        }
//
//
//        if (json.containsKey("emailAfter")) {
//          fanMin = json["emailAfter"];
//        }
//
//        if (json.containsKey("alertOn")) {
//          alertOn = json["alertOn"];
//        }


      }
    }
  }
}
