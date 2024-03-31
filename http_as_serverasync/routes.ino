/* 
 * Auteur : G.Menez
 * Fichier : http_as_serverasync/routes.ino 
 */

#include "ESPAsyncWebServer.h"

#include "routes.h"

#include "SPIFFS.h"

#include "wifi_utils.h"


#define USE_SERIAL Serial
extern String last_temp, last_light, hostname;
extern int HIGH_TEMP_THRESHOLD, LOW_TEMP_THRESHOLD, LEDpin, redLedPin;

/*===================================================*/
String processor(const String &
  var) {
  /* Replaces "placeholder" in  html file with sensors values */
  /* accessors functions get_... are in sensors.ino file   */

  //Serial.println(var);
  if (var == "TEMPERATURE") {
    return last_temp;
    /* On aimerait écrire : return get_temperature(TempSensor);
     * mais c'est un exemple de ce qu'il ne faut surtout pas écrire ! 
     * yield + async => core dump ! 
     */
    //return get_temperature(TempSensor);
  } else if (var == "HOSTNAME") {
    return hostname;
  } else if (var == "WHERE") {
    return "Sophia,";
  } else if (var == "LIGHT") {
    return last_light;
  } else if (var == "SSID") {
    return WiFi.SSID();
  } else if (var == "MAC") {
    return WiFi.macAddress();
  } else if (var == "IP") {
    return WiFi.localIP().toString();
  } else if (var == "LT") {
    return String(LOW_TEMP_THRESHOLD);
  } else if (var == "HT") {
    return String(HIGH_TEMP_THRESHOLD);
  } else if (var == "COOLER") {
    return String(greenLedPin);
  } else if (var == "HEATER") {
    return String(redLedPin);
  } else if (var == "UPTIME") {
    return String(millis() / 1000); // Uptime en secondes
  }

  return String(); // parce que => cf doc de asyncwebserver
}

