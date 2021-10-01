/*
Autour: Furkan Metin OĞUZ
Date:2021
*/

#include <Adafruit_MAX31856.h>

#include "sys/time.h"

#include <Arduino.h>

#include "BLEDevice.h"

#include "BLEUtils.h"

#include "BLEBeacon.h"

#include "BLEAdvertising.h"

#include "BLEEddystoneURL.h"

#include "esp_sleep.h"

#define GPIO_DEEP_SLEEP_DURATION 5
RTC_DATA_ATTR static time_t last;
RTC_DATA_ATTR static uint32_t bootcount;
BLEAdvertising * pAdvertising;
struct timeval nowTimeStruct;

time_t lastTenth;
Adafruit_MAX31856 maxthermo = Adafruit_MAX31856(17, 16, 5, 4);
#define BEACON_UUID "8ec76ea3-6668-48da-9866-75be8bc86f4d"
void setBeacon() {
  char beacon_data[25];
  uint16_t beconUUID = 0xFEAA;
  uint16_t volt = 0;

  float tempFloat = maxthermo.readThermocoupleTemperature();

  int temp = (int)(tempFloat * 256);
  Serial.printf("Converted to 8.8 format %0X%0X\n", (temp >> 8), (temp & 0xFF));

  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  BLEAdvertisementData oScanResponseData = BLEAdvertisementData();

  oScanResponseData.setFlags(0x06);
  oScanResponseData.setCompleteServices(BLEUUID(beconUUID));

  beacon_data[0] = 0x20;
  beacon_data[1] = 0x00;
  beacon_data[2] = 0x00;
  beacon_data[3] = 0x00;
  beacon_data[4] = (temp >> 8);
  beacon_data[5] = (temp & 0xFF);
  beacon_data[6] = ((bootcount & 0xFF000000) >> 24);
  beacon_data[7] = ((bootcount & 0xFF0000) >> 16);
  beacon_data[8] = ((bootcount & 0xFF00) >> 8);
  beacon_data[9] = (bootcount & 0xFF);
  beacon_data[10] = ((lastTenth & 0xFF000000) >> 24);
  beacon_data[11] = ((lastTenth & 0xFF0000) >> 16);
  beacon_data[12] = ((lastTenth & 0xFF00) >> 8);
  beacon_data[13] = (lastTenth & 0xFF);

  oScanResponseData.setServiceData(BLEUUID(beconUUID), std::string(beacon_data, 14));
  oAdvertisementData.setName("FMO1");
  pAdvertising -> setAdvertisementData(oAdvertisementData);
  pAdvertising -> setScanResponseData(oScanResponseData);
}

void setup() {

  Serial.begin(115200);
  while (!Serial) delay(10);

  if (!maxthermo.begin()) {

    while (1) delay(10);
  }

  maxthermo.setThermocoupleType(MAX31856_TCTYPE_K);

  Serial.print("Thermocouple type: ");
  switch (maxthermo.getThermocoupleType()) {
  case MAX31856_TCTYPE_B:
    Serial.println("B Type");
    break;
  case MAX31856_TCTYPE_E:
    Serial.println("E Type");
    break;
  case MAX31856_TCTYPE_J:
    Serial.println("J Type");
    break;
  case MAX31856_TCTYPE_K:
    Serial.println("K Type");
    break;
  case MAX31856_TCTYPE_N:
    Serial.println("N Type");
    break;
  case MAX31856_TCTYPE_R:
    Serial.println("R Type");
    break;
  case MAX31856_TCTYPE_S:
    Serial.println("S Type");
    break;
  case MAX31856_TCTYPE_T:
    Serial.println("T Type");
    break;
  case MAX31856_VMODE_G8:
    Serial.println("Voltage x8 Gain mode");
    break;
  case MAX31856_VMODE_G32:
    Serial.println("Voltage x8 Gain mode");
    break;
  default:
    Serial.println("Unknown");
    break;
  }

  maxthermo.setConversionMode(MAX31856_ONESHOT_NOWAIT);

  gettimeofday( & nowTimeStruct, NULL);

  last = nowTimeStruct.tv_sec;
  lastTenth = nowTimeStruct.tv_sec * 10;

  BLEDevice::init("FMO DENEME");

  BLEDevice::setPower(ESP_PWR_LVL_N12);

  pAdvertising = BLEDevice::getAdvertising();

  setBeacon();

  pAdvertising -> start();

}

void loop() {
  maxthermo.triggerOneShot();
  delay(1000);
  setBeacon();

}