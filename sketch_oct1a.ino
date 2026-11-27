#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "R200.h"

#define RXPIN 16
#define TXPIN 17

#define PIN_LED_STATUS 2
#define PIN_BUZZER     4

#define DEVICE_NAME         "CattleVision-RFID"
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

R200 rfid;

BLEServer *pServer = nullptr;
BLECharacteristic *pCharacteristic = nullptr;
bool deviceConnected = false;
bool oldDeviceConnected = false;

unsigned long lastReading = 0;
const unsigned long READING_INTERVAL_MS = 200;

uint8_t ultimoUID[12] = {0};
unsigned long tempoUltimoEnvio = 0;
const unsigned long INTERVALO_MESMA_TAG = 3000; // 3 segundos (debounce zootécnico)

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) override {
    deviceConnected = true;
    digitalWrite(PIN_LED_STATUS, HIGH);
    Serial.println("[BLE] Smartphone conectado ao CattleVision-RFID.");
  }

  void onDisconnect(BLEServer* pServer) override {
    deviceConnected = false;
    digitalWrite(PIN_LED_STATUS, LOW);
    Serial.println("[BLE] Smartphone desconectado.");
  }
};

void feedbackLeituraSucesso() {
  digitalWrite(PIN_LED_STATUS, LOW);
  digitalWrite(PIN_BUZZER, HIGH);
  delay(80);
  digitalWrite(PIN_BUZZER, LOW);
  digitalWrite(PIN_LED_STATUS, deviceConnected ? HIGH : LOW);
}

bool uidValido(const uint8_t* uid) {
  for (int i = 0; i < 12; i++) {
    if (uid[i] != 0x00) {
      return true;
    }
  }
  return false;
}

void processarLeitura() {
  if (!uidValido(rfid.uid)) {
    return;
  }

  unsigned long agora = millis();
  bool mesmaTag = true;

  for (int i = 0; i < 12; i++) {
    if (rfid.uid[i] != ultimoUID[i]) {
      mesmaTag = false;
      break;
    }
  }

  if (!mesmaTag || (agora - tempoUltimoEnvio >= INTERVALO_MESMA_TAG)) {
    memcpy(ultimoUID, rfid.uid, 12);
    tempoUltimoEnvio = agora;

    char hexBuffer[25];
    for (int i = 0; i < 12; i++) {
      sprintf(&hexBuffer[i * 2], "%02X", rfid.uid[i]);
    }
    hexBuffer[24] = '\0';

    Serial.print("[RFID] Tag detectada: ");
    Serial.println(hexBuffer);

    feedbackLeituraSucesso();

    if (deviceConnected && pCharacteristic != nullptr) {
      pCharacteristic->setValue((uint8_t*)hexBuffer, 24);
      pCharacteristic->notify();
      Serial.println("[BLE] Notificacao enviada.");
    } else {
      Serial.println("[BLE] Nenhum cliente conectado.");
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(PIN_LED_STATUS, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_LED_STATUS, LOW);
  digitalWrite(PIN_BUZZER, LOW);

  if (!rfid.begin(&Serial2, 115200, RXPIN, TXPIN)) {
    Serial.println("[R200] Erro ao inicializar o leitor R200.");
    while (1);
  }

  BLEDevice::init(DEVICE_NAME);
  BLEDevice::setMTU(64);
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  pCharacteristic->addDescriptor(new BLE2902());

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  Serial.println("[BLE] Servidor GATT iniciado: CattleVision-RFID");
}

void loop() {
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);
    pServer->startAdvertising();
    Serial.println("[BLE] Reiniciando advertising...");
    oldDeviceConnected = deviceConnected;
  }
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }

  rfid.loop();

  if (millis() - lastReading >= READING_INTERVAL_MS) {
    lastReading = millis();
    rfid.poll();
    processarLeitura();
  }

  delay(20);
}