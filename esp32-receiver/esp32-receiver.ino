#include <PubSubClient.h>
#include "heltec.h"
#include "images.h"
#include "WiFi.h"
#include "Arduino.h"

#define BAND    915E6  //Frequência de operação do LoRa

// Replace the next variables with your SSID/Password combination
//const char* ssid = "Asilo 2.4 GHz";
//const char* password = "reppolter99";
const char* ssid = "Sofia-quarto";
const char* password = "a1b2c3d4e5";

//// Add your MQTT Broker IP address, example:
const char* mqtt_server = "192.168.1.101";

WiFiClient espClient;
PubSubClient client(espClient);
long lastSendTime = 0;
String rssi = "RSSI --";
String packSize = "--";
String packet ;
String currentId = "";
unsigned int totalReceived = 0;
int interval = 1000;          // interval between sends
int counter = 0;

//Display
void logo() {
  Heltec.display->clear();
  Heltec.display->drawXbm(0, 5, logo_width, logo_height, logo_bits);
  Heltec.display->display();
}

void SendData(String dataToSend) {
  char charBuf[dataToSend.length() + 1];
  dataToSend.toCharArray(charBuf, dataToSend.length() + 1);
  Serial.println(charBuf);
  LoRa.beginPacket();
  LoRa.print(charBuf);
  LoRa.endPacket();
}

void SetupDisplay() {
  pinMode(16, OUTPUT);
  pinMode(25, OUTPUT);
  digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
  delay(50);
  digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 in high
  Heltec.display->init();
  Heltec.display->flipScreenVertically();
  Heltec.display->setFont(ArialMT_Plain_10);
  logo();
  delay(500);
}

void PrintDisplaySuccess()
{
  Heltec.display->clear();
  Heltec.display->drawString(0, 0, "Display inicializado com sucesso!");
  Heltec.display->display();
  delay(500);
}

void LoRaData() {
  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->drawString(0 , 12 , "Mensagem: ");
  Heltec.display->drawStringMaxWidth(0 , 24 , 128, packet);
  Heltec.display->drawString(0, 0, "Total de UUIDS: ");
  Heltec.display->drawString(0, 48, rssi);
  Heltec.display->display();
}

//Lora
void LoraCallback(int packetSize) {
  packet = "";
  packSize = String(packetSize, DEC);
  for (int i = 0; i < packetSize; i++) {
    packet += (char) LoRa.read();
  }
  rssi = "RSSI " + String(LoRa.packetRssi(), DEC) ;
  LoRaData();
  char charBuf[packetSize];
  packet.toCharArray(charBuf, packetSize);
  Serial.println(charBuf);
  Serial.println(String(charBuf).substring(0, 2));
  if (currentId != String(charBuf).substring(0, 2)) {
    currentId = String(charBuf).substring(0, 2);
    CheckMqttConnection();
    totalReceived++;
    PubMqttMessage(String(charBuf));
  }
}

void SetupLora() {
  LoRa.enableCrc();
  LoRa.receive();
}

void LoraListener() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    LoraCallback(packetSize);
    SendData(currentId);
  }
}

//Wifi
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

void ReconnectMqtt() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Tentando reconectar mqtt...");
    // Attempt to connect
    if (client.connect("LoRa-Receiver-Client")) {
      Serial.println("connected");
      //client.subscribe("ble/uuid");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//Mqtt
void mqttCallback(char* topic, byte* message, unsigned int length) {
  Serial.println("Chegou mensagem no tópico: ");
  Serial.println(topic);
  Serial.println(". Mensagem: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
}

void SetupMqtt() {
  client.setServer(mqtt_server, 1883);
  client.setCallback(mqttCallback);
}

void CheckMqttConnection() {
  if (!client.connected()) {
    ReconnectMqtt();
  }
  client.loop();
}

void PubMqttMessage(String msg) {
  char charBuf[msg.length() + 1];
  msg.toCharArray(charBuf, msg.length() + 1);
  client.publish("ble/uuid", charBuf);
}

//**************************MAIN******************************
void setup() {
  Heltec.begin(true, true, true, true, BAND);
  Serial.begin(115200);
  SetupDisplay();
  PrintDisplaySuccess();
  SetupLora();
  WIFISetUp();
  SetupMqtt();
}

void loop() {

  LoraListener();
  if (counter % 100 == 0) {
    Serial.println("Tick");
    Serial.println(String(totalReceived));
  }
  counter++;
  delay(100);

}
