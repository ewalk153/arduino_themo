#include "DHT.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>

#include "secrets.h"

#define DHTTYPE DHT11 // DHT 11

// network
const char* ssid = SSID;
const char* password = PASSWORD;
const char* url = THERMO_URL;

// DHT Sensor
const int DHTPin = 3;
// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);

// Temporary variables
static char celsiusTemp[7];
static char fahrenheitTemp[7];
static char humidityTemp[7];

String clientId = "5"; // make something up

// delay 20 min
const unsigned long postingInterval = 20L * 60L * 1000L; // delay between updates, in milliseconds

unsigned long lastConnectionTime = -postingInterval;            // last time you connected to the server, in milliseconds


void setup() {
  Serial.begin(115200);
  setupSensor();
  wifiConnect();
}

void loop() {
  if (millis() - lastConnectionTime > postingInterval) {
    httpRequest();
  }
}

void setupSensor() {
  dht.begin();
}

void wifiConnect() {
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  // attempt to connect to Wifi network:
  while (WiFi.status() != WL_CONNECTED) {
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void fetchDHT() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f))
  {
    Serial.println("Failed to read from DHT sensor!");
  }
  else {
    // Computes temperature values in Celsius + Fahrenheit and Humidity
    float hic = dht.computeHeatIndex(t, h, false);
    dtostrf(hic, 6, 2, celsiusTemp);
    float hif = dht.computeHeatIndex(f, h);
    dtostrf(hif, 6, 2, fahrenheitTemp);
    dtostrf(h, 6, 2, humidityTemp);
  }
}

void httpRequest() {
  if ((WiFi.status() == WL_CONNECTED)) { //Check the current connection status
    fetchDHT();
    //    dtostrf(22.45, 6, 2, celsiusTemp);

    HTTPClient http;

    http.begin(url, null); //Specify the URL
    http.addHeader("Content-Type", "application/json");
    JSONVar jsonData;
    jsonData["client_id"] = clientId;

    String postTemp = String(celsiusTemp);
    postTemp.trim();
    jsonData["data"] = postTemp;
    String jsonString = JSON.stringify(jsonData);
    int httpCode = http.POST(jsonString);   //Make the request

    if (httpCode > 0) { //Check for the returning code

      String payload = http.getString();
      Serial.println(httpCode);
      Serial.println(payload);

      lastConnectionTime = millis();
    } else {
      Serial.println("Error on HTTP request");
      Serial.println(httpCode);
      delay(5L * 1000L);
    }
    http.end(); //Free the resources
  }
}
