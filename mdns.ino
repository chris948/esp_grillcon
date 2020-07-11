
/******************************************
 * mDNS
 ******************************************/
void mdnsSetup() {
  Serial.println("Starting mDNS server....");
  MDNS.begin(hostname);
  MDNS.addService("socket", "tcp", 80);
}

