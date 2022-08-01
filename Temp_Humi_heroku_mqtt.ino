#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ArduinoJson.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <WebSocketsClient.h>
#include <Hash.h>

#define sense D2
#define DHTTYPE DHT11
#define LED 2

float prev_humi = 0.0;
float prev_temp = 0.0;
float temp = 0.0;
float humi = 0.0;

unsigned int starttime1 = millis();
unsigned int endtime1 = starttime1;
unsigned int starttime2 = starttime1;
unsigned int endtime2 = starttime1;
String msg="";

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;

DHT dht(sense, DHTTYPE);

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {


  switch (type) {
    case WStype_DISCONNECTED:
      //Serial.printf("[WSc] Disconnected!\n");
      break;
    case WStype_CONNECTED:
      {
        Serial.println("[WSc] connected!\n");
        // send message to server when Connected
        // webSocket.sendTXT("Connected");
        String data1;
      DynamicJsonDocument doc1(200);
    doc1["type"] = "subscribe";
    doc1["topic"] = "Set";
    serializeJson(doc1, data1);
    webSocket.sendTXT(data1);
    digitalWrite(LED, LOW);
    delay(1);
    digitalWrite(LED, HIGH);
      }
      break;
    case WStype_TEXT:
    {
      digitalWrite(LED, LOW);
    delay(1);
    digitalWrite(LED, HIGH);
    DynamicJsonDocument doc2(200);
    deserializeJson(doc2, payload);
      String msg1 =doc2["message"];
      msg = msg1;
      Serial.println(msg1.c_str());
    }
      // send message to server
      // webSocket.sendTXT("message here");
      break;
    case WStype_BIN:
      hexdump(payload, length);

      // send data to server
      // webSocket.sendBIN(payload, length);
      break;
  }

}

void setup() {
  Serial.begin(115200);
  Serial.println(" ");
  dht.begin();
  pinMode(LED, OUTPUT);
  digitalWrite(LED,HIGH);
  WiFiMulti.addAP("Jio4g", "Live.com");

  //WiFi.disconnect();
  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(100);
  }

  webSocket.beginSSL("riskycase-mqtt.herokuapp.com", 443);
  webSocket.onEvent(webSocketEvent);

}

void loop() {
  humi = dht.readHumidity();
  temp = dht.readTemperature();
  webSocket.loop();
  String data1, data2;
  endtime1 = millis();
  if (temp != prev_temp or (endtime1 - starttime1) >= 7000) {
    DynamicJsonDocument doc1(200);
    doc1["type"] = "message";
    doc1["topic"] = "Temperature";
    doc1["message"] = String(temp)+"°C / "+String((temp*9/5)+32)+"°F";
    serializeJson(doc1, data1);
    webSocket.sendTXT(data1);
    digitalWrite(LED, LOW);
    delay(1);
    digitalWrite(LED, HIGH);
    prev_temp = temp;
    starttime1 = millis();
    endtime1 = starttime1;
  }
  endtime2 = millis();
  if (humi != prev_humi or (endtime2 - starttime2) >= 7000) {
    DynamicJsonDocument doc2(200);
    doc2["type"] = "message";
    doc2["topic"] = "Humidity";
    doc2["message"] = String(humi)+'%';
    serializeJson(doc2, data2);
    webSocket.sendTXT(data2);
    prev_humi = humi;
    digitalWrite(LED, LOW);
    delay(1);
    digitalWrite(LED, HIGH);
    starttime2 = millis();
    endtime2 = starttime2;
  }
}
