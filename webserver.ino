ESP8266WebServer server(80);

void webServerSetup() {
  Serial.println("Starting web server on port 80");
  server.on("/", handleStatus);
  server.on("/graph", drawGraph);
  server.on("/OnOff", handleCookSubmit);
  //  server.on("/version", HTTP_GET, handleVersion);
  server.on("/config", handleConfig);
  server.on("/configSubmit", handleConfigSubmit);

  server.begin();
}

void drawGraph() {
  
  String message;
  
  message = ("<!DOCTYPE html><html><head><meta http-equiv='refresh' content='20'><title>Cook History</title>\n");
  message += ("<script type = \"text/javascript\" src = \"https://www.gstatic.com/charts/loader.js\"></script>\n");
  message += ("<script type = \"text/javascript\">google.charts.load('current', {packages: ['corechart','line']}); </script> </head> \n");
  //message += "<body><div id='main'><h1>Cook History</h1>";
     message += ("<body><div id = \"container\" style = \"width: 1200px; height: 800px; margin: 0 auto\"></div>\n");
      message += ("<script language = \"JavaScript\">\n");
         message += ("function drawChart() {\n");
            // Define the chart to be drawn.
            message += ("var data = new google.visualization.DataTable();\n");
            message += ("data.addColumn('string', 'Date');\n");
            message += ("data.addColumn('number', 'GrillTemp');\n");
            message += ("data.addColumn('number', 'MeatTemp');\n");
            message += ("data.addRows([\n");

              for (int i = 0; i< rowCount ; i++){
                message += ("['" + String(timeStamp[i]) +  "'," + String(grillTemp[i]) +  "," + String(meatTemp[i]) + "],\n");
                //Serial.println("['" + String(timeStamp[i]) +  "'," + String(grillTemp[i]) +  "," + String(meatTemp[i]) + "],\n");
                
              }
              message += ("]);\n");

              //server.send(200, "text/html", message);

            
            // Set chart options
            message += ("var options = {'title' : 'Current Cook',\n");
               //message += ("hAxis: {\n");
                  //message += ("title: 'Time'},\n");
               // += ("vAxis: {\n");
                  //message += ("title: 'Temperature'},\n") ;  
               message += ("'width':1200,\n");
               message += ("'height':800,\n");
               message += ("curveType: 'function'};\n");

            // Instantiate and draw the chart.
            message += ("var chart = new google.visualization.LineChart(document.getElementById('container'));\n");
            message += ("chart.draw(data, options);}\n");
         message += ("google.charts.setOnLoadCallback(drawChart);</script>\n");
      message += ("<a  href='/'><h2>Main</a></h2></body></html>\n");

     server.send(200, "text/html", message);
}

void webServerLoop() {
  //Serial.println("in webserverloop");
  server.handleClient();
}

void handleStatus() {

  String message;
  
  message = ("<!DOCTYPE html><html><head><meta http-equiv='refresh' content='20'><title>Grill Controller</title>\n");
  message += ("<script type = \"text/javascript\" src = \"https://www.gstatic.com/charts/loader.js\"></script>\n");
  message += ("<script type = \"text/javascript\">google.charts.load('current', {'packages':['gauge']}); google.charts.setOnLoadCallback(drawChart);\n");

        message += ("function drawChart() {\n");

          message += ("var data = google.visualization.arrayToDataTable([\n");
            message += ("['Label', 'Value'],\n");
            message += ("['Grill', " + String(my_current_grill_temp) + "],\n");
            message += ("['Meat', " + String(my_current_meat_temp) + "],\n");
            message += ("['Fan', " + String((my_fan_speed / 10.24)) + "]\n");
          message += ("]);\n");

          message += ("var options = {\n");
            message += ("width: 800, height: 240,\n");
            message += ("redFrom: 325, redTo: 999,\n");
            message += ("yellowFrom:280, yellowTo: 325,\n");
            message += ("max: 350, minorTicks: 5\n");
          message += ("};\n");

          message += ("var chart = new google.visualization.Gauge(document.getElementById('chart_div'));\n");

          message += ("chart.draw(data, options);}</script>\n");
          
          message += ("<style>[type='text'] { font-size: 48px; }</style>\n");

      message += ("</head><body>\n");
      message += ("<div id=\"chart_div\" style=\"width: 800px; height: 240px;\"></div>\n");
    //message += ("</body></html>\n");


  ///////////////////////////////////
  message += "<form action='OnOff'> <h1> Set Grill Target ";
  message += "<input type='text' name='grillTemp' ​maxlength='3' size='3' value='";
  message += String(my_target_grill_temp);
  //°
  message += "'>";
  message += "<p>";
  if (my_state == 1) {
    message += "<input type='radio' style='width:40px; height:40px;' name='state' value='1' checked='yes'>On <input type='radio' style='width:40px; height:40px;' name='state' value='0'>Off";
    Serial.println("loaded website, mystate = 1");
  }
  else {
    message += "<input type='radio' style='width:40px; height:40px;' name='state' value='1'>On <input type='radio' style='width:40px; height:40px;' name='state' value='0' checked='yes'>Off";
    Serial.println("loaded website, mystate = 0");
  }

  message += "<p><input type='submit' style='width:80px; height:40px;' value='Submit'></form>";

  message += ("<a  href='/graph'><h2>Graph</a></h2></div>");
  
  server.send(200, "text/html", message);
}


