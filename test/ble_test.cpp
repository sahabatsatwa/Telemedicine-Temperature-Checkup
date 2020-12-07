#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <SparkFunMLX90614.h>

#define SERVICE_SENSING (uint16_t)0x181A

BLECharacteristic *pTemperatureCharacteristic;
bool deviceConnected = false;
IRTherm temp;
float objectT;
float ambientT;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

void setup() {
    Serial.begin(115200);
    BLEDevice::init("Temperature Device");

    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    BLEService *pEnvironment = pServer->createService(SERVICE_SENSING);

    pTemperatureCharacteristic = pEnvironment->createCharacteristic(
        BLEUUID((uint16_t)0x2A6E), 
        BLECharacteristic::PROPERTY_READ | 
        BLECharacteristic::PROPERTY_NOTIFY
    );

    pTemperatureCharacteristic->addDescriptor(new BLE2902());
    pEnvironment->start();
    pServer->getAdvertising()->start();
    Serial.println("Wainting a client connection to notify..");
}

void loop() {
    if (temp.read()) {
        objectT = temp.object();
        ambientT = temp.ambient();
    }
    if (deviceConnected) {
        int value = (objectT*100);
        Serial.println(value);
        pTemperatureCharacteristic->setValue((uint8_t*)&value, 2);
        pTemperatureCharacteristic->notify();
    }
    delay(200);   
}