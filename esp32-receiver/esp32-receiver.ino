#include <PubSubClient.h>
#include "heltec.h"
#include "images.h"
#include "WiFi.h"
#include "Arduino.h"
#define BAND    915E6  //you can set band here directly,e.g. 868E6,915E6

// Replace the next variables with your SSID/Password combination
const char* ssid = "Asilo 2.4 GHz";
const char* password = "reppolter99";

//// Add your MQTT Broker IP address, example:
const char* mqtt_server = "172.23.224.1";

//
WiFiClient espClient;
PubSubClient client(espClient);

String rssi = "RSSI --";
String packSize = "--";
String packet ;
unsigned int counter = 0;
long lastSendTime = 0;        // last send time
int interval = 1000;          // interval between sends


void logo() {
  Heltec.display->clear();
  Heltec.display->drawXbm(0, 5, logo_width, logo_height, logo_bits);
  Heltec.display->display();
}

void LoRaData() {
  counter++;
  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->drawString(0 , 12 , "Mensagem: ");
  Heltec.display->drawStringMaxWidth(0 , 24 , 128, packet);
  Heltec.display->drawString(0, 0, "Total de UUIDS: " + String(counter));
  Heltec.display->drawString(0, 48, rssi);

  Heltec.display->display();
}

void cbk(int packetSize) {
  packet = "";
  packSize = String(packetSize, DEC);
  for (int i = 0; i < packetSize; i++) {
    packet += (char) LoRa.read();
  }
  rssi = "RSSI " + String(LoRa.packetRssi(), DEC) ;
  LoRaData();
  char charBuf[packetSize];
  packet.toCharArray(charBuf, packetSize);
  client.publish("ble/uuid", charBuf);
}

void WIFISetUp(void)
{
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  Heltec.display->clear();
  WiFi.disconnect(true);
  delay(1000);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoConnect(true);
  Serial.println("Tentando conectar");
  WiFi.begin(ssid, password);
  delay(100);

  byte count = 0;
  while (WiFi.status() != WL_CONNECTED && count < 10)
  {
    count ++;
    delay(500);
    Heltec.display -> drawString(0, 0, "Tentando conectar no wifi...");
    Heltec.display -> display();
  }

  Heltec.display -> clear();
  if (WiFi.status() == WL_CONNECTED)
  {
    Heltec.display -> drawString(0, 0, "Conexão bem sucedida.");
    Heltec.display -> display();
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    //    delay(500);
  }
  else
  {
    Heltec.display -> clear();
    Heltec.display -> drawString(0, 0, "Conexão falhou.");
    Heltec.display -> display();
    while (1);
  }
  Heltec.display -> drawString(0, 10, "Setup do Wifi completo.");
  Heltec.display -> display();
  delay(500);
}
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribeclient.subscribe("ble/uuid");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

}
void setup() {
  //WIFI Kit series V1 not support Vext control

  pinMode(16, OUTPUT);
  pinMode(25, OUTPUT);
  digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
  delay(50);
  digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 in high
  Heltec.begin(true /*DisplayEnable Enable*/, true /*Heltec.Heltec.Heltec.LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);

  Heltec.display->init();
  Heltec.display->flipScreenVertically();
  Heltec.display->setFont(ArialMT_Plain_10);
  logo();
  delay(1500);
  WIFISetUp();
  Heltec.display->clear();

  Heltec.display->drawString(0, 0, "LoRa inicializado!");
  Heltec.display->drawString(0, 10, "Esperando receber dado...");
  Heltec.display->display();
  delay(1000);
  LoRa.receive();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    cbk(packetSize);
  }
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  delay(10);
}
