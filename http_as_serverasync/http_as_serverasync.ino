/* 
 * Auteur : Zaid Laasri
 * Fichier : http_as_serverasync/http_as_serverasync.ino 

 * Many sources :
 => https://raw.githubusercontent.com/RuiSantosdotme/ESP32-Course/master/code/WiFi_Web_Server_DHT/WiFi_Web_Server_DHT.ino
 => https://randomnerdtutorials.com/esp32-dht11-dht22-temperature-humidity-web-server-arduino-ide/
 => Kevin Levy 
*/
/*====== Import required libraries ===========*/
#include <ArduinoOTA.h>

#include "ArduinoJson.h"

#include <WiFi.h>

#include "ESPAsyncWebServer.h"

#include "wifi_utils.h"

#include "sensors.h"

#include "OneWire.h"

#include "DallasTemperature.h"

#include <Adafruit_NeoPixel.h>

#include "SPIFFS.h"

#include <HTTPClient.h>

#include "routes.h"

#define USE_SERIAL Serial

// Temperature sensor threshold defines
int HIGH_TEMP_THRESHOLD = 27;
int LOW_TEMP_THRESHOLD = 25;

// Light sensor threshold
int LIGHT_THRESHOLD = 2000;

void setup_OTA(); // from ota.ino

// LED strip configuration
#define LED_STRIP_PIN 13
#define NUM_LEDS 5

/*===== ESP GPIO configuration ==============*/
/* ---- LED         ----*/
int greenLedPin = 19; // LE D will use GPIO pin 19
int redLedPin = 21;
int onboardLedPin = 2;

bool greenLed = false;
bool redLed = false;
bool blueLed = false;
bool fire = false;
bool automatique = true;



/* ---- Light       ----*/
const int LightPin = A5; // Read analog input on ADC1_CHANNEL_5 (GPIO 33)
/* ---- Temperature ----*/
OneWire oneWire(23); // Pour utiliser une entite oneWire sur le port 23
DallasTemperature TempSensor( & oneWire); // Cette entite est utilisee par le capteur de temperature

// LED strip initialization
Adafruit_NeoPixel ledStrip(NUM_LEDS, LED_STRIP_PIN, NEO_GRB + NEO_KHZ800);

/*====== ESP Statut =========================*/
// Ces variables permettent d'avoir une representation
// interne au programme du statut "electrique" de l'objet.
// Car on ne peut pas "interroger" une GPIO pour lui demander !
String LEDState = "off";
String last_temp;
String last_light;
int fanSpeed = 0;

/*====== Process configuration ==============*/
// Set timer 
unsigned long loop_period = 10L * 1000; /* =>  10000ms : 10 s */

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Thresholds
short int Light_threshold = 250; // Less => night, more => day

// Host for periodic data report
String target_ip = "";
int target_port = 1880;
int target_sp = 0; // Remaining time before the ESP stops transmitting

/*=======================================================*/
/* Delay can be non compatible with OTA handle or with async !!!! 
 *  because ESP32 pauses your program during the delay(). 
 *  If next OTA request is generated while Arduino is paused waiting for 
 *  the delay() to pass, your program will miss that request.
 *   => Use a non-blocking method, using mills() and a delta to decide to read or not. eg */

void DoSmtg(int delai) {
  static uint32_t tick = 0;
  if (millis() - tick < delai) {
    return;
  } else {
    /* Do stuff here every 10 second */
    USE_SERIAL.print("deja 10 secondes écoulées ! \n");
    tick = millis();
    //*last_temp = atoi(get_temperature(TempSensor).c_str());
    last_temp = get_temperature(TempSensor);
    last_light = get_light(LightPin);
    float ambientTemperature = readAmbientTemperature();
    int ambientLight = readAmbientLightLevel();

    if (automatique == true) {
      handleTemperatureLEDControl(ambientTemperature);
      handleTemperatureLEDStripControl(ambientTemperature);
    }
    handleLightDetectionAction(ambientLight);
    printJsonData(ambientTemperature, ambientLight, greenLed, redLed, blueLed, fanSpeed, fire, automatique);

  }
}

/*============================================*/
/*---- Arduino IDE paradigm : setup+loop -----*/
/*============================================*/
String hostname = "Mon petit objet ESP32";
void setup() {
  /* Serial connection -----------------------*/
  USE_SERIAL.begin(9600);
  while (!USE_SERIAL); //wait for a serial connection  

  /* WiFi connection  -----------------------*/

  wifi_connect_multi(hostname);

  /* WiFi status     --------------------------*/
  if (WiFi.status() == WL_CONNECTED) {
    USE_SERIAL.print("\nWiFi connected : yes ! \n");
    wifi_printstatus(0);
  } else {
    USE_SERIAL.print("\nWiFi connected : no ! \n");
    //  ESP.restart();
  }

  // Init OTA
  //setup_OTA();

  // Initialize the LED 
  setup_led(greenLedPin, OUTPUT, HIGH);
  setup_led(redLedPin, OUTPUT, HIGH);
  pinMode(onboardLedPin, OUTPUT);

  // PWM configuration for the fan: pin 27, channel 0
  ledcAttachPin(27, 0);
  ledcSetup(0, 25000, 8);
  ledcWrite(0, 255); // Maximum speed as default

  // Init temperature sensor 
  TempSensor.begin();

  ledStrip.begin();

  // Initialize SPIFFS
  SPIFFS.begin(true);

  // Setup routes of the ESP Web server
  setup_http_routes( & server);

  // Start ESP Web server
  server.begin();
}

unsigned long lastReportTime = 0;
void loop() {
  int delai = 10000;
  /* OTA*/
  //ArduinoOTA.handle();

  /* Update sensors */
  DoSmtg(delai);

  unsigned long currentTime = millis();
  if (currentTime - lastReportTime >= target_sp * 1000) {
    // Convertissez `sp` en millisecondes
    sendPeriodicReports();
    lastReportTime = currentTime;
  }

  //delay(loop_period); // ms
}

void sendPeriodicReports() {
  if (target_sp > 0) {
    HTTPClient http;
    String serverPath = "http://" + target_ip + ":" + String(target_port) + "/esp";

    // Assurez-vous que la fonction getSensorDataJson retourne une String du JSON des données de capteurs
    float ambientTemperature = readAmbientTemperature();
    int ambientLight = readAmbientLightLevel();
    String jsonData = printJsonData(ambientTemperature, ambientLight, greenLed, redLed, blueLed, fanSpeed, fire, automatique);

    http.begin(serverPath);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(jsonData);
    Serial.print(serverPath);

    if (httpResponseCode == 200) {
      Serial.println("Data sent to Node-RED successfully");
    } else {
      Serial.print(jsonData);
      Serial.print(serverPath);
      Serial.print("Error sending data: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  }
}
