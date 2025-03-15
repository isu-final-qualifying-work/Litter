#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClient.h>
#include <lib/GyverStepper.h>

#define SERVO_PIN 26
#define SSID "****"
#define PASS "********"

Servo servoMotor;  
BLEScan* pBLEScan;
GStepper<STEPPER4WIRE> stepper(2048, 19, 18, 17, 16);

int pos = 0;
int RSSI_THRESHOLD = -50;
int scanTime = 2;
int old_pos = 0;


bool device_found;

String collar;


class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    HTTPClient http;
    WiFiClient client;
    void onResult(BLEAdvertisedDevice advertisedDevice) {

    String deviceName = advertisedDevice.getName();
    if (http.begin(client, "http://192.168.0.103:8000/collar/get_collars_by_litter")){
      http.addHeader("accept", "application/json");
      http.addHeader("Content-Type", "application/json");
      StaticJsonDocument<1> TempDataJSON;
      TempDataJSON["litter_name"] = "LITTER_001";
      String TempDataString;
      serializeJson(TempDataJSON, TempDataString);
      int httpResponseCode = http.POST(TempDataString);
      String result = http.getString();
      Serial.println(result);
      if (httpResponseCode == 200){
        StaticJsonDocument<200> data;
        deserializeJson(data, result);
        for(byte i = 0; i < sizeof(data); i++){
          if (deviceName == data[i]) 
                        {
          device_found = true;
          collar = deviceName;
          Serial.println(advertisedDevice.toString().c_str());
                          break;
                        }
        }
      }
    }
    http.end();
}};

void setup() {
  Serial.begin(115200);
  BLEDevice::init("LITTER_001");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);
  servoMotor.setPeriodHertz(50);
  servoMotor.attach(SERVO_PIN);
  servoMotor.write(0);
  stepper.setSpeed(10);
  WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void loop() {
  old_pos = pos;
  device_found = false;
  HTTPClient http;
  WiFiClient client;
  BLEScanResults *foundDevices = pBLEScan->start(scanTime, false);
  for (int i = 0; i < foundDevices->getCount(); i++)
  {
    BLEAdvertisedDevice device = foundDevices->getDevice(i);
    int rssi = device.getRSSI();
    if (rssi > RSSI_THRESHOLD && device_found == true)
      {
        Serial.println("its here");
        pos = 180;
        break;

  }
      
    else{
    pos = 0;

  }
  }
  Serial.print("pos ");
  Serial.println(pos);
  servoMotor.write(pos);
  if (old_pos == 180 && pos == 0){
    if (http.begin(client, "http://192.168.0.103:8000/activity/litter_clean_activity")){
      http.addHeader("accept", "application/json");
      http.addHeader("Content-Type", "application/json");
      StaticJsonDocument<1> TempDataJSON;
      TempDataJSON["litter_name"] = "LITTER_001";
      TempDataJSON["collar_name"] = collar;
      String TempDataString;
      serializeJson(TempDataJSON, TempDataString);
      int httpResponseCode = http.POST(TempDataString);
      String result = http.getString();
    }
    http.end();
    clean();
  }
  pBLEScan->clearResults();
}

void clean() {
  if (!stepper.tick()) {
    stepper.setTargetDeg(90);
    delay(1000);
    stepper.setTargetDeg(-90);
  }
}
