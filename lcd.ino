

void lcdSetup(){

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();

  // Draw a single pixel in white
  display.drawPixel(10, 10, SSD1306_WHITE);

  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display.display();
  delay(2000);
  display.clearDisplay();

  //display.setFont(&FreeMono9pt7b);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);


}

void lcdWifiBroadcast(String msg){
   Serial.print("Broadcasting SSID");
   display.clearDisplay();
   display.setTextSize(1);
   display.setCursor(0, 0);
   display.print("Connect to AP ");
   display.print("\n");
   display.print(msg);
   
   display.display(); 
}

void lcdWrite(){

   Serial.print("F = ");
   Serial.println(my_current_grill_temp);
   display.clearDisplay();
   display.setTextSize(2);
   display.setCursor(0, 0);
   display.print("Grill ");
   display.print(my_current_grill_temp);
   display.print("\n");
   display.print("Meat ");
   display.print(my_current_meat_temp);
   display.print("\n");
   display.setTextSize(1);
   display.print("\n");
   //display.print("Status: ");
   display.print("Power: ");
   if (my_state == 1){
    display.print("On ");
    display.print("Target: ");
    display.print(my_target_grill_temp);
   }
   else {
    display.print("Off ");
   }
   display.print("\n");
   display.print("IP:");
   display.print(WiFi.localIP());
   //Serial.print(WiFi.localIP());
   display.display(); 
   
}
