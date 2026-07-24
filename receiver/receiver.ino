#include <esp_now.h>
#include <WiFi.h>

#define RELAY_PIN 18

typedef struct message {
  char state[3];
} struct_message;

struct_message msg;

void triggerRelay() {
  digitalWrite(RELAY_PIN, HIGH);
  delay(300);
  digitalWrite(RELAY_PIN, LOW);

  Serial.println("[INFO] Relay triggered for 300ms");
}

void onDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&msg, incomingData, sizeof(msg));

  Serial.print("[INFO] Received state: ");
  Serial.println(msg.state);

  if (strcmp(msg.state, "ON") == 0) {
    Serial.println("[INFO] Triggering relay...");
    triggerRelay();
  }
}
 
void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("[ERR] Error initializing ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(esp_now_recv_cb_t(onDataRecv));

  Serial.print("[INFO] MAC Address: ");
  Serial.println(WiFi.macAddress());

  Serial.println("[INFO] Receiver started, waiting for messages...");
}
 
void loop() {}
