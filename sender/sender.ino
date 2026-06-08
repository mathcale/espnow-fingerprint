#include <esp_now.h>
#include <WiFi.h>
#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>

#include "config.h"

HardwareSerial mySerial(2);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

int8_t RX_PIN = 25;
int8_t TX_PIN = 26;

uint8_t receiverAddrs[] = RECEIVER_MAC;
esp_now_peer_info_t peerInfo;

typedef struct message {
  char state[3];
} struct_message;

struct_message msg;

void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("[DEBUG] Last Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

uint8_t waitForFingerprint() {
  int p = -1;

  Serial.println("[INFO] Waiting for finger...");

  while (p != FINGERPRINT_OK) {
    p = finger.getImage();

    if (p == FINGERPRINT_NOFINGER) continue;

    if (p != FINGERPRINT_OK) {
      Serial.println("[ERR] Image error, retrying...");
    }
  }

  p = finger.image2Tz();

  if (p != FINGERPRINT_OK) {
    Serial.println("[ERR] Could not convert image");
    return 0;
  }

  p = finger.fingerSearch();

  if (p == FINGERPRINT_OK) {
    return finger.fingerID;
  }

  return 0;
}

void setup() {
  Serial.begin(115200);

  mySerial.begin(57600, SERIAL_8N1, RX_PIN, TX_PIN);
  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("[DEBUG] Found fingerprint sensor!");
  } else {
    Serial.println("[ERR] Did not find fingerprint sensor!");

    while (1) {
      delay(1);
    }
  }

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("[ERR] Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(esp_now_send_cb_t(onDataSent));

  memcpy(peerInfo.peer_addr, receiverAddrs, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("[ERR] Failed to add peer");
    return;
  }
}

void loop() {
  uint8_t matchedId = waitForFingerprint();

  if (matchedId == 1) {
    Serial.println("[INFO] Success, sending message...");

    strcpy(msg.state, "ON");

    esp_now_send(receiverAddrs, (uint8_t *) &msg, sizeof(msg));
  } else {
    Serial.println("[WARN] Incorrect fingerprint");
  }

  while (finger.getImage() != FINGERPRINT_NOFINGER) {
    delay(50);
  }
}