void handleCookSubmit() {

  Serial.print("Received Cook Submit - temp is ");
  Serial.println(server.arg("grillTemp"));
  Serial.print("Received state is ");
  Serial.println(server.arg("state"));

  my_target_grill_temp = server.arg("grillTemp").toInt();
  my_state = server.arg("state").toInt();

  cookSave();

  String message = "<!DOCTYPE html><html><head><title>Changes Received</title> <meta http-equiv = 'refresh' content = '2; url = /' /> </head><body><div id='main'><h1>Processing Changes</h1>";

  server.send(200, "text/html", message);
}

void handleConfig() {

  String message = "<!DOCTYPE html><html><head><title>Smoker Control Config</title></head><body><div id='main'><h1>Smoker Control Config</h1>";
  message += "<h2> Current Temp Dif: ";
  message += String(tempDif);
  message += "<p> Current Fan Min: ";
  message += String(fanMin);
  message += "<p> Time Zone Offset: ";
  message += String(timeZoneOffset / 3600);
  //message += "<p> Email Username: ";
  //message += String(emailUsername);
  //message += "<p> Email Password: ";
  //message += String(emailPassword);
  //message += "<p> Email SendTo: ";
  //message += String(emailSendTo);
  //message += "<p> Email After Mins: ";
  //message += String(emailAfter);
  //message += "<p> Email Alert On: ";
  //message += String(alertOn);



  ///////////////////////////////////
  message += "<form action='configSubmit'>";
  message += "Temp Dif <input type='text' name='tempdif' ​maxlength='3' size='3' value='";
  message += String(tempDif);
  message += "'><p>";
  message += "Fan Min<input type='text' name='fanmin' ​maxlength='3' size='3' value='";
  message += String(fanMin);
  message += "'><p>";
  message += "Time Zone Offset<input type='text' name='tz' ​maxlength='3' size='3' value='";
  message += String(timeZoneOffset / 3600);
  message += "'><p>";


  message += "<p><input type='submit' style='width:80px; height:40px;' value='Submit'></form>";


    server.send(200, "text/html", message);
}

void handleConfigSubmit() {

Serial.print("Received config submit tempdif is now: ");
Serial.println(server.arg("tempdif"));
Serial.print("Received config submit fanmin is now: ");
Serial.println(server.arg("fanmin"));
tempDif = server.arg("tempdif").toInt();
fanMin = server.arg("fanmin").toInt();
timeZoneOffset = server.arg("tz").toInt() * 3600;
Serial.print("saving timezone offset ");
Serial.println(timeZoneOffset);


  configSave();

  configLoad();

  String message = "<!DOCTYPE html><html><head><title>Changes Received</title> <meta http-equiv = 'refresh' content = '2; url = /config' /> </head><body><div id='main'><h1>Processing Changes</h1>";
  server.send(200, "text/html", message);
}
