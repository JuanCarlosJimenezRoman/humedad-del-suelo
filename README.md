# 🌱 Sistema de Monitoreo de Humedad con ESP-NOW

Sistema de monitoreo en tiempo real de sensores de humedad de suelo utilizando el protocolo ESP-NOW y visualización mediante servidor web.

![Estado](https://img.shields.io/badge/estado-activo-success)
![Plataforma](https://img.shields.io/badge/plataforma-ESP32-blue)
![Licencia](https://img.shields.io/badge/licencia-MIT-green)

## 📋 Tabla de Contenidos

- [Descripción](#-descripción)
- [Características](#-características)
- [Arquitectura del Sistema](#-arquitectura-del-sistema)
- [Requisitos](#-requisitos)
- [Instalación](#-instalación)
- [Configuración](#-configuración)
- [Estructura de Datos](#-estructura-de-datos)
- [Uso](#-uso)
- [Interfaz Web](#-interfaz-web)
- [Solución de Problemas](#-solución-de-problemas)
- [Contribuir](#-contribuir)
- [Licencia](#-licencia)

## 🎯 Descripción

Este proyecto implementa un sistema de monitoreo distribuido para sensores de humedad de suelo utilizando el protocolo ESP-NOW de Espressif. El sistema permite la comunicación inalámbrica entre múltiples nodos ESP32 sin necesidad de un router WiFi tradicional, con visualización de datos en tiempo real mediante un servidor web.

### ¿Por qué ESP-NOW?

- ✅ **Bajo consumo energético** - Ideal para sensores alimentados por batería
- ✅ **Comunicación rápida** - Latencia mínima entre dispositivos
- ✅ **Sin router necesario** - Comunicación directa peer-to-peer
- ✅ **Alcance extendido** - Mayor rango que WiFi tradicional
- ✅ **Topología flexible** - Soporta comunicación en cadena (daisy chain)

## ✨ Características

- 📡 **Comunicación ESP-NOW** para transmisión eficiente de datos
- 🌐 **Servidor web integrado** con interfaz moderna y responsive
- 🔄 **Actualización automática** de datos cada 2 segundos
- 📊 **Monitoreo multi-sensor** (hasta 2 sensores en configuración actual)
- 🔋 **Monitoreo de batería** en tiempo real
- 💧 **Medición de humedad** con valores porcentuales
- ⚡ **Medición de voltaje** para diagnóstico de batería
- 📱 **Acceso desde cualquier dispositivo** en la red local
- 🎨 **Interfaz visual intuitiva** con indicadores de estado

## 🏗️ Arquitectura del Sistema

```
┌─────────────┐         ┌─────────────┐         ┌─────────────┐
│  EMISOR B   │ ESP-NOW │  EMISOR A   │ ESP-NOW │  RECEPTOR   │
│  (Sensor)   ├────────►│ (Nodo Inter)├────────►│ (Servidor)  │
│             │         │             │         │             │
│  Sensor 1   │         │  Sensor 2   │         │  Web Server │
└─────────────┘         └─────────────┘         └─────────────┘
                                                       │
                                                       │ WiFi
                                                       ▼
                                                 ┌──────────┐
                                                 │ Usuarios │
                                                 │ (Browser)│
                                                 └──────────┘
```

### Flujo de Datos

1. **EMISOR B** lee datos del sensor y los envía a **EMISOR A** vía ESP-NOW
2. **EMISOR A** recibe datos de EMISOR B, lee su propio sensor y combina ambos
3. **EMISOR A** envía el paquete combinado al **RECEPTOR** vía ESP-NOW
4. **RECEPTOR** almacena los datos y los sirve mediante servidor web
5. **Navegadores** consultan el servidor y actualizan la interfaz automáticamente

## 🔧 Requisitos

### Hardware

- 3x ESP32 (cualquier modelo compatible con ESP-NOW)
- 2x Sensores de humedad de suelo (capacitivos recomendados)
- 2x Baterías LiPo 3.7V (o fuente de alimentación)
- Cables y conectores

### Software

- [Arduino IDE](https://www.arduino.cc/en/software) (1.8.x o superior) o [PlatformIO](https://platformio.org/)
- Soporte para placas ESP32
- Bibliotecas requeridas:
  - `WiFi.h` (incluida en el núcleo ESP32)
  - `esp_now.h` (incluida en el núcleo ESP32)
  - `WebServer.h` (incluida en el núcleo ESP32)

### Instalación del Soporte ESP32 en Arduino IDE

1. Abre Arduino IDE
2. Ve a `Archivo` → `Preferencias`
3. En "Gestor de URLs Adicionales de Tarjetas" añade:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Ve a `Herramientas` → `Placa` → `Gestor de tarjetas`
5. Busca "ESP32" e instala "esp32 by Espressif Systems"

## 📥 Instalación

### 1. Clonar el Repositorio

```bash
git clone https://github.com/tu-usuario/esp-now-humidity-monitor.git
cd esp-now-humidity-monitor
```

### 2. Estructura del Proyecto

```
esp-now-humidity-monitor/
├── README.md
├── receptor/
│   └── receptor.ino          # Código del receptor con servidor web
├── emisor_a/
│   └── emisor_a.ino          # Código del nodo intermedio
├── emisor_b/
│   └── emisor_b.ino          # Código del sensor final
├── docs/
│   ├── images/               # Capturas de pantalla
│   └── wiring_diagram.png    # Diagrama de conexiones
└── LICENSE
```

## ⚙️ Configuración

### 1. Obtener Direcciones MAC

Primero necesitas obtener las direcciones MAC de tus ESP32. Sube este sketch a cada dispositivo:

```cpp
#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
}

void loop() {}
```

### 2. Configurar el Receptor

Abre `receptor/receptor.ino` y modifica:

```cpp
// Líneas 11-12: Credenciales WiFi
const char* ssid = "TU_WIFI_SSID";
const char* password = "TU_WIFI_PASSWORD";

// Líneas 28-30: Direcciones MAC de tus sensores
const String macEmisorA = "E4:B0:63:41:EA:DC";  // MAC de tu EMISOR A
const String macEmisorB = "E4:B0:63:41:EA:4C";  // MAC de tu EMISOR B
```

### 3. Configurar los Emisores

En cada emisor (A y B), configura:

- La dirección MAC del receptor
- La dirección MAC del siguiente nodo (en caso del EMISOR B)
- Los pines del sensor de humedad
- Los parámetros de calibración del sensor

### 4. Subir el Código

1. **EMISOR B**: Sube `emisor_b.ino`
2. **EMISOR A**: Sube `emisor_a.ino`
3. **RECEPTOR**: Sube `receptor.ino`

## 📊 Estructura de Datos

### Paquete de Datos Combinado

```cpp
typedef struct struct_combined_message {
  // Datos del EMISOR A (Nodo Intermedio)
  char sensorId[20];          // Identificador del sensor A
  float humedad;              // Humedad del suelo en % (0-100)
  float voltaje;              // Voltaje de batería en V
  int porcentaje;             // Porcentaje de batería (0-100)
  unsigned long timestamp;    // Marca de tiempo en ms
  
  // Datos del EMISOR B (Sensor Final)
  char sensorId_B[20];        // Identificador del sensor B
  float humedad_B;            // Humedad del suelo en %
  float voltaje_B;            // Voltaje de batería en V
  int porcentaje_B;           // Porcentaje de batería
  unsigned long timestamp_B;  // Marca de tiempo en ms
} struct_combined_message;
```

### Valores Típicos

| Parámetro | Rango | Descripción |
|-----------|-------|-------------|
| Humedad | 0% - 100% | 0% = Seco, 100% = Saturado |
| Voltaje | 3.0V - 4.2V | Batería LiPo típica |
| Batería | 0% - 100% | Calculado a partir del voltaje |
| Timestamp | 0 - ∞ | Milisegundos desde el inicio |

## 🚀 Uso

### Inicio del Sistema

1. **Enciende el RECEPTOR** primero
2. Abre el Monitor Serial (115200 baudios)
3. Anota la **dirección IP** mostrada
4. **Enciende los EMISORES** (B primero, luego A)

### Acceso a la Interfaz Web

1. Abre un navegador web
2. Ingresa la IP del receptor: `http://192.168.x.x`
3. La página se actualizará automáticamente cada 2 segundos

### Monitor Serial (Opcional)

El receptor también muestra información en el Monitor Serial para depuración:

```
📦 Datos recibidos:
   Emisor A: 45.2% humedad, 3.87V, 78% batería
   Emisor B: 62.8% humedad, 4.05V, 92% batería
```

## 🎨 Interfaz Web

### Características de la Interfaz

- **Diseño Responsive**: Se adapta a móviles, tablets y computadoras
- **Tarjetas Individuales**: Una tarjeta por cada sensor
- **Indicadores de Estado**: 
  - 🟢 **ACTIVO**: Sensor enviando datos
  - 🔴 **INACTIVO**: Sin comunicación
- **Información en Tiempo Real**:
  - 💧 Humedad del suelo
  - ⚡ Voltaje de batería
  - 🔋 Porcentaje de batería
  - 📡 Dirección MAC
- **Panel de Información**: Datos del sistema y última actualización

### Capturas de Pantalla

![Interfaz Principal](docs/images/interfaz-principal.png)
*Vista principal mostrando ambos sensores activos*

## 🔍 Solución de Problemas

### El receptor no se conecta a WiFi

**Síntomas**: Mensaje "❌ No se pudo conectar a WiFi"

**Soluciones**:
- Verifica SSID y contraseña
- Asegúrate de que la red sea 2.4GHz (ESP32 no soporta 5GHz)
- Revisa que el router esté encendido y en rango

### No se reciben datos de los sensores

**Síntomas**: Interfaz muestra "INACTIVO" en ambos sensores

**Soluciones**:
1. Verifica que las direcciones MAC estén correctas
2. Asegúrate de que los emisores estén encendidos
3. Revisa la distancia entre dispositivos (máx. ~100m en campo abierto)
4. Verifica en el Monitor Serial si hay mensajes de error

### Los datos se actualizan lentamente

**Síntomas**: Valores no cambian por varios segundos

**Soluciones**:
- Los emisores pueden estar en modo deep sleep
- Ajusta el intervalo de transmisión en el código de los emisores
- Verifica la calidad de la señal ESP-NOW

### Uno de los sensores no aparece

**Síntomas**: Solo aparece un sensor como activo

**Soluciones**:
- Verifica que EMISOR B esté encendido (debe arrancar primero)
- Revisa que EMISOR A esté recibiendo datos de B
- Comprueba las direcciones MAC en el código

### La interfaz web no carga

**Síntomas**: El navegador no puede acceder a la IP

**Soluciones**:
- Verifica que estés en la misma red WiFi que el receptor
- Prueba hacer ping a la IP del receptor
- Reinicia el receptor y anota la nueva IP

## 🔋 Optimización de Batería

Para maximizar la duración de la batería en los emisores:

1. **Deep Sleep**: Implementa períodos de sueño profundo
2. **Intervalo de Transmisión**: Aumenta el tiempo entre lecturas
3. **Apaga WiFi**: En los emisores solo usa ESP-NOW
4. **Voltaje de Operación**: Monitorea y apaga antes de descarga completa

Ejemplo de configuración de deep sleep:

```cpp
// Dormir por 60 segundos
esp_sleep_enable_timer_wakeup(60 * 1000000);
esp_deep_sleep_start();
```

## 📈 Mejoras Futuras

- [ ] Almacenamiento de histórico de datos
- [ ] Gráficas de tendencias
- [ ] Notificaciones push cuando humedad < umbral
- [ ] Soporte para más de 2 sensores
- [ ] Modo AP para configuración inicial
- [ ] Exportación de datos a CSV
- [ ] Integración con Home Assistant
- [ ] Control de riego automático

## 🤝 Contribuir

¡Las contribuciones son bienvenidas! Por favor:

1. Haz fork del proyecto
2. Crea una rama para tu feature (`git checkout -b feature/AmazingFeature`)
3. Commit tus cambios (`git commit -m 'Add some AmazingFeature'`)
4. Push a la rama (`git push origin feature/AmazingFeature`)
5. Abre un Pull Request

## 📝 Licencia

Este proyecto está bajo la Licencia MIT. Ver el archivo `LICENSE` para más detalles.

## 👨‍💻 Autor

**Tu Nombre**
- GitHub: [@tu-usuario](https://github.com/tu-usuario)
- Email: tu-email@ejemplo.com

## 🙏 Agradecimientos

- Espressif Systems por el protocolo ESP-NOW
- Comunidad de Arduino y ESP32
- Todos los contribuidores del proyecto

## 📚 Referencias

- [Documentación ESP-NOW](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_now.html)
- [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32)
- [Sensores de Humedad Capacitivos](https://how2electronics.com/interface-capacitive-soil-moisture-sensor-arduino/)

---

⭐ Si este proyecto te fue útil, considera darle una estrella en GitHub