#include <NimBLEDevice.h>
#include "mbedtls/md.h"

/* ===== UUIDs (MATCH ANDROID) ===== */
#define SERVICE_UUID "12345678-1234-1234-1234-1234567890ab"
#define CHALLENGE_UUID "11111111-2222-3333-4444-555555555555"
#define RESPONSE_UUID  "66666666-7777-8888-9999-aaaaaaaaaaaa"
#define STATUS_UUID    "bbbbbbbb-cccc-dddd-eeee-ffffffffffff"

/* ===== RELAY PIN ===== */
#define RELAY_PIN 26
unsigned long relayOffTime = 0;
bool relayActive = false;

/* ===== SECURITY ===== */
static uint32_t counter = 1;
static const char* SECRET = "PHANTOM_SECRET";

/* ===== BLE OBJECTS ===== */
NimBLECharacteristic *challengeChar;
NimBLECharacteristic *responseChar;
NimBLECharacteristic *statusChar;

/* ===== HMAC FUNCTION ===== */
String hmacSHA256(String msg) {
  byte output[32];
  mbedtls_md_context_t ctx;
  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);
  mbedtls_md_hmac_starts(&ctx, (const unsigned char*)SECRET, strlen(SECRET));
  mbedtls_md_hmac_update(&ctx, (const unsigned char*)msg.c_str(), msg.length());
  mbedtls_md_hmac_finish(&ctx, output);
  mbedtls_md_free(&ctx);

  char buf[65];
  for (int i = 0; i < 32; i++) {
    sprintf(buf + i * 2, "%02x", output[i]);
  }
  return String(buf);
}

/* ===== CHALLENGE CALLBACK ===== */
class ChallengeCallback : public NimBLECharacteristicCallbacks {
  void onRead(NimBLECharacteristic* c, NimBLEConnInfo& connInfo) override {
    Serial.println("Challenge READ request received");
    c->setValue(String(counter));
  }

  void onSubscribe(NimBLECharacteristic* c, NimBLEConnInfo& connInfo, uint16_t subValue) override {
    Serial.println("Client subscribed to CHALLENGE, sending first notification");
    c->setValue(String(counter));
    c->notify();
  }
};

/* ===== RESPONSE CALLBACK ===== */
class ResponseCallback : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic *c, NimBLEConnInfo &connInfo) override {
    String received = c->getValue().c_str();
    String expected = hmacSHA256(String(counter));

    Serial.println("Received: " + received);
    Serial.println("Expected: " + expected);

    if (received == expected) {
      counter++;   // advance rolling code
      Serial.println("AUTH SUCCESS");

      // Send next challenge to client
      challengeChar->setValue(String(counter));
      challengeChar->notify();

      statusChar->setValue("AUTH_OK");
      digitalWrite(RELAY_PIN, LOW); // Unlock relay
      relayOffTime = millis() + 2000;
      relayActive = true;
    } else {
      Serial.println("AUTH FAIL");
      statusChar->setValue("AUTH_FAIL");
    }

    statusChar->notify();
  }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Starting PhantomKey BLE...");

  // Relay setup
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);

  // Initialize BLE
  NimBLEDevice::init("PhantomKey");
  NimBLEServer *server = NimBLEDevice::createServer();
  NimBLEService *service = server->createService(SERVICE_UUID);

  // Challenge characteristic (READ + NOTIFY)
  challengeChar = service->createCharacteristic(
    CHALLENGE_UUID,
    NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
  );
  challengeChar->setCallbacks(new ChallengeCallback());
  challengeChar->createDescriptor("2902", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
  challengeChar->setValue(String(counter)); // initial value

  // Response characteristic (WRITE)
  responseChar = service->createCharacteristic(
    RESPONSE_UUID,
    NIMBLE_PROPERTY::WRITE
  );
  responseChar->setCallbacks(new ResponseCallback());

  // Status characteristic (NOTIFY)
  statusChar = service->createCharacteristic(
    STATUS_UUID,
    NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
  );
  statusChar->createDescriptor("2902", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);

  service->start();

  // Start advertising
  NimBLEAdvertising *adv = NimBLEDevice::getAdvertising();
  adv->addServiceUUID(SERVICE_UUID);
  adv->setName("PhantomKey");
  NimBLEDevice::startAdvertising();

  Serial.println("BLE Rolling Code Ready");
}

void loop() {
  if (relayActive && millis() > relayOffTime) {
    digitalWrite(RELAY_PIN, HIGH);
    relayActive = false;
  }
}
