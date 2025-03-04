#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <ESP32Servo.h>
#include <WiFi.h>

#define SERVO_PIN 26 
Servo servoMotor;  
int pos = 0;
String knownBLENames[] = {"OSHEYNIK_001", "OSHEYNIK_002"};
int RSSI_THRESHOLD = -35;
bool device_found;
int scanTime = 2;
BLEScan* pBLEScan;
const char* ssid     = "111";
const char* password = "11111111";


class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {

    String deviceName = advertisedDevice.getName();

      for (int i = 0; i < (sizeof(knownBLENames) / sizeof(knownBLENames[0])); i++)
      {
        if (deviceName == knownBLENames[i].c_str()) 
                        {
          device_found = true;
          
          Serial.println(advertisedDevice.toString().c_str());
                          break;
                        }
    }
}};
void setup() {
  Serial.begin(115200);
  BLEDevice::init("Feeder001");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);
  servoMotor.setPeriodHertz(50);
  servoMotor.attach(SERVO_PIN);
  servoMotor.write(0);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void loop() {
  BLEScanResults *foundDevices = pBLEScan->start(scanTime, false);
  for (int i = 0; i < foundDevices->getCount(); i++)
  {
    BLEAdvertisedDevice device = foundDevices->getDevice(i);
    int rssi = device.getRSSI();
    if (rssi > RSSI_THRESHOLD && device_found == true)
      {
        Serial.println("its here");
        pos = 180;

  }
      
    else{
    pos = 0;
  }
  }
  Serial.print("pos ");
  Serial.println(pos);
  servoMotor.write(pos);
  pBLEScan->clearResults();
}

