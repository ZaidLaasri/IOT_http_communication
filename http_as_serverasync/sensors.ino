/*--------------------------------*/
void setup_led(int LEDpin, int mode, int status){
  pinMode(LEDpin, mode);
  digitalWrite(LEDpin, status);// Set outputs to LOW
}

/*--------------------------------*/
String get_temperature(DallasTemperature tempSensor) {
  /* Renvoie la valeur du capteur de temperature dans une String
   * Attention !!
   *    J'ai enleve le delay mais convertir prend du temps ! 
   *    moins que les requetes Http.
   */
  float t;
  String s;
  tempSensor.requestTemperaturesByIndex(0);
  t = tempSensor.getTempCByIndex(0);
  s = String(t);
  Serial.println("get_temperature() : "+s+" C"); // for debug
  return s;
}
/*--------------------------------*/
String get_light(int LightPin) {
  /* Renvoie la valeur du capteur de lumiere dans une String
   */
  int sensorValue;
  sensorValue = analogRead(LightPin);
  String s = String(sensorValue);
  Serial.println("get_light() : "+s+" Lum"); // for debug
  return s;
}


void handleTemperatureLEDControl(float temperature) {
  digitalWrite(onboardLedPin, LOW);

  if (temperature > HIGH_TEMP_THRESHOLD) {
    digitalWrite(redLedPin, HIGH);
    digitalWrite(greenLedPin, LOW);

    turnFanOnHigh();
    greenLed = false;
    redLed = true;
    blueLed = false;
    fire = true;
  } else if (temperature < LOW_TEMP_THRESHOLD) {
    digitalWrite(greenLedPin, HIGH);
    digitalWrite(redLedPin, LOW);

    turnFanOff();
    greenLed = true;
    redLed = false;
    blueLed = false;
        fire = false;

  } else {
    turnFanOnMedium();
    digitalWrite(greenLedPin, LOW);
    digitalWrite(redLedPin, LOW);
    greenLed = false;
    redLed = false;
    blueLed = true;
    fire = false;

  }
}

// Function to turn the fan on to its highest speed
void turnFanOnHigh() {
  ledcWrite(0, 255); // Set fan speed to maximum
  fanSpeed = 255;
}

// Function to turn the fan on to a medium speed
void turnFanOnMedium() {
  ledcWrite(0, 127); // Set fan speed to medium
  fanSpeed = 127;
}

// Function to turn off the fan
void turnFanOff() {
  ledcWrite(0, 0); // Turn off the fan
  fanSpeed = 0;
}


int readAmbientLightLevel() {
  int lightLevel = analogRead(A5);
  return lightLevel;
}


void handleLightDetectionAction(int lightLevel) {
  if (lightLevel > LIGHT_THRESHOLD) {
    digitalWrite(onboardLedPin, HIGH);
            fire = true;

  } else {
    digitalWrite(onboardLedPin, LOW);
     fire = false;

  }
}


float readAmbientTemperature() {
  TempSensor.requestTemperaturesByIndex(0);
  float temperature = TempSensor.getTempCByIndex(0);
  return temperature;
}

String printJsonData(float temperature, int lightLevel, boolean greenLed, boolean redLed, boolean blueLed, int fanSpeed, boolean fire, bool automatique) {
  
  StaticJsonDocument<300> jsonData;

  
  jsonData["temperature"] = temperature;
  jsonData["lightLevel"] = lightLevel;
  jsonData["greenLedStatus"] = greenLed; 
  jsonData["redLedStatus"] = redLed;     
  jsonData["blueLedStatus"] = blueLed;   
  jsonData["fanSpeed"] = fanSpeed;
  jsonData["fire"] = fire;
jsonData["automatique"] = automatique;
  
JsonObject wifi = jsonData.createNestedObject("wifi");
wifi["SSID"] = WiFi.SSID();        
wifi["macAddress"] = WiFi.macAddress();   
wifi["ipAddress"] = WiFi.localIP().toString(); 



  String jsonStr;
  serializeJson(jsonData, jsonStr);
  return jsonStr;
}

void handleTemperatureLEDStripControl(float temperature) {
  if (temperature > HIGH_TEMP_THRESHOLD) {
    for (int i = 0; i < NUM_LEDS; i++) {
      setLEDStripColor(255, 0, 0); // Red color for high temperature
    }
    turnFanOnHigh();
  } else if (temperature < LOW_TEMP_THRESHOLD) {
    for (int i = 0; i < NUM_LEDS; i++) {
      setLEDStripColor(0, 255, 0); // Green color for low temperature
    }
    turnFanOff();
  } else {
    for (int i = 0; i < NUM_LEDS; i++) {
      setLEDStripColor(0, 0, 255); // Blue color for moderate temperature
    }
    turnFanOnMedium();
  }
}
void setLEDStripColor(int red, int green, int blue) { 
  for (int i = 0; i < NUM_LEDS; i++) {
    ledStrip.setPixelColor(i, ledStrip.Color(red, green, blue));
  }
  ledStrip.show();
}
