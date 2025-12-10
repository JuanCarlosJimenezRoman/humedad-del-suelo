#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <Pangodream_18650_CL.h>

#define SENSOR_NAME "Sensor_02"

// Dirección MAC del Emisor A (nodo intermedio)
uint8_t emisorA_Address[] = {0xE4, 0xB0, 0x63, 0x41, 0xEA, 0xDC};

// 🔹 MISMOS pines y estructura del código original
#define ADC_PIN 6
#define CONV_FACTOR 2.04
#define READS 20
#define LED_R 18
#define LED_G 15
#define LED_B 14
#define SOIL_SENSOR_PIN 1
#define LED_TX_PIN 9
#define LED_PWR_PIN 8
#define MIN_USB_VOL 3.9

typedef struct struct_message {
  char sensorId[20];
  float humedad;
  float voltaje;
  int porcentaje;
  unsigned long timestamp;
} struct_message;

struct_message myData;
Pangodream_18650_CL battery(ADC_PIN, CONV_FACTOR, READS);

float voltaje = 0;
int nivel = 0;

void updateLED() {
  // 🔹 MISMA función del código original
  if (voltaje >= MIN_USB_VOL) {
    static unsigned long lastBlink = 0;
    static bool ledState = false;
    if (millis() - lastBlink >= 1000) {
      ledState = !ledState;
      lastBlink = millis();
      analogWrite(LED_R, ledState ? 255 : 0);
      analogWrite(LED_G, ledState ? 255 : 0);
      analogWrite(LED_B, ledState ? 255 : 0);
    }
  } else {
    if (nivel >= 80) {
      analogWrite(LED_R, 0);
      analogWrite(LED_G, 255);
      analogWrite(LED_B, 0);
    } else if (nivel >= 50) {
      analogWrite(LED_R, 255);
      analogWrite(LED_G, 255);
      analogWrite(LED_B, 0);
    } else if (nivel >= 20) {
      analogWrite(LED_R, 255);
      analogWrite(LED_G, 165);
      analogWrite(LED_B, 0);
    } else {
      analogWrite(LED_R, 255);
      analogWrite(LED_G, 0);
      analogWrite(LED_B, 0);
    }
  }
}

void blinkTXLed() {
  digitalWrite(LED_TX_PIN, HIGH);
  delay(100);
  digitalWrite(LED_TX_PIN, LOW);
}

void OnDataSent(const esp_now_send_info_t *info, esp_now_send_status_t status) {
  Serial.print("Estado de envío a Emisor A: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "✅ Éxito" : "❌ Falló");
}

float readHumiditySensor() {
  uint16_t lectura = analogRead(SOIL_SENSOR_PIN);
  float hum = map(lectura, 4095, 1200, 0, 100);
  return constrain(hum, 0, 100);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n🚀 Emisor B - Enviando a Emisor A");
  Serial.printf("🏷 ID: %s\n", SENSOR_NAME);

  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  pinMode(LED_TX_PIN, OUTPUT);
  pinMode(LED_PWR_PIN, OUTPUT);
  pinMode(SOIL_SENSOR_PIN, INPUT);
  digitalWrite(LED_PWR_PIN, HIGH);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.print("📡 MAC del Emisor B: ");
  Serial.println(WiFi.macAddress());
  Serial.print("🎯 MAC del Emisor A: ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", emisorA_Address[i]);
    if (i < 5) Serial.print(":");
  }
  Serial.println();

  if (esp_now_init() != ESP_OK) {
    Serial.println("❌ Error inicializando ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, emisorA_Address, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("❌ Error agregando Emisor A como peer");
    return;
  }

  Serial.println("✅ Conectado a Emisor A\n");
}

void loop() {
  voltaje = battery.getBatteryVolts();
  nivel = battery.getBatteryChargeLevel();
  float humedad = readHumiditySensor();

  strcpy(myData.sensorId, SENSOR_NAME);
  myData.humedad = humedad;
  myData.voltaje = voltaje;
  myData.porcentaje = nivel;
  myData.timestamp = millis();

  Serial.printf("📤 Enviando a A | 💧 %.1f%% | 🔋 %.2fV (%d%%)\n",
                humedad, voltaje, nivel);

  esp_err_t result = esp_now_send(emisorA_Address, (uint8_t *)&myData, sizeof(myData));
  
  blinkTXLed();
  updateLED();
  delay(1500);
}