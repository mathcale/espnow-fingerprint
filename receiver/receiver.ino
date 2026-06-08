#include <esp_now.h>
#include <WiFi.h>

typedef struct message {
  char state[3];
} struct_message;

struct_message msg;

void onDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&msg, incomingData, sizeof(msg));

  Serial.print("[INFO] Received state: ");
  Serial.println(msg.state);
}
 
void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("[ERR] Error initializing ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(esp_now_recv_cb_t(onDataRecv));

  Serial.println("[INFO] Receiver started, waiting for messages...");
}
 
void loop() {}