/*===================================================*/
void setup_http_routes(AsyncWebServer * server) {
  /* 
   * Sets up AsyncWebServer and routes 
   */

  // doc => Serve files in directory "/" when request url starts with "/"
  // Request to the root or none existing files will try to server the default
  // file name "index.htm" if exists 
  // => premier param la route et second param le repertoire servi (dans le SPIFFS) 
  // Sert donc les fichiers css !  
  server -> serveStatic("/", SPIFFS, "/").setTemplateProcessor(processor);

  // Declaring root handler, and action to be taken when root is requested
  auto root_handler = server -> on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    /* This handler will download index.html (stored as SPIFFS file) and will send it back */
    request -> send(SPIFFS, "/index.html", String(), false, processor);
    // cf "Respond with content coming from a File containing templates" section in manual !
    // https://github.com/me-no-dev/ESPAsyncWebServer
    // request->send_P(200, "text/html", page_html, processor); // if page_html was a string .   
  });

  server -> on("/temperature", HTTP_GET, [](AsyncWebServerRequest * request) {
    /* The most simple route => hope a response with temperature value */
    USE_SERIAL.printf("GET /temperature request \n");
    /* Exemple de ce qu'il ne faut surtout pas écrire car yield + async => core dump !*/
    //request->send_P(200, "text/plain", get_temperature(TempSensor).c_str());
    request -> send_P(200, "text/plain", last_temp.c_str());
  });

  server -> on("/light", HTTP_GET, [](AsyncWebServerRequest * request) {
    /* The most simple route => hope a response with light value */
    request -> send_P(200, "text/plain", last_light.c_str());
  });

  server -> on("/upTime", HTTP_GET, [](AsyncWebServerRequest * request) {
    /* The most simple route => hope a response with light value */
    request -> send_P(200, "text/plain", String(millis() / 1000).c_str());
  });

  // This route allows users to change thresholds values through GET params
  server -> on("/set", HTTP_GET, [](AsyncWebServerRequest * request) {
    String response = "";

    // Réglage du cooler
    if (request -> hasArg("cool")) {
      automatique = false;
      String coolValue = request -> arg("cool");
      if (coolValue == "on") {
        digitalWrite(greenLedPin, HIGH);
        greenLed = true;

        // Active le cooler
        response += "\n Cooler turned on.\n";
        response += "You've switched to manual mode for LEDs and fan control. ";
        response += "If you want to switch back to automatic mode put the url /set?automatique=true";
      } else if (coolValue == "off") {
        digitalWrite(greenLedPin, LOW);
        greenLed = false;
        // Désactive le cooler
        response += "\n Cooler turned off. \n";
        response += "You've switched to manual mode for LEDs and fan control. ";
        response += "If you want to switch back to automatic mode put the url /set?automatique=true";
      }
    }

    // Réglage du heater
    if (request -> hasArg("heat")) {
      automatique = false;
      String heatValue = request -> arg("heat");
      if (heatValue == "on") {
        redLed = true;
        digitalWrite(redLedPin, HIGH); // Active le heater
        response += "\n Heater turned on. \n";
        response += "You've switched to manual mode for LEDs and fan control. ";
        response += "If you want to switch back to automatic mode put the url /set?automatique=true";
      } else if (heatValue == "off") {
        redLed = false;

        digitalWrite(redLedPin, LOW); // Désactive le heater
        response += "\n Heater turned off. \n";
        response += "You've switched to manual mode for LEDs and fan control. ";
        response += "If you want to switch back to automatic mode put the url /set?automatique=true";
      }
    }

    // Réglage de la vitesse du ventilateur (fan speed)
    if (request -> hasArg("fanspeed")) {
      automatique = false;
      int speedValue = request -> arg("fanspeed").toInt();
      ledcWrite(0, speedValue); // Réglage de la vitesse du ventilateur
      fanSpeed = speedValue; // Mise à jour de la variable globale si nécessaire
      response += "\n Fan speed set to " + String(speedValue) + ". \n";
      response += "You've switched to manual mode for LEDs and fan control. ";
      response += "If you want to switch back to automatic mode put the url /set?automatique=true";
    }

    if (request -> hasArg("ledStrip")) {
      automatique = false;
      String ledStripValue = request -> arg("ledStrip");
      if (ledStripValue == "red") {
        for (int i = 0; i < NUM_LEDS; i++) {
          setLEDStripColor(255, 0, 0); // Red color for high temperature
        } // Active le heater
        response += "\n Red Led Strip. \n";
        response += "You've switched to manual mode for LEDs and fan control. ";
        response += "If you want to switch back to automatic mode put the url /set?automatique=true";

      } else if (ledStripValue == "blue") {
        for (int i = 0; i < NUM_LEDS; i++) {
          setLEDStripColor(0, 0, 255);
        }
        blueLed = true;
        response += "\n blue Led Strip. \n";
        response += "You've switched to manual mode for LEDs and fan control. ";
        response += "If you want to switch back to automatic mode put the url /set?automatique=true";
      } else if (ledStripValue == "green") {
        for (int i = 0; i < NUM_LEDS; i++) {
          setLEDStripColor(0, 255, 0);
        }
        response += "\n green Led Strip. \n";
        response += "You've switched to manual mode for LEDs and fan control. ";
        response += "If you want to switch back to automatic mode put the url /set?automatique=true";
      } else if (ledStripValue == "off") {
        for (int i = 0; i < 12; i++) {
          setLEDStripColor(0, 0, 0);
        }
        response += "\n Led Strip off. \n";
        response += "You've switched to manual mode for LEDs and fan control. ";
        response += "If you want to switch back to automatic mode put the url /set?automatique=true";
      } else {
        response += "\n this color does not exist \n";
      }

    }
    // Réglage du seuil de lumière (light threshold)
    if (request -> hasArg("light_threshold")) {
      Light_threshold = request -> arg("light_threshold").toInt();
      response += "\n Light threshold set to " + String(Light_threshold) + ". \n";
      response += "You've switched to manual mode for LEDs and fan control. ";
      response += "If you want to switch back to automatic mode put the url /set?automatique=true";
    }

    if (request -> hasArg("automatique")) {

      String automatiqueValue = request -> arg("automatique");
      if (automatiqueValue == "true") {
        automatique = true;
        handleTemperatureLEDControl(readAmbientTemperature());
        response += "Automatic mode on. \n";
      } else if (automatiqueValue == "false") {
        automatique = false;
        response += "Automatic mode off. \n";
      }
    }

    if (request -> hasArg("HIGH_TEMP_THRESHOLD")) {
      String HIGH_TEMP_THRESHOLDValue = request -> arg("HIGH_TEMP_THRESHOLD");
      if (HIGH_TEMP_THRESHOLDValue.toInt() > LOW_TEMP_THRESHOLD) {
        HIGH_TEMP_THRESHOLD = HIGH_TEMP_THRESHOLDValue.toInt();
        response += "\n Modified upper threshold \n";
      } else {
        response += "\n Error modifing : high temperature treshold cannot be lower than low temp threshold \n";
      }
    }

    if (request -> hasArg("LOW_TEMP_THRESHOLD")) {
      String LOW_TEMP_THRESHOLDValue = request -> arg("LOW_TEMP_THRESHOLD");
      if (LOW_TEMP_THRESHOLDValue.toInt() < HIGH_TEMP_THRESHOLD) {
        LOW_TEMP_THRESHOLD = LOW_TEMP_THRESHOLDValue.toInt();
        response += "\n Modified lower threshold \n";
      } else {
        response += "\n Error modifing : low temperature treshold cannot be higher than high temp threshold \n";
      }
    }
    // Envoyer la réponse
    if (response.length() > 0) {
      request -> send(200, "text/plain", response);
    } else {
      // Si aucun paramètre valide n'a été fourni, renvoyez une erreur 404
      request -> send(404, "text/plain", "Not Found");
    }
  });

  server -> on("/target", HTTP_POST, [](AsyncWebServerRequest * request) {
    /* A route receiving a POST request with Internet coordinates 
     * of the reporting target host.
     */
    Serial.println("Receive Request for a periodic report !");
    if (request -> hasArg("ip") &&
      request -> hasArg("port") &&
      request -> hasArg("sp")) {
      target_ip = request -> arg("ip");
      target_port = atoi(request -> arg("port").c_str());
      target_sp = atoi(request -> arg("sp").c_str());
    }
    request -> send(SPIFFS, "/index.html", String(), false, processor);
    sendPeriodicReports();
  });

  // If request doesn't match any route, returns 404.
  server -> onNotFound([](AsyncWebServerRequest * request) {
    request -> send(404);
  });

  server -> on("/value", HTTP_GET, [](AsyncWebServerRequest * request) {
    StaticJsonDocument < 200 > jsonDoc;

    // Ajoutez les données à jsonDoc en utilisant la fonction existante
    addToJsonData(jsonDoc, readAmbientTemperature(), readAmbientLightLevel(), greenLed, redLed, blueLed, fanSpeed, fire, automatique);

    // Créez un nouveau JsonDocument pour la réponse
    StaticJsonDocument < 200 > responseJsonDoc;

    // Ajoutez uniquement les clés requises par la requête GET
    if (request -> hasParam("temperature")) {
      responseJsonDoc["temperature"] = jsonDoc["temperature"];
    }
    if (request -> hasParam("light")) {
      responseJsonDoc["light"] = jsonDoc["lightLevel"];
    }
    if (request -> hasParam("redLed")) {
      responseJsonDoc["redLed"] = jsonDoc["redLedStatus"];
    }
    if (request -> hasParam("fanSpeed")) {
      responseJsonDoc["fanSpeed"] = jsonDoc["fanSpeed"];
    }
    if (request -> hasParam("fire")) {
      responseJsonDoc["fire"] = jsonDoc["fire"];
    }
    if (request -> hasParam("automatique")) {
      responseJsonDoc["automatique"] = jsonDoc["automatique"];
    }
    if (request -> hasParam("wifi")) {

      JsonObject wifi = responseJsonDoc.createNestedObject("wifi");
      wifi["SSID"] = WiFi.SSID();
      wifi["macAddress"] = WiFi.macAddress();
      wifi["ipAddress"] = WiFi.localIP().toString();

    }

    if (responseJsonDoc.size() > 0) {
      String jsonResponse;
      serializeJson(responseJsonDoc, jsonResponse);
      request -> send(200, "application/json", jsonResponse);
    } else {
      request -> send(404, "text/plain", "Not Found");
    }
  });

  server -> on("/control", HTTP_GET, [](AsyncWebServerRequest * request) {
     float ambientTemperature = readAmbientTemperature();
 int ambientLight = readAmbientLightLevel();
    String greenLedValue;
    Serial.println("On rentre dans le control");
    Serial.print("URL: ");
    automatique = false;
    Serial.println(request -> url());
    if (request -> hasParam("greenLed")) {
      Serial.println("On rentre dans le control cool");
      greenLedValue = request -> getParam("greenLed") -> value();
      if (greenLedValue == "on") {
        Serial.println("On rentre dans le control cool true");
        for (int i = 0; i < NUM_LEDS; i++) {
          setLEDStripColor(0, 255, 0);
        }
        digitalWrite(greenLedPin, HIGH);
        greenLed = true;
        redLed = false;
        blueLed = false;

        digitalWrite(redLedPin, LOW);

      } else {
        for (int i = 0; i < NUM_LEDS; i++) {
          setLEDStripColor(0, 0, 0);
        }
        digitalWrite(greenLedPin, LOW);
        greenLed = false;
      }
    } else if (request -> hasParam("redLed")) {
      String redLedValue;
      redLedValue = request -> getParam("redLed") -> value();
      if (redLedValue == "on") {
        digitalWrite(redLedPin, HIGH);
        digitalWrite(greenLedPin, LOW);
        for (int i = 0; i < NUM_LEDS; i++) {
          setLEDStripColor(255, 0, 0);
        }
        redLed = true;
        greenLed = false;
        blueLed = false;

      } else {
        redLed = false;
        digitalWrite(redLedPin, LOW);
        for (int i = 0; i < NUM_LEDS; i++) {
          setLEDStripColor(0, 0, 0);
        }
        redLed = false;
      }
    } else if (request -> hasParam("blueLed")) {
      String blueLedValue;
      blueLedValue = request -> getParam("blueLed") -> value();
      if (blueLedValue == "on") {
        for (int i = 0; i < NUM_LEDS; i++) {
          setLEDStripColor(0, 0, 255);
        }
        blueLed = true;
        digitalWrite(greenLedPin, LOW);
        digitalWrite(redLedPin, LOW);
        greenLed = false;
        redLed = false;
      } else {
        for (int i = 0; i < NUM_LEDS; i++) {
          setLEDStripColor(0, 0, 0);
        }
        blueLed = false;
      }
    } else if (request -> hasParam("fanspeed")) {
      String fanspeedValue;
      fanspeedValue = request -> getParam("fanspeed") -> value();
      Serial.println(fanspeedValue);
      fanSpeed = fanspeedValue.toInt();
      ledcWrite(0, fanSpeed);
    } else if (request -> hasParam("automatique")) {

      String automatiqueValue = request -> getParam("automatique") -> value();
      if (automatiqueValue == "true") {
        automatique = true;
        handleTemperatureLEDControl(ambientTemperature);
      } else if (automatiqueValue == "false") {
        automatique = false;
      }
    }
   

    // Envoyez jsonData en tant que réponse de type JSON
    request -> send(200, "application/json", printJsonData(ambientTemperature, ambientLight, greenLed, redLed, blueLed, fanSpeed, fire, automatique));

  });

}
/*===================================================*/

void addToJsonData(StaticJsonDocument < 200 > & jsonData, float temperature, int lightLevel, boolean greenLed, boolean redLed, boolean blueLed, int fanSpeed, boolean fire, bool automatique) {
  jsonData["temperature"] = temperature;
  jsonData["lightLevel"] = lightLevel;
  jsonData["greenLedStatus"] = greenLed;
  jsonData["redLedStatus"] = redLed;
  jsonData["blueLedStatus"] = blueLed;
  jsonData["fanSpeed"] = fanSpeed;
  jsonData["fire"] = fire;
  jsonData["automatique"] = automatique;

}
