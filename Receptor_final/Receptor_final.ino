#include <WiFi.h>
#include <esp_now.h>
#include <WebServer.h>

// 🔹 Credenciales WiFi (para el servidor web)
const char* ssid = "TU_WIFI_SSID";        // Cambia esto
const char* password = "TU_WIFI_PASSWORD"; // Cambia esto

// 🔹 Estructura para datos combinados
typedef struct struct_combined_message {
  char sensorId[20];
  float humedad;
  float voltaje;
  int porcentaje;
  unsigned long timestamp;
  
  char sensorId_B[20];
  float humedad_B;
  float voltaje_B;
  int porcentaje_B;
  unsigned long timestamp_B;
} struct_combined_message;

struct_combined_message incomingData;
bool dataReceived = false;
unsigned long lastUpdate = 0;

// 🔹 MAC addresses conocidas
const String macEmisorA = "E4:B0:63:41:EA:DC";
const String macEmisorB = "E4:B0:63:41:EA:4C";
String macReceptor = "";

// 🔹 Servidor web en el puerto 80
WebServer server(80);

// 🔹 Función para generar el HTML
String generateHTML() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Monitor ESP-NOW</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
        }
        .header {
            background: white;
            padding: 30px;
            border-radius: 15px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.3);
            margin-bottom: 30px;
            text-align: center;
        }
        .header h1 {
            color: #667eea;
            font-size: 2.5em;
            margin-bottom: 10px;
        }
        .header p {
            color: #666;
            font-size: 1.1em;
        }
        .status {
            display: inline-block;
            padding: 8px 20px;
            border-radius: 20px;
            font-weight: bold;
            margin-top: 10px;
        }
        .status.online {
            background: #4CAF50;
            color: white;
        }
        .status.waiting {
            background: #ff9800;
            color: white;
        }
        .cards {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
            margin-bottom: 30px;
        }
        .card {
            background: white;
            padding: 25px;
            border-radius: 15px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.3);
            transition: transform 0.3s;
        }
        .card:hover {
            transform: translateY(-5px);
        }
        .card-header {
            display: flex;
            align-items: center;
            justify-content: space-between;
            margin-bottom: 20px;
            padding-bottom: 15px;
            border-bottom: 2px solid #f0f0f0;
        }
        .card-title {
            font-size: 1.5em;
            color: #333;
            font-weight: bold;
        }
        .badge {
            padding: 5px 15px;
            border-radius: 20px;
            font-size: 0.85em;
            font-weight: bold;
        }
        .badge.active {
            background: #4CAF50;
            color: white;
        }
        .badge.inactive {
            background: #ccc;
            color: #666;
        }
        .data-row {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 12px 0;
            border-bottom: 1px solid #f0f0f0;
        }
        .data-row:last-child {
            border-bottom: none;
        }
        .data-label {
            color: #666;
            font-size: 0.95em;
        }
        .data-value {
            font-size: 1.3em;
            font-weight: bold;
            color: #667eea;
        }
        .icon {
            font-size: 1.2em;
            margin-right: 8px;
        }
        .mac-address {
            font-family: 'Courier New', monospace;
            background: #f5f5f5;
            padding: 10px;
            border-radius: 8px;
            font-size: 0.9em;
            color: #333;
            text-align: center;
            margin-top: 15px;
        }
        .info-panel {
            background: white;
            padding: 25px;
            border-radius: 15px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.3);
        }
        .info-title {
            font-size: 1.3em;
            color: #333;
            margin-bottom: 15px;
            font-weight: bold;
        }
        .info-item {
            padding: 10px;
            background: #f9f9f9;
            border-left: 4px solid #667eea;
            margin-bottom: 10px;
            border-radius: 5px;
        }
        @keyframes pulse {
            0%, 100% { opacity: 1; }
            50% { opacity: 0.5; }
        }
        .loading {
            animation: pulse 2s infinite;
        }
    </style>
    <script>
        function updateData() {
            fetch('/data')
                .then(response => response.json())
                .then(data => {
                    // Actualizar Emisor A
                    document.getElementById('humedad_a').textContent = data.humedad_a + '%';
                    document.getElementById('voltaje_a').textContent = data.voltaje_a + 'V';
                    document.getElementById('bateria_a').textContent = data.bateria_a + '%';
                    document.getElementById('mac_a').textContent = data.mac_a;
                    document.getElementById('badge_a').className = 'badge ' + (data.activo_a ? 'active' : 'inactive');
                    document.getElementById('badge_a').textContent = data.activo_a ? '● ACTIVO' : '● INACTIVO';
                    
                    // Actualizar Emisor B
                    document.getElementById('humedad_b').textContent = data.humedad_b + '%';
                    document.getElementById('voltaje_b').textContent = data.voltaje_b + 'V';
                    document.getElementById('bateria_b').textContent = data.bateria_b + '%';
                    document.getElementById('mac_b').textContent = data.mac_b;
                    document.getElementById('badge_b').className = 'badge ' + (data.activo_b ? 'active' : 'inactive');
                    document.getElementById('badge_b').textContent = data.activo_b ? '● ACTIVO' : '● ACTIVO';
                    
                    // Actualizar estado
                    document.getElementById('status').className = 'status ' + (data.dataReceived ? 'online' : 'waiting');
                    document.getElementById('status').textContent = data.dataReceived ? '🟢 RECIBIENDO DATOS' : '🟡 ESPERANDO DATOS';
                    
                    // Actualizar última actualización
                    document.getElementById('last_update').textContent = data.lastUpdate + ' ms';
                })
                .catch(error => console.error('Error:', error));
        }
        
        // Actualizar cada 2 segundos
        setInterval(updateData, 2000);
        
        // Actualizar al cargar
        window.onload = updateData;
    </script>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>📡 Sistema de Monitoreo ESP-NOW</h1>
            <p>Monitoreo en tiempo real de sensores de humedad</p>
            <div id="status" class="status waiting">🟡 ESPERANDO DATOS</div>
        </div>
        
        <div class="cards">
            <!-- EMISOR A -->
            <div class="card">
                <div class="card-header">
                    <span class="card-title">🔵 EMISOR A</span>
                    <span id="badge_a" class="badge inactive">● INACTIVO</span>
                </div>
                <div class="data-row">
                    <span class="data-label"><span class="icon">💧</span>Humedad</span>
                    <span id="humedad_a" class="data-value">---%</span>
                </div>
                <div class="data-row">
                    <span class="data-label"><span class="icon">⚡</span>Voltaje</span>
                    <span id="voltaje_a" class="data-value">---V</span>
                </div>
                <div class="data-row">
                    <span class="data-label"><span class="icon">🔋</span>Batería</span>
                    <span id="bateria_a" class="data-value">---%</span>
                </div>
                <div class="mac-address">
                    <strong>MAC:</strong> <span id="mac_a">--:--:--:--:--:--</span>
                </div>
            </div>
            
            <!-- EMISOR B -->
            <div class="card">
                <div class="card-header">
                    <span class="card-title">🟢 EMISOR B</span>
                    <span id="badge_b" class="badge inactive">● INACTIVO</span>
                </div>
                <div class="data-row">
                    <span class="data-label"><span class="icon">💧</span>Humedad</span>
                    <span id="humedad_b" class="data-value">---%</span>
                </div>
                <div class="data-row">
                    <span class="data-label"><span class="icon">⚡</span>Voltaje</span>
                    <span id="voltaje_b" class="data-value">---V</span>
                </div>
                <div class="data-row">
                    <span class="data-label"><span class="icon">🔋</span>Batería</span>
                    <span id="bateria_b" class="data-value">---%</span>
                </div>
                <div class="mac-address">
                    <strong>MAC:</strong> <span id="mac_b">--:--:--:--:--:--</span>
                </div>
            </div>
        </div>
        
        <div class="info-panel">
            <div class="info-title">📊 Información del Sistema</div>
            <div class="info-item">
                <strong>🔧 Receptor MAC:</strong> )rawliteral" + macReceptor + R"rawliteral(
            </div>
            <div class="info-item">
                <strong>⏱️ Última actualización:</strong> <span id="last_update">---</span>
            </div>
            <div class="info-item">
                <strong>📶 Estado:</strong> Sistema funcionando correctamente
            </div>
        </div>
    </div>
</body>
</html>
)rawliteral";
  return html;
}

