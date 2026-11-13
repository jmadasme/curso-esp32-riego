# Guía de Referencia Rápida

Tablas de referencia de funciones, comandos y configuraciones útiles para el desarrollo con ESP32

---

## 📡 WiFi

### Modos de Operación

| Modo | Código | Descripción |
|------|--------|-------------|
| Station | `WiFi.mode(WIFI_STA)` | Cliente WiFi |
| Access Point | `WiFi.mode(WIFI_AP)` | Crea su propia red |
| AP+STA | `WiFi.mode(WIFI_AP_STA)` | Ambos modos (recomendado) |

### Funciones WiFi

| Función | Descripción | Ejemplo |
|---------|-------------|---------|
| `WiFi.softAP(ssid, pass)` | Crear Access Point | `WiFi.softAP("MiRed", "12345678")` |
| `WiFi.softAPIP()` | IP del AP | `192.168.4.1` |
| `WiFi.begin(ssid, pass)` | Conectar a red WiFi | `WiFi.begin("MiFibra", "pass")` |
| `WiFi.localIP()` | IP asignada por router | `192.168.1.42` |
| `WiFi.status()` | Estado de conexión | `WL_CONNECTED` |
| `WiFi.scanNetworks()` | Escanear redes | Devuelve número de redes |
| `WiFi.SSID(i)` | Nombre de red i | `WiFi.SSID(0)` |
| `WiFi.RSSI(i)` | Señal de red i (dBm) | `WiFi.RSSI(0)` |
| `WiFi.softAPgetStationNum()` | Clientes conectados al AP | `3` |

### Estados de Conexión

| Constante | Valor | Significado |
|-----------|-------|-------------|
| `WL_IDLE_STATUS` | 0 | Esperando |
| `WL_CONNECTED` | 3 | Conectado |
| `WL_DISCONNECTED` | 6 | Desconectado |

---

## 🖥️ WebServer

### Configuración

```cpp
WebServer server(80);          // Crear servidor en puerto 80
server.begin();                // Iniciar
server.handleClient();         // Atender peticiones (en loop)
```

### Métodos de Ruta

| Método | Uso |
|--------|-----|
| `server.on("/", HTTP_GET, handler)` | Ruta GET |
| `server.on("/guardar", HTTP_POST, handler)` | Ruta POST |
| `server.onNotFound(handler)` | Rutas no definidas |

### Envío de Respuestas

```cpp
// HTML
server.send(200, "text/html", "<html>...</html>");

// JSON
server.send(200, "application/json", "{\"key\":\"value\"}");

// Redirección
server.sendHeader("Location", "/", true);
server.send(302, "text/plain", "");

// Texto plano
server.send(200, "text/plain", "Hola mundo");
```

### Obtener Datos

```cpp
String valor = server.arg("nombre_campo");     // De formulario POST/GET
IPAddress ip = server.client().remoteIP();      // IP del cliente
```

---

## 🌐 DNSServer (Captive Portal)

| Función | Descripción |
|---------|-------------|
| `dnsServer.start(port, "*", apIP)` | Iniciar DNS spoofing |
| `dnsServer.processNextRequest()` | Procesar consultas (en loop) |

```cpp
DNSServer dnsServer;
dnsServer.start(53, "*", IPAddress(192,168,4,1));
```

---

## 💾 EEPROM

### Funciones Básicas

| Función | Descripción |
|---------|-------------|
| `EEPROM.begin(tamaño)` | Inicializar (ej: 512) |
| `EEPROM.write(dir, valor)` | Escribir byte (0-255) |
| `EEPROM.read(dir)` | Leer byte |
| `EEPROM.put(dir, struct)` | Escribir estructura |
| `EEPROM.get(dir, struct)` | Leer estructura |
| `EEPROM.commit()` | Guardar en flash (¡importante!) |
| `EEPROM.end()` | Finalizar |

### Ejemplo de Checksum

