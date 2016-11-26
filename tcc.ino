//TCC code
//Carrega a bibliotecas 
#include <Ultrasonic.h>
#include <ESP8266WiFi.h>
#include "DHT.h"
#include "ArduinoJson.h"
#include <ESP8266HTTPClient.h>
#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>
  #include <stdarg.h>
#include <string.h>

//Define os pinos para o trigger e echo e sensor
#define DHTTYPE DHT22
#define pino_trigger 12
#define pino_echo 13

// Define a rede
const char* ssid = "delta01";
const char* password = "21543105";

// Web Server na porta 80
WiFiServer server(80);

// Sensor DHT no pino D1 = 5
const int DHTPin = 5;

// Inicia o sensor DHT e o de distância
DHT dht(DHTPin, DHTTYPE);
Ultrasonic ultrasonic(pino_trigger, pino_echo);

// Temporary variables
static char celsiusTemp[7];
static char humidityTemp[7];

// iniciando wifi 
void setup() {
  // Initializing serial port for debugging purposes
  Serial.begin(115200);
  delay(10);
  dht.begin();
  
  // Connecting to WiFi network
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi conectado");
  
  // Iniciando Web Server
  server.begin();
  Serial.println("Web server está rodando. Aguardando o IP da placa ESP8266...");
  delay(10000);
  
  // Mostra o endereço IP
  Serial.println(WiFi.localIP());
}

void loop() {
  // Listenning for new clients
  WiFiClient client = server.available();
  
  if (client) {
    float cmMsec, inMsec;
    long microsec;

    while (client.connected()) {
      if (client.available()) {
        cmMsec = ultrasonic.convert(microsec, Ultrasonic::CM);
        inMsec = ultrasonic.convert(microsec, Ultrasonic::IN);
        microsec = ultrasonic.timing();

        // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
        float h = dht.readHumidity();
        
        // Read temperature as Celsius (the default)
        float t = dht.readTemperature();
        
        // Check if any reads failed and exit early (to try again).
        if (isnan(h) || isnan(t)) {
          Serial.println("Falha ao ler o sensor!");
          strcpy(celsiusTemp,"Falha");
          strcpy(humidityTemp, "Falha");
        } else {
          // Computes temperature values in Celsius + Humidity
          float hic = dht.computeHeatIndex(t, h, false);       
          dtostrf(hic, 6, 2, celsiusTemp);            
          dtostrf(h, 6, 2, humidityTemp);

          Serial.print("Umidade: ");
          Serial.print(h);
          Serial.print(" %\t Temperatura: ");
          Serial.print(t);
          Serial.print(" *C ");
          Serial.print("Distancia em cm: ");
          Serial.print(cmMsec);
          Serial.print(" - Distancia em polegadas: ");
          Serial.println(inMsec);
        }

        //String request = String("{\"sensorId\":") + String(1) + String(",\"distance\":") + String(cmMsec) + String("}");
        String request = String("{\"sensorId\":") + String(2) + String(",\"temperature\":") + String(t) + String(",\"humidity\":") + String(h) + String("}");
        HTTPClient http;
        http.begin("http://52.67.179.27:8080/sensor/history");
        http.addHeader("Content-Type", "application/json"); 
        http.POST(request);
        http.writeToStream(&Serial);
        http.end();
      }
      delay(30000);
    }  

    Serial.println("Client disconnected.");
  }
}
