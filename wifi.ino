bool shouldSaveConfig = false;

/******************************************
 * WifiManager
 ******************************************/
void wifiSetup() {
  Serial.println("Setting up wifi connection....");
  WiFiManager wifiManager;

  char apSid[20];
  sprintf (apSid, "smoker_%08X", ESP.getChipId());
  
  wifiManager.setConfigPortalTimeout(300);
  wifiManager.setDebugOutput(false);
  wifiManager.setAPCallback(wifiConfigModeCallback);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  if (!wifiManager.autoConnect(apSid)) { 
    Serial.println("Failed to connect, trying again...");
    ESP.restart();
  }

  if (shouldSaveConfig) {
    configSave();
  }

}

//call by WifiManager when entering AP mode
void wifiConfigModeCallback (WiFiManager *myWiFiManager) {
  //fast ticker while waiting to config
  ticker.attach(0.2, tick);
  Serial.println("In AP Mode");
  Serial.println(myWiFiManager->getConfigPortalSSID());
  lcdWifiBroadcast(myWiFiManager->getConfigPortalSSID());
}

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Will save config");
  shouldSaveConfig = true;
}
