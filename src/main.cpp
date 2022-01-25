//Librerias necesarias

#include <Arduino.h>
#include <OneWire.h>                
#include <DallasTemperature.h>
#include <PubSubclient.h>
#include <WiFi.h>
#include<esp_task_wdt.h>

// ******Requerido para perro guardian***************
//3 seconds WDT
#define WDT_TIMEOUT 5

//Variables
unsigned long tiempo1 = 0;
unsigned long tiempo2 = 0;

OneWire ourWire(4);                //Se establece el pin 2  como bus OneWire
DallasTemperature sensors(&ourWire); //Se declara una variable u objeto para nuestro sensor

// Configuracion variables WatchDog

//Datos Conexión Wifi
const char* ssid = "TU_SSID"; // Nombre de la red WiFi
const char* password = "TU_PASSWORD"; // Contraseña de la red WiFi

// Funcion conexion Wifi
void connectToWiFi() { // Funcion conexion Wifi

  // Conexión con la red WiFi
  Serial.print("conectando a la  Wifi  ");
  Serial.print(ssid);

  WiFi.mode(WIFI_STA);   // Configuración en modo cliente

  WiFi.begin(ssid, password);   // Iniciar conexión con la red WiFi

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Conexión realizada en ");
  Serial.println(ssid);
  
}

// Configuracion cliente MQTT
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient); 
char mqttServer[] ="TU SERVIDOR MQTT";
int mqttPort = 1883;

//Funcion de CallBack MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
}
void setupMQTT() {
  mqttClient.setServer(mqttServer, mqttPort);
  // set the callback function
  mqttClient.setCallback(callback);
}

//Funcion conectar al Broker MQTT
void reconnect() {
  Serial.println("Connectando al MQTT Broker...");
  while (!mqttClient.connected()) {
      Serial.print("intentando conexion con el MQTT Broker..");
      Serial.println(mqttServer);
      String clientId = "ESP32Client-";
      clientId += String(random(0xffff), HEX);
      
      if (mqttClient.connect(clientId.c_str())) {
        Serial.print("Connectado a ");
        Serial.println(mqttServer);
        // subscribirse al topic
        mqttClient.subscribe("TU TOPIC MQTT");
      }
      
  }
}




void setup() {

delay(500);
Serial.begin(9600);
connectToWiFi();
sensors.begin();   //Se inicia el sensor
setupMQTT();
if (!mqttClient.connected())

    reconnect();

Serial.println("Configurando perro Guardian...");
esp_task_wdt_init(WDT_TIMEOUT, true); //habilitar el perro guardian
esp_task_wdt_add(NULL); //agregar hilo actual al reloj WDT

}

void loop() {
esp_task_wdt_reset(); //Se resetea al 
sensors.requestTemperatures();   //Se envía la orden para leer la temperatura
float temp= sensors.getTempCByIndex(0); //Se obtiene la temperatura en ºC
delay(2500);

if (!mqttClient.connected())
    reconnect();
  mqttClient.loop();


       
tiempo2 = millis();

if(tiempo2 > (tiempo1+6000)) {
  // envio de datos
  temp = sensors.getTempCByIndex(0);

  // Publishing data throgh MQTT
  char array[7];
  sprintf(array, "%f", temp);
  mqttClient.publish("cuina/nevera1/temperatura", array);

  }
}
