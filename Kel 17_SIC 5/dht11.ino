#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>
#include <WiFiGeneric.h>
#include <WiFiMulti.h>
#include <WiFiSTA.h>
#include <WiFiScan.h>
#include <WiFiServer.h>
#include <WiFiType.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include "DHT.h"
#define DHT11PIN 4


DHT dht(DHT11PIN, DHT11);



const char* ssid = "Bintang A";
const char* password = "0204bintang";
const char* mqtt_server = "test.mosquitto.org";
const int port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
float temp = 0;
float hum = 0;

const char* topic_temperature = "/sensor/data/temperature";
const char* topic_humidity = "/sensor/data/humidity";
const char* topic_command = "/sic/command/mqtt";

void setup_wifi() { 
  delay(200);
  
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA); 
  WiFi.begin(ssid, password); 

  while (WiFi.status() != WL_CONNECTED) { 
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) { 
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) { 
    Serial.print((char)payload[i]);
  }
  Serial.println();

  
  if ((char)payload[0] == '1') {
    digitalWrite(2, LOW);   
  } else {
    digitalWrite(2, HIGH);  
  }
}

void reconnect() { 
  
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("Connected");
      
      client.publish("/sic/mqtt", "Hello!"); 
      
      client.subscribe(topic_command); 
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      
      delay(5000);
    }
  }
}

void setup() {
  pinMode(2, OUTPUT);     
  Serial.begin(115200);
  setup_wifi(); 
  client.setServer(mqtt_server, port); 
  client.setCallback(callback); 
  dht.begin();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) { 
    lastMsg = now;
    float humi = dht.readHumidity();
    float temp = dht.readTemperature();

    String temp_string = String(temp, 2); 
    client.publish(topic_temperature, temp_string.c_str()); 
    
    String hum_string = String(humi, 1); 
    client.publish(topic_humidity, hum_string.c_str()); 

    Serial.print("Temperature: ");
    Serial.println(temp);
    Serial.print("Humidity: ");
    Serial.println(humi);
  }
}