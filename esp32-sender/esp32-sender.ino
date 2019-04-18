#include "heltec.h"
#include "images.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#define BAND    915E6  //you can set band here directly,e.g. 868E6,915E6

unsigned int counter = 0;
String rssi = "RSSI --";
String packSize = "--";
String packet ;
//String string= "i:001:99999999999999999999999999999999";
char dataString[40];
char auxString[32];
int howManyAreSending = 0;
int scanTime = 5; //In seconds
BLEScan *pBLEScan;

void AdvertisingPayLoadReader(uint8_t *payload, size_t payloadSize)
{
  counter++;

  sprintf(auxString, "%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",
          payload[6], payload[7], payload[8], payload[9],
          payload[10], payload[11], payload[12], payload[13],
          payload[14], payload[15], payload[16], payload[17],
          payload[18], payload[19], payload[20], payload[21]);
  String auxString2 = String(auxString);
  auxString2 += " i: " + String(counter);
  auxString2.toCharArray(dataString, 40);
  //Serial.print("Before send: ");
  //Serial.print(String(howManyAreSending));
  LoRa.beginPacket();
  LoRa.print(auxString2);
  LoRa.endPacket();
  //Serial.print("After send: ");
  //howManyAreSending--;
  //Serial.print(String(howManyAreSending));
}
void LoRaData() {
  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->drawString(0 , 0 , "Enviado " + String(counter) + " UUID's");
  Heltec.display->drawString(0 , 15 , "Ultimo enviado:");
  Serial.print(dataString);
  Heltec.display->drawStringMaxWidth(0 , 30 , 128, String(dataString));
  Heltec.display->display();
}
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
      //howManyAreSending++;
      //Serial.printf("\n Advertised Device: %s \n", advertisedDevice.toString().c_str());
      //Serial.printf("Advertised getPayloadLength: %d  \n", advertisedDevice.getPayloadLength());
      AdvertisingPayLoadReader(advertisedDevice.getPayload(), advertisedDevice.getPayloadLength());
    }
};
void logo() {
  Heltec.display->clear();
  Heltec.display->drawXbm(0, 5, logo_width, logo_height, logo_bits);
  Heltec.display->display();
}
void setup()
{
  pinMode(16, OUTPUT);
  pinMode(25, OUTPUT);
  digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
  delay(50);
  digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 in high
  //WIFI Kit series V1 not support Vext control
  Heltec.begin(true /*DisplayEnable Enable*/, true /*Heltec.Heltec.Heltec.LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);

  Heltec.display->init();
  Heltec.display->flipScreenVertically();
  Heltec.display->setFont(ArialMT_Plain_10);
  logo();
  delay(1500);
  Heltec.display->clear();
  Heltec.display->drawString(0, 0, "LoRa inicializado com sucesso!");
  Heltec.display->display();
  delay(1000);
  Serial.begin(115200);

  Serial.println("Scanning...");
  BLEAdvertisedDevice myDevice;
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99); // less or equal setInterval value
}

void loop()
{
  // put your main code here, to run repeatedly:
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  Serial.print("Devices found: ");
  Serial.println(foundDevices.getCount());

  Serial.println("Scan done!");
  pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory
  LoRaData();
  delay(scanTime);
}