```cpp
uint16_t calcChecksum(void* data, size_t len) {
  uint16_t sum = 0;
  uint8_t* bytes = (uint8_t*)data;
  for (size_t i = 0; i < len - 2; i++) {
    sum += bytes[i];
  }
  return sum;
}

// Guardar
config.checksum = calcChecksum(&config, sizeof(config));
EEPROM.put(0, config);
EEPROM.commit();

// Cargar y verificar
EEPROM.get(0, config);
uint16_t saved = config.checksum;
config.checksum = 0;
if (saved == calcChecksum(&config, sizeof(config))) {
  // Datos válidos
}
```

---

## 🌤️ HTTPClient (API Clima)

### Funciones Principales

| Función | Descripción |
|---------|-------------|
| `http.begin(url)` | Iniciar conexión |
| `http.setTimeout(ms)` | Timeout en milisegundos |
| `int code = http.GET()` | Realizar petición GET |
| `String resp = http.getString()` | Obtener respuesta |
| `http.end()` | Cerrar conexión |

### Códigos de Estado HTTP Comunes

| Código | Significado |
|--------|-------------|
| 200 | OK |
| 301 | Movido permanentemente |
| 302 | Redirección temporal |
| 400 | Bad request |
| 401 | No autorizado |
| 404 | No encontrado |
| 429 | Demasiadas peticiones |
| 500 | Error interno del servidor |

---

## 📦 ArduinoJson v6

### Parseo de JSON

```cpp
DynamicJsonDocument doc(16384);
DeserializationError error = deserializeJson(doc, jsonString);

if (!error) {
  float valor = doc["clave"]["subclave"] | -1.0;     // Con valor por defecto
  String texto = doc["clave"] | "default";            // String con default
  JsonArray arr = doc["arreglo"];                     // Arreglo
}
```

### Acceso a Datos Anidados

```json
{
  "hourly": [
    {
      "pop": 0.35,
      "weather": [
        { "description": "nubes dispersas" }
      ]
    }
  ]
}
```

```cpp
float pop = doc["hourly"][0]["pop"] | -1.0;
String desc = doc["hourly"][0]["weather"][0]["description"] | "";
```

---

## ⏰ NTP (Tiempo)

| Función | Descripción |
|---------|-------------|
| `configTime(gmtOffset, dstOffset, server)` | Configurar NTP |
| `time(&now)` | Obtener timestamp actual |
| `localtime_r(&now, &timeinfo)` | Convertir a estructura tm |
| `setenv("TZ", tz, 1)` | Configurar zona horaria |
| `tzset()` | Aplicar zona horaria |

### Estructura tm

```cpp
struct tm {
  int tm_sec;     // Segundos (0-59)
  int tm_min;     // Minutos (0-59)
  int tm_hour;    // Horas (0-23)
  int tm_mday;    // Día del mes (1-31)
  int tm_mon;     // Mes (0-11, enero=0)
  int tm_year;    // Año desde 1900
  int tm_wday;    // Día de la semana (0=domingo)
  int tm_yday;    // Día del año (0-365)
};
```

### Zonas Horarias

| País | Configuración |
|------|---------------|
| Chile | `setenv("TZ", "CLT", 1)` |
| Argentina | `setenv("TZ", "ART3", 1)` |
| Colombia | `setenv("TZ", "COT5", 1)` |
| México | `setenv("TZ", "CST6CDT,M4.1.0/2,M10.5.0/2", 1)` |
| España | `setenv("TZ", "CET-1CEST-2,M3.5.0/2,M10.5.0/2", 1)` |

---

## ⚡ GPIO (Pines)

### Funciones

| Función | Descripción |
|---------|-------------|
| `pinMode(pin, OUTPUT)` | Configurar como salida |
| `pinMode(pin, INPUT)` | Configurar como entrada |
| `pinMode(pin, INPUT_PULLUP)` | Entrada con resistencia pull-up |
| `digitalWrite(pin, HIGH)` | Encender pin (3.3V) |
| `digitalWrite(pin, LOW)` | Apagar pin (0V) |
| `digitalRead(pin)` | Leer estado del pin |

