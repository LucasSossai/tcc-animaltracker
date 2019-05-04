#include "heltec.h"
#include "images.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

//Variáveis globais
#define BAND    915E6  // frequência de operação do LoRa (915 MHz)
String rssi = "RSSI --";
String packSize = "--";
String packet ;
String dataString = "";
char auxCharArray[40];
char auxTx[4];
unsigned int totalSending = 0;
unsigned int totalSent = 0;
int scanTime = 3;
int maxDeviceCount = 7;
BLEScan *pBLEScan;


//Funções de iBeacon
void AdvertisingPayLoadReader(uint8_t *payload, size_t payloadSize)
{
  if (totalSending < maxDeviceCount) {
    sprintf(auxCharArray, "%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX;",
            payload[6], payload[7], payload[8], payload[9],
            payload[10], payload[11], payload[12], payload[13],
            payload[14], payload[15], payload[16], payload[17],
            payload[18], payload[19], payload[20], payload[21]);
    sprintf(auxTx, "%02hhi", payload[26]);
    dataString += String(auxCharArray);
    //int wtf = (int)auxTx;
    Serial.println("\n auxTx: " + String(auxTx));
    dataString += String(auxTx);
    dataString += "/";
    totalSending++;
  }
};

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
      Serial.printf("\n Advertised Device: %s \n", advertisedDevice.toString().c_str());
      //Serial.printf("Advertised getPayloadLength: %d  \n", advertisedDevice.getPayloadLength());
      AdvertisingPayLoadReader(advertisedDevice.getPayload(), advertisedDevice.getPayloadLength());
    }
};

void SetupBLE() {
  //BLEAdvertisedDevice myDevice;
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99); // less or equal setInterval value
}


//Funções de Display
void logo() {
  Heltec.display->clear();
  Heltec.display->drawXbm(0, 5, logo_width, logo_height, logo_bits);
  Heltec.display->display();
}

void DisplayStatus() {
  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->drawString(0 , 0 , "Enviado " + String(totalSent) + " pacotes");
  Heltec.display->display();
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

//LoRA
void SetupLora() {
  LoRa.enableCrc();
}

void SendData(String dataToSend) {
  char charBuf[dataToSend.length()];
  dataToSend.toCharArray(charBuf, dataToSend.length());
  Serial.println(charBuf);
  LoRa.beginPacket();
  LoRa.print(charBuf);
  LoRa.endPacket();
}

//Main
void setup()
{
  Heltec.begin(true, true, true, true, BAND);
  Serial.begin(115200);
  SetupDisplay();
  PrintDisplaySuccess();
  SetupBLE();
  SetupLora();
}

void loop()
{
  totalSending = 0;
  dataString = "id:" + String(totalSent) + "/";
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  dataString += "|";

  bool messageArrived = false;
  int auxCounter = 0;
  while (!messageArrived) {
    auxCounter++;
    SendData(dataString);

    if (auxCounter > 5) {
      messageArrived = true;
      totalSent++;
    }
  }
  DisplayStatus();
  pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory
}