// 🔹 Función para enviar datos JSON
void handleData() {
  String json = "{";
  json += "\"humedad_a\":" + String(incomingData.humedad, 1) + ",";
  json += "\"voltaje_a\":" + String(incomingData.voltaje, 2) + ",";
  json += "\"bateria_a\":" + String(incomingData.porcentaje) + ",";
  json += "\"mac_a\":\"" + macEmisorA + "\",";
  json += "\"activo_a\":" + String(dataReceived ? "true" : "false") + ",";
  
  json += "\"humedad_b\":" + String(incomingData.humedad_B, 1) + ",";
  json += "\"voltaje_b\":" + String(incomingData.voltaje_B, 2) + ",";
  json += "\"bateria_b\":" + String(incomingData.porcentaje_B) + ",";
  json += "\"mac_b\":\"" + macEmisorB + "\",";
  json += "\"activo_b\":" + String((strlen(incomingData.sensorId_B) > 0) ? "true" : "false") + ",";
  
  json += "\"dataReceived\":" + String(dataReceived ? "true" : "false") + ",";
  json += "\"lastUpdate\":" + String(lastUpdate);
  json += "}";
  
  server.send(200, "application/json", json);
}

// 🔹 Callback cuando llegan datos por ESP-NOW
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  memcpy(&incomingData, data, sizeof(incomingData));
  dataReceived = true;
  lastUpdate = millis();
  
  // 🔹 También mostrar en Serial para debug (opcional)
  Serial.println("\n📦 Datos recibidos:");
  Serial.printf("   Emisor A: %.1f%% humedad, %.2fV, %d%% batería\n", 
                incomingData.humedad, incomingData.voltaje, incomingData.porcentaje);
  
  if (strlen(incomingData.sensorId_B) > 0) {
    Serial.printf("   Emisor B: %.1f%% humedad, %.2fV, %d%% batería\n", 
                  incomingData.humedad_B, incomingData.voltaje_B, incomingData.porcentaje_B);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=== SISTEMA ESP-NOW CON SERVIDOR WEB ===");
  
  // 🔹 Conectar a WiFi primero
  Serial.println("📡 Conectando a WiFi...");
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi conectado");
    Serial.print("🌐 IP del servidor: ");
    Serial.println(WiFi.localIP());
    macReceptor = WiFi.macAddress();
    Serial.print("📡 MAC del Receptor: ");
    Serial.println(macReceptor);
  } else {
    Serial.println("\n❌ No se pudo conectar a WiFi");
    Serial.println("⚠️ Verifica SSID y contraseña");
  }
  
  // 🔹 Inicializar ESP-NOW
  Serial.println("\n🔧 Inicializando ESP-NOW...");
  if (esp_now_init() != ESP_OK) {
    Serial.println("❌ Error inicializando ESP-NOW");
    return;
  }
  Serial.println("✅ ESP-NOW inicializado");
  
  // 🔹 Registrar callback de recepción
  esp_now_register_recv_cb(OnDataRecv);
  
  // 🔹 Configurar peer broadcast
  esp_now_peer_info_t peerInfo = {};
  memset(&peerInfo, 0, sizeof(peerInfo));
  for (int i = 0; i < 6; ++i) peerInfo.peer_addr[i] = 0xFF;
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) == ESP_OK) {
    Serial.println("✅ Peer broadcast configurado");
  }
  
  // 🔹 Configurar servidor web
  server.on("/", []() {
    server.send(200, "text/html", generateHTML());
  });
  
  server.on("/data", handleData);
  
  server.begin();
  Serial.println("✅ Servidor web iniciado");
  
  Serial.println("\n🟢 Sistema listo!");
  Serial.println("📱 Abre tu navegador en: http://" + WiFi.localIP().toString());
  Serial.println("🎯 Esperando datos de sensores...\n");
}

void loop() {
  server.handleClient();
  delay(10);
}