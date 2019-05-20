// Código desenvolvido por Lucas Sossai em 2019 para TCC.
// Email para contato: lucas.sossai2@gmail.com
// A principal função da estação coletora é scanear sinais BLE , processar o dado e enviar via LoRa para a estação central
#include "heltec.h"
#include "images.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#define BAND    915E6  // frequência de operação do LoRa (915 MHz)
#define TIMEWAITINGRECEIVE 500

//Variáveis globais
long lastSendTime = 0;
String rssi = "RSSI --";
String packSize = "--";
String packet ;
String dataString = "";
char auxCharArray[80];
char auxTx[4];
unsigned int totalSending = 0;
unsigned int totalSent = 0;
int scanTime = 3;
BLEScan *pBLEScan;
bool messageArrived;
char buf[3];
const int maxSize = 80;
char result0[maxSize] = "";
char result1[maxSize] = "";
char result2[maxSize] = "";
char result3[maxSize] = "";
char result4[maxSize] = "";
char result5[maxSize] = "";
char *pointer[6];

//Funções da BLE
void AdvertisingPayLoadReader(uint8_t *payload, unsigned char rssi)
{
  if (totalSending < 5) {
    sprintf(auxCharArray, "%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX,",
            payload[6], payload[7], payload[8], payload[9],
            payload[10], payload[11], payload[12], payload[13],
            payload[14], payload[15], payload[16], payload[17],
            payload[18], payload[19], payload[20], payload[21]);
    String auxDataString = String(auxCharArray);
    int auxRssi = int(rssi) - 256;
    auxDataString += String(auxRssi);
    auxDataString += "/";
    auxDataString.toCharArray(pointer[totalSending], auxDataString.length() + 1);
    Serial.println("Added this string: " + auxDataString + "in this index: pointer[" + totalSending + "]");
    totalSending++;
  }
};

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
      //Serial.printf("\n Advertised Device: %s \n", advertisedDevice.toString().c_str());
      AdvertisingPayLoadReader(advertisedDevice.getPayload(), advertisedDevice.getRSSI());
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

// Funções que envolvem LoRa
void SetupLora() {
  LoRa.enableCrc();
  LoRa.receive();
}

void SendData(String dataToSend) {
  char charBuf[dataToSend.length() + 1];
  dataToSend.toCharArray(charBuf, dataToSend.length() + 1);
  Serial.println("Sending this data via LoRa: " + dataToSend);
  LoRa.beginPacket();
  LoRa.print(charBuf);
  LoRa.endPacket();
}

void receive(String id) {
  String packet = "";
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    for (int i = 0; i < packetSize; i++) {
      packet += (char) LoRa.read();
    }
    if (packet == id) {
      messageArrived = true;
    }
  }
}
//**************************MAIN******************************
void setup()
{
  Heltec.begin(true, true, true, true, BAND);
  Serial.begin(115200);
  SetupDisplay();
  PrintDisplaySuccess();
  SetupBLE();
  SetupLora();
  pointer[0] = result0;
  pointer[1] = result1;
  pointer[2] = result2;
  pointer[3] = result3;
  pointer[4] = result4;
  pointer[5] = result5;

}

void loop()
{
  totalSending = 0;
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  Serial.println("Devices that will be sent:");
  Serial.println("0 : " + String(pointer[0]));
  Serial.println("1 : " + String(pointer[1]));
  Serial.println("2 : " + String(pointer[2]));
  Serial.println("3 : " + String(pointer[3]));
  Serial.println("4 : " + String(pointer[4]));
  int i = 0;
  int maxCounter = totalSending;
  for ( i = 0; i < maxCounter; i++) {
    sprintf(buf, "%02d", totalSent);
    dataString = String(buf) + "/";
    messageArrived = false;
    while (!messageArrived) {

      String msg = String(dataString) + String(pointer[totalSending - 1]);
      SendData(msg);
      lastSendTime = millis();
      while (millis() < lastSendTime + TIMEWAITINGRECEIVE) {
        receive(dataString.substring(0, 2));
        delay(50);
      }
    }
    totalSent++;
    if (totalSent == 100) {
      totalSent = 0;
    }
    totalSending--;
  }
  String clearString = "";
  clearString.toCharArray(result0, clearString.length() + 1);
  clearString.toCharArray(result1, clearString.length() + 1);
  clearString.toCharArray(result2, clearString.length() + 1);
  clearString.toCharArray(result3, clearString.length() + 1);
  clearString.toCharArray(result4, clearString.length() + 1);
  clearString.toCharArray(result5, clearString.length() + 1);
  DisplayStatus();
  pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory
}
