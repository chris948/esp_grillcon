ESP8266WebServer server(80);








void webServerSetup() {
  Serial.println("Starting web server on port 80");
  server.on("/", handleStatus);
  server.on("/OnOff", handleCookSubmit);
  //  server.on("/version", HTTP_GET, handleVersion);
  server.on("/config", handleConfig);
  server.on("/configSubmit", handleConfigSubmit);
  

  server.begin();
}

void webServerLoop() {
  //Serial.println("in webserverloop");
  server.handleClient();
}

void handleCookSubmit() {
  my_target_grill_temp = server.arg("grillTemp").toInt();
  my_state = server.arg("state").toInt();

  Serial.println("temp is ");
  Serial.println(my_target_grill_temp);
  Serial.println("state is ");
  Serial.println(my_state);

  cookSave();

  String message = "<!DOCTYPE html><html><head><title>Changes Received</title> <meta http-equiv = 'refresh' content = '2; url = /' /> </head><body><div id='main'><h1>Processing Changes</h1>";

  server.send(200, "text/html", message);
}

void handleStatus() {
  //String header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\nRefresh: 5\r\n\r\n";
  String title = "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='20'><title>Smoker Control</title></head><body><div id='main'><h1>Smoker Control</h1>";  
  
  String message = title;

  message += "<h2> Grill Temp: ";
  message += String(my_current_grill_temp);
  message += "&deg </h2> <p>";
  //message += buildOnForm(my_target_grill_temp, my_state);
  if (my_fan_speed > 0){
    message += "<h2> Fan On ";
    message += "&deg </h2> <p>";
  }
  message += "<form action='OnOff'> <h2> Set Grill Target ";
  message += "<input type='text' name='grillTemp' ​maxlength='3' size='3' value='";
  message += String(my_target_grill_temp);
  //°
  message += "'>";
  //onForm += "&deg";
  message += "<p>";
  if (my_state == 1) {
    message += "<input type='radio' name='state' value='1' checked>On <input type='radio' name='state' value='0'>Off";
  }
  else {
    message += "<input type='radio' name='state' value='1'>On <input type='radio' name='state' value='0' checked>Off";
  }

  message += "</h2><input type='submit' value='Submit'></form>";
  
  server.send(200, "text/html", message);

}


String buildOnForm(int target, int state) {
  String onForm = "<form action='OnOff'> <h2> Set Grill Target ";
  onForm += "<input type='text' name='grillTemp' ​maxlength='3' size='3' value='";
  onForm += String(target);
  //°
  onForm += "'>";
  //onForm += "&deg";
  onForm += "<p>";
  if (state == 1) {
    onForm += "<input type='radio' name='state' value='1' checked>On <input type='radio' name='state' value='0'>Off";
  }
  else {
    onForm += "<input type='radio' name='state' value='1'>On <input type='radio' name='state' value='0' checked>Off";
  }

  onForm += "</h2><input type='submit' value='Submit'></form>";
  return onForm;
}

void handleConfig() {

  String message = "<!DOCTYPE html><html><head><title>Smoker Control Config</title></head><body><div id='main'><h1>Smoker Control Config</h1>";
  message += "<h2> Temp Dif: ";
  message += String(tempDif);
  message += "<p> Fan Min: ";
  message += String(tempDif);
  message += "<p> Email Username: ";
  message += String(emailUsername);
  message += "<p> Email Password: ";
  message += String(emailPassword);
  message += "<p> Email SendTo: ";
  message += String(emailSendTo);
  message += "<p> Email After Mins: ";
  message += String(emailAfter);
  message += "<p> Email Alert On: ";
  message += String(alertOn);
  server.send(200, "text/html", message);
}

void handleConfigSubmit() {
  int argCount = server.args();
  for (int i = 0; i < argCount; i++) {
    String argName = server.argName(i);
    String argValue = server.arg(i);

    if (argName == "tempDif") {
      tempDif = argValue.toInt();
    } else if (argName == "fanMin") {
      fanMin = argValue.toInt();
    } else if (argName == "emailUsername") {
      argValue.toCharArray(emailUsername, 150);
    } else if (argName == "emailPassword") {
      argValue.toCharArray(emailPassword, 150);
    } else if (argName == "emailSendTo") {
      argValue.toCharArray(emailSendTo, 150);
    } else if (argName == "emailAfter") {
      emailAfter == argValue.toInt();
    } else if (argName == "alertOn") {
      alertOn == argValue.toInt();
    }
  }

  configSave();

  configLoad();
}
