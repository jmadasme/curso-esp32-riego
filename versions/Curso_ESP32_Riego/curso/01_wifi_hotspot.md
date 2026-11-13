# Capítulo 1: WiFi y Hotspot en ESP32

**Duración:** 1.5 horas
**Objetivo:** Aprender a configurar el ESP32 como Access Point (hotspot) WiFi

---

## 🎯 Objetivos del Capítulo

- Comprender los diferentes modos WiFi del ESP32
- Configurar el ESP32 como Access Point (AP)
- Conectarse al hotspot desde un teléfono o PC
- Visualizar mensajes de depuración por Serial
- Escanear redes WiFi visibles

---

## 📖 1.1 Teoría: Modos WiFi del ESP32

El ESP32 tiene un módulo WiFi muy potente que puede funcionar en varios modos:

### Modos de Operación

| Modo | Constante | Descripción |
|------|-----------|-------------|
| **Station (STA)** | `WIFI_STA` | Cliente WiFi: se conecta a un router existente |
| **Access Point (AP)** | `WIFI_AP` | Crea su propia red WiFi (hotspot) |
| **AP + Station** | `WIFI_AP_STA` | Ambos simultáneamente (el que usaremos) |

### Direcciones IP

En una red WiFi, cada dispositivo tiene una dirección IP única:

- **Router doméstico**: Normalmente `192.168.1.1` o `192.168.0.1`
- **ESP32 en modo AP**: Por defecto `192.168.4.1`
- **Clientes conectados**: Reciben IP automática vía DHCP

> 💡 **Dato importante:** En modo AP, el ESP32 actúa como router, asignando IPs a los dispositivos que se conectan a él.

### Frecuencias

El ESP32 solo funciona en **2.4 GHz** (no en 5 GHz). Asegúrate de que tu teléfono o PC estén en esta banda.

---

## 💻 1.2 Código: Crear un Hotspot Básico

Vamos a crear nuestro primer programa: un hotspot WiFi.

### Código Mínimo

```cpp
#include <WiFi.h>

// Configuración del hotspot
const char* ssid = "MiRiego-ESP32";
const char* password = "[REDACTED]";

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\n=== HOTSPOT ESP32 ===");
  Serial.print("Creando red: ");
  Serial.println(ssid);

  // Configurar ESP32 en modo AP
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  Serial.print("Dirección IP del AP: ");
  Serial.println(WiFi.softAPIP());
  Serial.println("=== LISTO ===\n");
}

void loop() {
  // Contar clientes conectados
  Serial.print("Clientes conectados: ");
  Serial.println(WiFi.softAPgetStationNum());
  delay(5000); // Mostrar cada 5 segundos
}
```

### Explicación línea por línea

| Línea | Explicación |
|-------|-------------|
| `WiFi.mode(WIFI_AP)` | Establece el modo Access Point |
| `WiFi.softAP(ssid, pass)` | Crea la red con nombre y contraseña |
| `WiFi.softAPIP()` | Devuelve la IP del AP (192.168.4.1) |
| `WiFi.softAPgetStationNum()` | Cuenta dispositivos conectados |

### Modo AP+STA (el que usaremos en el proyecto)

```cpp
WiFi.mode(WIFI_AP_STA); // Modo dual
WiFi.softAP(ssid, pass); // Inicia AP
WiFi.begin(staSsid, staPass); // Conecta a internet (opcional)
```

---

## 🔬 1.3 Laboratorio 1: Mi Primer Hotspot

### Objetivo
Crear un hotspot personalizado y verificar su funcionamiento.

### Material
- ESP32-S3 con cable USB
- PC o teléfono con WiFi

### Paso a Paso

**Paso 1:** Conecta tu ESP32 al PC por USB.

**Paso 2:** Abre Arduino IDE y selecciona:
- **Placa:** `ESP32S3 Dev Module`
- **Puerto:** El que aparezca (ej: COM3, /dev/ttyACM0)

**Paso 3:** Copia y pega el código anterior.

**Paso 4:** Modifica el SSID y contraseña:

```cpp
const char* ssid = "MiRiego-XXXX"; // Cámbialo
const char* password = "[REDACTED]"; // Mínimo 8 caracteres
```

**Paso 5:** Sube el código al ESP32.

**Paso 6:** Abre el Monitor Serial (115200 baudios).

**Paso 7:** Conéctate desde tu teléfono:
- Ve a Configuración > WiFi
- Busca la red "MiRiego-XXXX"
- Ingresa la contraseña
- ¡Conectado!

**Paso 8:** Verifica en el Monitor Serial que ves el mensaje de cliente conectado.

### Resultado Esperado

```
=== HOTSPOT ESP32 ===
Creando red: MiRiego-XXXX
Dirección IP del AP: 192.168.4.1
=== LISTO ===

Clientes conectados: 1
Clientes conectados: 1
```

---

## ✏️ 1.4 Ejercicio: Escanear Redes WiFi

Modifica el programa anterior para que, además de crear el hotspot, escanee las redes WiFi disponibles y las muestre por Serial.

### Código del Ejercicio

```cpp
#include <WiFi.h>

const char* ssid = "MiRiego-XXXX";
const char* password = "[REDACTED]";

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Escanear redes primero
  Serial.println("Escaneando redes WiFi...");
  int n = WiFi.scanNetworks();

  if (n == 0) {
    Serial.println("No se encontraron redes.");
  } else {
    Serial.print("Se encontraron ");
    Serial.print(n);
    Serial.println(" redes:");
    for (int i = 0; i < n; i++) {
      Serial.print("  ");
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.println(" dBm)");
    }
  }

  // Crear hotspot
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  Serial.println("\nHotspot creado:");
  Serial.print("SSID: "); Serial.println(ssid);
  Serial.print("IP: "); Serial.println(WiFi.softAPIP());
}

void loop() {
  delay(10000);
}
```

### Pistas
1. Usa `WiFi.scanNetworks()` que devuelve el número de redes encontradas
2. `WiFi.SSID(i)` te da el nombre de cada red
3. `WiFi.RSSI(i)` te da la intensidad de la señal en dBm (más cercano a 0 es mejor)

---

## ✅ Resumen del Capítulo

| Concepto | Aprendido |
|----------|-----------|
| ✅ Modos WiFi del ESP32 | STA, AP, AP+STA |
| ✅ Crear un Access Point | `WiFi.softAP(ssid, pass)` |
| ✅ IP del AP | `192.168.4.1` |
| ✅ Monitor Serial | Depuración y mensajes |
| ✅ Escaneo de redes | `WiFi.scanNetworks()` |
| ✅ Contar clientes | `WiFi.softAPgetStationNum()` |

### En el Proyecto Final...

En nuestro sistema de riego, el ESP32 funcionará en **modo AP+STA**:
- Como **AP** para que puedas configurarlo desde tu teléfono
- Como **STA** para conectarse a internet y consultar el clima

➡️ **Siguiente capítulo:** [02 - Captive Portal](02_captive_portal.md)
