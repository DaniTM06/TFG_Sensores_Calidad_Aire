#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHTesp.h"
#include <ArduinoJson.h>
#ifdef ESP32
#pragma message(THIS EXAMPLE IS FOR ESP8266 ONLY!)
#error Select ESP8266 board.
#endif
DHTesp dht;


///DATOS CLIENTE ///

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
 
///////////////// Datos Wifi ///////////////////////////


const char* ssid = "MOVISTAR_6878";
const char* password = "C4DC9667A6CACC4CD6FF";
const char* mqtt_server = "broker.mqttdashboard.com"; 

void setup_wifi() {
  
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);       

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
}

////////////////ESTRUCTURA DE DATOS /////////////////////


struct registro_datos {
  double temperatura;
  double humedad;
  double aire;
  };

  String serializa_JSON1 (struct registro_datos datos)
{
  StaticJsonDocument<300> jsonRoot;
  String jsonString;
 
  JsonObject DHT11=jsonRoot.createNestedObject("DHT11");
  DHT11["temp"] = datos.temperatura;
  DHT11["hum"] = datos.humedad;
  jsonRoot["aire"]=datos.aire;
    
  serializeJson(jsonRoot,jsonString);
  return jsonString;
}

struct registro_datos informacion;

/////////////////////////    Funcion Reconnect    ////////////////////////

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(),("",""),("tfg/datos/conexion",0,true,"desconectado"))) {
      Serial.println("connected");
      client.publish("tfg/datos/conexion","conectado");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
/////////////////////////    Funcion setup    ////////////////////////
void setup() {
  dht.setup(2, DHTesp::DHT11); // Connect DHT sensor to GPIO 5
  
  Serial.begin(115200);
  String thisBoard = ARDUINO_BOARD;
  Serial.println(thisBoard);

  setup_wifi();

  client.setServer(mqtt_server, 1883);
  //client.setCallback(callback);


  uint8_t macAddr[6];
  WiFi.macAddress(macAddr);
  Serial.printf("Tu MAC es: %02X:%02X:%02X:%02X:%02X:%02X\n", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
}



///////////////////LOOP PRINCIPAL ////////////////////
void loop()
{

    if (!client.connected()) {
    reconnect();
  }
  client.loop();


    double hum = dht.getHumidity();
    double tem = dht.getTemperature();
    double air = analogRead(A0);
    
    informacion.humedad = hum;
    informacion.temperatura = tem;
    informacion.aire=air;

    Serial.println("Datos actualizados:");
    Serial.println(serializa_JSON1(informacion));
    client.publish("tfg/datos", serializa_JSON1(informacion).c_str()); // "/datos"
    Serial.println("Datos publicados");

delay(30000);
}