### Pines Comunes ESP32-S3

| GPIO | Función | Nota |
|------|---------|------|
| 0 | Boot button | INPUT_PULLUP, LOW al presionar |
| 1-14 | I/O | Disponibles |
| 2 | LED onboard | Muchas placas lo tienen |
| 4 | Relé (por defecto) | Usado en el proyecto |
| 18-21 | I/O | Disponibles |
| 40-48 | I/O | Algunos son solo entrada |

---

## 🕒 millis() - Temporización No Bloqueante

### Patrón Base

```cpp
unsigned long previousMillis = 0;
const long interval = 1000; // 1 segundo

void loop() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    // Acción periódica aquí (no bloqueante)
  }
}
```

### Conversiones Útiles

```
1 segundo = 1000 ms
1 minuto = 60,000 ms
1 hora   = 3,600,000 ms
120 min  = 7,200,000 ms  (límite de seguridad)
```

---

## 🧩 PROGMEM (Strings en Flash)

### Almacenar Strings Grandes

```cpp
const char miHtml[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
... contenido HTML ...
</html>
)rawliteral";

// Usar en el servidor
server.send(200, "text/html", FPSTR(miHtml));
// O con concatenación:
String page = FPSTR(miHtml);
```

### Ventajas
- Ahorra RAM valiosa (~300 bytes por página)
- El ESP32 tiene mucha flash pero poca RAM
- Las páginas HTML grandes deben ir en PROGMEM

---

## 🛠️ Comandos Útiles del Monitor Serial

| Comando | Baud rate | Descripción |
|---------|-----------|-------------|
| `Serial.begin(115200)` | 115200 | Velocidad estándar ESP32 |
| `Serial.println(msg)` | — | Imprimir con salto de línea |
| `Serial.printf(fmt, args)` | — | Formatear como printf |
| `Serial.read()` | — | Leer un byte |
| `Serial.readStringUntil('\n')` | — | Leer línea completa |
| `Serial.available()` | — | Verificar si hay datos |

### Atajos del Monitor Serial en Arduino IDE

| Atajo | Acción |
|-------|--------|
| Ctrl+Shift+M | Abrir Monitor Serial |
| Ctrl+Shift+L | Abrir Serial Plotter |
| Ctrl+U | Subir código |
| Ctrl+R | Compilar |

---

## 📐 Config.h - Constantes del Proyecto

```cpp
#define EEPROM_SIZE         512   // Tamaño EEPROM
#define MAX_SCHEDULES       4     // Máx. horarios
#define RELAY_PIN           4     // GPIO del relé
#define MAX_IRRIGATION_MIN  120   // Máx. minutos de riego
#define RAIN_THRESHOLD      90    // % lluvia para saltar
#define AP_SSID             "Riego-Hortaliza"
#define AP_PASS             "12345678"
#define NTP_SERVER          "pool.ntp.org"
#define OWM_HOST            "api.openweathermap.org"
#define WIFI_TIMEOUT_MS     10000 // Timeout WiFi STA
#define NTP_TIMEOUT_S       30    // Timeout NTP
```

---

## 📊 Códigos de Error Comunes

| Error | Causa | Solución |
|-------|-------|----------|
| `Brownout detector` | Bajo voltaje | Mejorar alimentación |
| `Guru Meditation Error` | Pánico del kernel | Revisar punteros null |
| `WiFi.disconnect()` | Señal débil | Acercar al router |
| `HTTP 401` | API Key inválida | Verificar API Key |
| `JSON parse error` | Formato inesperado | Verificar URL y respuesta |
| `EEPROM commit failed` | Ciclo de escritura | Esperar, reintentar |

---

> 💡 **Consejo Final:** Guarda esta guía como referencia rápida mientras desarrollas. Con la práctica, estos comandos se volverán naturales.
