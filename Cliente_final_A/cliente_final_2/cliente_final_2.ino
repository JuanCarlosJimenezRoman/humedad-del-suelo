#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <Pangodream_18650_CL.h>

#define SENSOR_NAME "Sensor_01"

// Direcciones MAC
uint8_t receptorAddress[] = {0x68, 0x25, 0xDD, 0x47, 0xE5, 0xF8};  // Receptor final
uint8_t emisorB_Address[] = {0xE4, 0xB0, 0x63, 0x41, 0xEA, 0x4C};  // Emisor B

// 🔹 MISMOS pines y configuración
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

// 🔹 NUEVA estructura para datos combinados
typedef struct struct_combined_message {
  char sensorId[20];
  float humedad;
  float voltaje;
  int porcentaje;
  unsigned long timestamp;
  
  // 🔹 DATOS del Emisor B
  char sensorId_B[20];
  float humedad_B;
  float voltaje_B;
  int porcentaje_B;
  unsigned long timestamp_B;
} struct_combined_message;

typedef struct struct_simple_message {
  char sensorId[20];
  float humedad;
  float voltaje;
  int porcentaje;
  unsigned long timestamp;
} struct_simple_message;

struct_combined_message combinedData;
struct_simple_message myData;
struct_simple_message dataFromB;

Pangodream_18650_CL battery(ADC_PIN, CONV_FACTOR, READS);

float voltaje = 0;
int nivel = 0;
bool newDataFromB = false;

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

// 🔹 CALLBACK para recepción desde Emisor B
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  memcpy(&dataFromB, data, sizeof(dataFromB));
  newDataFromB = true;
  
  Serial.println("📩 Datos recibidos de Emisor B:");
  Serial.printf("🏷 Sensor: %s\n", dataFromB.sensorId);
  Serial.printf("💧 Humedad: %.1f %%\n", dataFromB.humedad);
  Serial.printf("🔋 Voltaje: %.2f V\n", dataFromB.voltaje);
  Serial.println("-------------------------------");
}

// 🔹 CALLBACK para envío al Receptor
void OnDataSent(const esp_now_send_info_t *info, esp_now_send_status_t status) {
  Serial.print("Estado de envío al Receptor: ");
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
  Serial.println("\n🚀 Emisor A - Nodo Intermedio");
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

  Serial.print("📡 MAC del Emisor A: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("❌ Error inicializando ESP-NOW");
    return;
  }

  // 🔹 Registrar callbacks
  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);

  // 🔹 Agregar RECEPTOR como peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receptorAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("❌ Error agregando Receptor como peer");
    return;
  }

  // 🔹 Agregar EMISOR B como peer (para recibir sus datos)
  memcpy(peerInfo.peer_addr, emisorB_Address, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("❌ Error agregando Emisor B como peer");
    return;
  }

  Serial.println("✅ Nodo intermedio listo:");
  Serial.println("   - Recibiendo de Emisor B");
  Serial.println("   - Enviando a Receptor\n");
}

void loop() {
  // 🔹 Leer datos propios del Emisor A
  voltaje = battery.getBatteryVolts();
  nivel = battery.getBatteryChargeLevel();
  float humedad = readHumiditySensor();

  // 🔹 Llenar datos propios
  strcpy(myData.sensorId, SENSOR_NAME);
  myData.humedad = humedad;
  myData.voltaje = voltaje;
  myData.porcentaje = nivel;
  myData.timestamp = millis();

  // 🔹 Preparar datos combinados
  strcpy(combinedData.sensorId, myData.sensorId);
  combinedData.humedad = myData.humedad;
  combinedData.voltaje = myData.voltaje;
  combinedData.porcentaje = myData.porcentaje;
  combinedData.timestamp = myData.timestamp;

  // 🔹 Si hay datos nuevos del Emisor B, incluirlos
  if (newDataFromB) {
    strcpy(combinedData.sensorId_B, dataFromB.sensorId);
    combinedData.humedad_B = dataFromB.humedad;
    combinedData.voltaje_B = dataFromB.voltaje;
    combinedData.porcentaje_B = dataFromB.porcentaje;
    combinedData.timestamp_B = dataFromB.timestamp;
    newDataFromB = false;
  }

  // 🔹 Enviar datos combinados al Receptor
  Serial.printf("📤 Enviando datos combinados al Receptor\n");
  Serial.printf("   Sensor A: 💧 %.1f%%, 🔋 %.2fV (%d%%)\n", 
                myData.humedad, myData.voltaje, myData.porcentaje);
  
  if (strlen(combinedData.sensorId_B) > 0) {
    Serial.printf("   Sensor B: 💧 %.1f%%, 🔋 %.2fV (%d%%)\n", 
                  combinedData.humedad_B, combinedData.voltaje_B, combinedData.porcentaje_B);
  }

  esp_err_t result = esp_now_send(receptorAddress, (uint8_t *)&combinedData, sizeof(combinedData));
  
  blinkTXLed();
  updateLED();
  delay(1500);
}