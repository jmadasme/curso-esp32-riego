# 📖 Explicación del Código Fuente

## Arquitectura del Firmware — Riego Inteligente ESP32-S3

---

## 1. Diagrama de Arquitectura General

```
┌──────────────────────────────────────────────────────────────────────┐
│                         ESP32-S3 (Arduino Framework)                  │
│                                                                       │
│  ┌─────────────────────────────────────────────────────────────────┐  │
│  │                      MAIN (esp32_riego_automatizado.ino)         │  │
│  │                                                                   │  │
│  │  setup():                                                         │  │
│  │    Serial.begin(115200)                                           │  │
│  │    EEPROM.begin(512)                                              │  │
│  │    loadConfig(config) ← Storage.h                                 │  │
│  │    captivePortal.begin() ← CaptivePortal.h                        │  │
│  │    WiFi.begin(ssid, pass)                                         │  │
│  │    relay.begin(pin) ← RelayControl.h                              │  │
│  │    configTime(...) ← NTP                                          │  │
│  │    setenv("TZ", "CLT")                                            │  │
│  │                                                                   │  │
│  │  loop():                                                          │  │
│  │    captivePortal.handleClient()  ← DNS + HTTP                     │  │
│  │    WiFi reconnect check                                           │  │
│  │    checkSchedules(config, timeinfo) ← Scheduler.h                 │  │
│  │    getRainProbability(...) ← WeatherCheck.h                       │  │
│  │    relay.activate() / relay.update() ← RelayControl.h             │  │
│  └─────────────────────────────────────────────────────────────────┘  │
│                                                                       │
│  ┌──────────┐  ┌──────────────┐  ┌──────────┐  ┌──────────────┐      │
│  │ Storage.h│  │CaptivePortal│  │Scheduler │  │WeatherCheck  │      │
│  │ (EEPROM) │  │.h (DNS+HTTP)│  │.h (Riego)│  │.h (Clima)    │      │
│  └────┬─────┘  └──────┬───────┘  └────┬─────┘  └──────┬───────┘      │
│       │               │               │               │              │
│       └───────┬───────┴───────┬───────┴───────┬───────┘              │
│               │               │               │                      │
│         ┌─────▼─────┐   ┌────▼──────┐   ┌────▼─────┐                │
│         │ WebInterf │   │RelayContro│   │  config.h │                │
│         │ ace.h     │   │l.h        │   │(Constantes│                │
│         │ (HTML/CSS)│   │(Relé/Bomba│   │Estructuras│                │
│         └───────────┘   └───────────┘   └──────────┘                │
└──────────────────────────────────────────────────────────────────────┘
```

---

## 2. Diagrama de Flujo del Sistema

```
                    ┌─────────────────────┐
                    │   ENCENDER ESP32     │
                    └──────────┬──────────┘
                               │
                    ┌──────────▼──────────┐
                    │    setup()           │
                    │  ─────────────────   │
                    │  Iniciar Serial      │
                    │  Iniciar EEPROM      │
                    │  Cargar config       │
                    │  Iniciar AP WiFi     │
                    │  Iniciar DNS Server  │
                    │  Iniciar Web Server  │
                    │  Conectar STA WiFi   │
                    │  Iniciar Relé        │
                    │  Sincronizar NTP     │
                    └──────────┬──────────┘
                               │
                    ┌──────────▼──────────┐
                    │      loop()          │◄────────────────────────────┐
                    │  ─────────────────   │                              │
                    │  handleClient()      │                              │
                    │  (DNS + HTTP)        │                              │
                    └──────────┬──────────┘                              │
                               │                                          │
                    ┌──────────▼──────────┐     ┌──────────────────────┐  │
                    │  ¿NTP sincronizado   │  No │ WiFi reconnect cada  │  │
                    │  y config cargada?   ├────►│ 30 segundos          ├──┤
                    └──────────┬──────────┘     └──────────────────────┘  │
                               │ Sí                                       │
                    ┌──────────▼──────────┐                              │
                    │  Obtener hora actual │                              │
                    │  time(&now)          │                              │
                    └──────────┬──────────┘                              │
                               │                                          │
                    ┌──────────▼──────────┐                              │
                    │  checkSchedules()   │                              │
                    │  ¿Coincide horario? │                              │
                    └──┬──────────────┬───┘                              │
                       │ No           │ Sí                               │
                       │              │                                  │
                       │     ┌────────▼────────┐                        │
                       │     │¿Ya se ejecutó   │                        │
                       │     │este horario hoy?│                        │
                       │     └──┬──────────┬───┘                        │
                       │        │ No       │ Sí                          │
                       │        │          │                             │
                       │   ┌────▼────┐     │                             │
                       │   │Consultar│     │                             │
                       │   │clima?   │     │                             │
                       │   └──┬──┬───┘     │                             │
                       │      │  │         │                             │
                       │      │  │         │                             │
                  ┌────┴──────┴──┴─────────┴───┐                       │
                  │   relay.update()            │                       │
                  │   delay(100)                ├───────────────────────┘
                  └─────────────────────────────┘
```

---

## 3. Módulos del Código

### 3.1 `config.h` — Constantes Globales y Estructuras

**Propósito:** Define todas las constantes del sistema y las estructuras de datos compartidas.

**Constantes principales:**

| Constante | Valor | Propósito |
|-----------|-------|-----------|
| `EEPROM_SIZE` | 512 | Tamaño de memoria persistente |
| `MAX_SCHEDULES` | 4 | Número máximo de horarios de riego |
| `RELAY_PIN` | 4 | GPIO por defecto para el relé |
| `MAX_IRRIGATION_MIN` | 120 | Duración máxima de riego (seguridad) |
| `RAIN_THRESHOLD` | 90 | Porcentaje de lluvia para saltar riego |
| `AP_SSID` | "Riego-Hortaliza" | Nombre del hotspot WiFi |
| `AP_PASS` | "12345678" | Contraseña del hotspot |
| `WIFI_TIMEOUT_MS` | 10000 | Timeout conexión WiFi STA |
| `NTP_TIMEOUT_S` | 30 | Timeout sincronización NTP |

**Estructuras:**

```cpp
struct Schedule {
  uint8_t hour;        // Hora del riego (0-23)
  uint8_t minute;      // Minuto del riego (0-59)
  uint16_t duration;   // Duración en minutos (1-120)
  bool enabled;        // Habilitado/deshabilitado
};

struct SystemConfig {
  char wifiSSID[32];                 // Red local para internet
  char wifiPass[64];                 // Contraseña de la red local
  char owmApiKey[48];               // API Key de OpenWeatherMap
  float latitude;                    // Latitud del cultivo
  float longitude;                   // Longitud del cultivo
  Schedule schedules[MAX_SCHEDULES]; // 4 horarios
  uint8_t relayPin;                  // GPIO del relé
  uint16_t checksum;                 // Integridad de datos
};
```

**Nota de diseño:** Las estructuras están optimizadas para ocupar exactamente 512 bytes (tamaño de EEPROM). El checksum permite detectar corrupción de datos.

---

### 3.2 `Storage.h` — Persistencia en EEPROM

**Propósito:** Guarda, carga y verifica la configuración del sistema en la EEPROM simulada del ESP32.

**Funciones:**

| Función | Parámetros | Retorno | Descripción |
|---------|-----------|---------|-------------|
| `saveConfig()` | `SystemConfig &config` | `void` | Escribe la estructura completa en EEPROM con checksum |
| `loadConfig()` | `SystemConfig &config` | `bool` | Lee y verifica checksum; retorna `true` si es válido |
| `clearConfig()` | — | `void` | Limpia toda la EEPROM (resetea a ceros) |
| `calculateChecksum()` | `SystemConfig &config` | `uint16_t` | Suma aritmética de 16 bits de todos los bytes excepto el checksum |

**Algoritmo de checksum:**

```
checksum = 0
for each byte in (config - last 2 bytes):
    checksum += byte
return checksum
```

**Flujo de guardado:**

```
1. Calcular checksum de la configuración actual
2. Asignar checksum a config.checksum
3. Escribir byte a byte (sizeof(SystemConfig) bytes) en EEPROM
4. Llamar EEPROM.commit() para guardar en flash
```

**Flujo de carga:**

```
1. Leer byte a byte desde EEPROM a la estructura
2. Guardar el checksum leído en variable temporal
3. Poner config.checksum = 0
4. Recalcular checksum
5. Comparar: si coinciden → datos íntegros; si no → configuración corrupta
```

---

### 3.3 `CaptivePortal.h` — Portal Cautivo + Servidor Web

**Propósito:** Implementa el hotspot WiFi con captive portal (DNS spoofing) y el servidor web de configuración.

**Clase:** `CaptivePortal`

**Miembros privados:**

| Miembro | Tipo | Descripción |
|---------|------|-------------|
| `_dnsServer` | `DNSServer` | Servidor DNS que intercepta todas las consultas |
| `_server` | `WebServer` | Servidor HTTP en puerto 80 |
| `_dnsPort` | `const byte` | Puerto DNS (53) |
| `_httpPort` | `const byte` | Puerto HTTP (80) |
| `_apIP` | `IPAddress` | IP del AP (192.168.4.1) |

**Métodos públicos:**

| Método | Descripción |
|--------|-------------|
| `CaptivePortal()` | Constructor, inicializa IP y servidor web |
| `begin()` | Inicia AP WiFi, DNS Server, Web Server y configura rutas |
| `handleClient()` | Procesa peticiones DNS y HTTP (llamar en loop) |

**Rutas del servidor web:**

| Ruta | Método | Función |
|------|--------|---------|
| `/` | GET | Página principal (dashboard) |
| `/config` | GET | Página de configuración general |
| `/schedules` | GET | Página de horarios de riego |
| `/save-config` | POST | Guarda configuración (WiFi, API, GPS) |
| `/save-schedules` | POST | Guarda horarios de riego |
| `/manual-irrigate` | POST | Activa riego manual con duración especificada |
| `/status` | GET | JSON con estado actual (AJAX) |
| `/irrigation-log` | GET | JSON con última acción |
| `*` (notFound) | ANY | Redirige al dashboard |

**Mecanismo de DNS Spoofing:**

```
Cliente busca "google.com"
  └── DNS query → ESP32 responde: "192.168.4.1"
Cliente busca cualquier dominio
  └── DNS query → ESP32 responde: "192.168.4.1"
Cliente hace HTTP a cualquier IP
  └── WebServer captura y redirige a "/"
```

**Nota de diseño:** Usa `WiFi.mode(WIFI_AP_STA)` para modo dual: el ESP32 es Access Point para los usuarios y simultáneamente cliente WiFi para acceder a internet.

---

### 3.4 `WebInterface.h` — Interfaz Web

**Propósito:** Genera todo el HTML, CSS y JavaScript que se sirve al navegador del usuario. El contenido está almacenado en PROGMEM (flash) para ahorrar RAM.

**Funciones generadoras de páginas:**

| Función | Retorna | Descripción |
|---------|---------|-------------|
| `buildMainPage()` | `String` | Dashboard principal con estado en vivo |
| `buildConfigPage()` | `String` | Formulario de configuración WiFi/API/GPS |
| `buildSchedulesPage()` | `String` | Formulario de horarios de riego |
| `buildStatusJSON()` | `String` | JSON para actualización AJAX |

**Recursos estáticos en PROGMEM:**

| Recurso | Tamaño aprox. | Descripción |
|---------|---------------|-------------|
| `STYLE_CSS` | ~3.5 KB | CSS completo con diseño responsive, colores verdes |
| `SHARED_JS` | ~1.5 KB | JavaScript para actualización automática cada 5s |

**Características del diseño:**

- **Paleta de colores:** verdes agro (#2e7d32, #4caf50, #1b5e20)
- **Layout:** tarjetas (cards), grid de estado, formularios compactos
- **Responsive:** adaptación a móviles con media queries
- **Actualización en vivo:** AJAX polling cada 5 segundos al endpoint `/status`
- **Interactividad:** toggles, selects, botones con feedback visual

**JavaScript de actualización automática:**

```javascript
function updateStatus() {
  fetch('/status')
    .then(response => response.json())
    .then(data => {
      // Actualiza: estado relé, tiempo restante,
      // última acción, clima, hora actual
    });
}
setInterval(updateStatus, 5000);
```

---

### 3.5 `Scheduler.h` — Programador de Horarios

**Propósito:** Compara la hora actual (obtenida vía NTP) contra los horarios configurados y determina si debe activarse el riego.

**Función principal:**

```cpp
int checkSchedules(SystemConfig &config, struct tm *timeinfo)
```

**Parámetros:**
- `config`: Estructura con los horarios configurados
- `timeinfo`: Estructura `tm` con la hora actual del sistema

**Retorno:**
- `-1`: Ningún horario coincide
- `0-3`: Índice del horario que coincide (para ejecutar)

**Algoritmo:**

```
for (i = 0; i < MAX_SCHEDULES; i++):
    if (config.schedules[i].enabled):
        if (schedules[i].hour == currentHour AND
            schedules[i].minute == currentMinute):
            return i  // Horario coincide
return -1  // Sin coincidencias
```

**Nota de diseño:** La ventana de ejecución es de 1 minuto (se verifica en cada iteración del loop, que ocurre cada ~100ms). El sistema principal usa `lastScheduleIdx` y `lastScheduleDay` para evitar ejecutar el mismo horario más de una vez en el mismo día.

---

### 3.6 `WeatherCheck.h` — Consulta Meteorológica

**Propósito:** Consulta la API OneCall de OpenWeatherMap para obtener la probabilidad de precipitación y la descripción del clima.

**Función principal:**

```cpp
float getRainProbability(const char* apiKey, float lat, float lon, String* weatherDesc)
```

**Parámetros:**
- `apiKey`: API Key de OpenWeatherMap
- `lat`, `lon`: Coordenadas del cultivo
- `weatherDesc` (opcional): Puntero para devolver descripción textual

**Retorno:**
- `0.0 - 1.0`: Probabilidad de precipitación
- `-1.0`: Error (sin conexión, timeout, parseo fallido, sin API key)

**URL construida:**

```
http://api.openweathermap.org/data/2.5/onecall
  ?lat={latitud}
  &lon={longitud}
  &exclude=current,minutely,daily
  &appid={api_key}
```

**Parseo de JSON:**

```json
{
  "hourly": [
    {
      "pop": 0.45,
      "weather": [
        { "description": "nubes dispersas" }
      ]
    }
  ]
}
```

**Modo seguro:** Si ocurre cualquier error (sin WiFi, timeout, HTTP error, JSON inválido), la función retorna `-1.0`. El sistema principal interpreta este valor como "regar por precaución", asumiendo que no hay datos de lluvia confiables.

---

### 3.7 `RelayControl.h` — Control de Relé

**Propósito:** Controla el relé de la bomba de agua con temporización no bloqueante y límites de seguridad.

**Clase:** `RelayControl`

**Miembros privados:**

| Miembro | Tipo | Descripción |
|---------|------|-------------|
| `_pin` | `uint8_t` | GPIO del relé |
| `_active` | `bool` | Estado actual (activo/inactivo) |
| `_startTime` | `unsigned long` | Momento de activación (millis) |
| `_durationMs` | `unsigned long` | Duración en milisegundos |
| `_maxDurationMs` | `unsigned long` | Máximo permitido (120 min en ms) |

**Métodos públicos:**

| Método | Parámetros | Descripción |
|--------|-------------|-------------|
| `begin(pin)` | `uint8_t` | Inicializa GPIO, estado LOW |
| `activate(minutes)` | `uint16_t` | Activa relé por N minutos (limitado a 120) |
| `deactivate()` | — | Desactiva relé inmediatamente |
| `update()` | — | Timer no bloqueante: desactiva si expiró |
| `isActive()` | — | Retorna `true` si el relé está activo |
| `getRemainingSeconds()` | — | Segundos restantes de riego |
| `getPin()` | — | GPIO configurado |

**Mecanismo de temporización no bloqueante:**

```cpp
void update() {
    if (!_active) return;

    unsigned long now = millis();
    unsigned long elapsed = now - _startTime;

    if (elapsed >= _durationMs) {
        deactivate();  // Se acabó el tiempo
    }
}
```

**Limitación de seguridad:**

```cpp
void activate(uint16_t durationMinutes) {
    if (durationMinutes > MAX_IRRIGATION_MIN) {
        durationMinutes = MAX_IRRIGATION_MIN;  // Forzar límite
    }
    // ... activar relé
}
```

---

## 4. Dependencias y Librerías

### Librerías del sistema (built-in del core ESP32)

| Librería | Archivo | Propósito |
|----------|---------|-----------|
| `WiFi.h` | CaptivePortal.h, WeatherCheck.h, main | Modos AP y STA WiFi |
| `WebServer.h` | CaptivePortal.h | Servidor HTTP en puerto 80 |
| `DNSServer.h` | CaptivePortal.h | Servidor DNS para captive portal |
| `EEPROM.h` | Storage.h, main | Persistencia en memoria flash |
| `HTTPClient.h` | WeatherCheck.h | Cliente HTTP para API REST |
| `time.h` | Scheduler.h, main | Funciones de tiempo y NTP |

### Librerías externas

| Librería | Versión | Archivo | Propósito |
|----------|---------|---------|-----------|
| `ArduinoJson.h` | ≥ v6.0 | WeatherCheck.h | Parseo de respuestas JSON |

### Instalación en Arduino IDE

1. Instalar el **core ESP32**: Archivo → Preferencias → URLs adicionales: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
2. Ir a Herramientas → Placa → Gestor de tarjetas → Buscar "ESP32" → Instalar
3. Instalar **ArduinoJson**: Herramientas → Gestor de librerías → Buscar "ArduinoJson" por Benoit Blanchon → Instalar v6.x
4. Seleccionar placa: `ESP32S3 Dev Module` (o la placa específica)

---

## 5. Personalización y Extensión

### 5.1 Agregar un Nuevo Horario (más de 4)

1. En `config.h`, aumentar `MAX_SCHEDULES` (ej: `#define MAX_SCHEDULES 6`)
2. En `main`, ajustar los valores por defecto en el `for` loop del `setup()`
3. En `WebInterface.h`, el `for (int i = 0; i < MAX_SCHEDULES; i++)` se ajusta automáticamente
4. Verificar que la estructura quepa en los 512 bytes de EEPROM

### 5.2 Agregar un Sensor de Humedad de Suelo

1. Crear un nuevo archivo `SoilMoisture.h`
2. Leer una entrada analógica (GPIO adecuado) con `analogRead()`
3. Integrar la lectura en el `loop()` antes de decidir el riego
4. Agregar la condición: si la humedad del suelo > umbral, saltar riego

### 5.3 Cambiar la Zona Horaria

En `esp32_riego_automatizado.ino`, línea 148:
```cpp
setenv("TZ", "CLT", 1);  // Cambiar "CLT" por la zona deseada
```

Zonas comunes:
- `"EST"` — Eastern Standard Time (UTC-5)
- `"CST"` — Central Standard Time (UTC-6)
- `"CET"` — Central European Time (UTC+1)
- `"BRT"` — Brasilia Time (UTC-3)
- `"ART"` — Argentina Time (UTC-3)

### 5.4 Agregar un Sensor de Flujo de Agua

1. Conectar un sensor de flujo (YF-S201) a un GPIO
2. Usar interrupción externa (`attachInterrupt`) para contar pulsos
3. En `RelayControl.h`, agregar lógica que detenga el riego al alcanzar cierto volumen

---

## 6. Consideraciones de Diseño

### 6.1 Gestión de Memoria

| Estrategia | Implementación |
|------------|---------------|
| **PROGMEM** | CSS y JS almacenados en flash en lugar de RAM |
| **Páginas dinámicas** | HTML se construye en RAM solo cuando se solicita |
| **JSON temporal** | `DynamicJsonDocument` de 16KB se asigna y libera en cada consulta climática |
| **Strings literales** | Uso de `F()` macro para almacenar strings en flash |

### 6.2 Seguridad por Diseño

| Mecanismo | Descripción |
|-----------|-------------|
| **Límite de tiempo** | El relé nunca puede estar activo más de 120 minutos seguidos |
| **Checksum EEPROM** | Detecta corrupción de datos; ante fallo, carga valores por defecto |
| **Modo seguro clima** | Si la API falla, el sistema riega por precaución (no asume lluvia) |
| **Anti-repetición** | Un horario no se ejecuta dos veces el mismo día |
| **Timeout de conexión** | WiFi, NTP y HTTP tienen timeouts que evitan bloqueos permanentes |

### 6.3 Limitaciones Conocidas

| Limitación | Impacto | Mitigación |
|------------|---------|------------|
| **millis() overflow (~50 días)** | El timer del relé podría fallar si el sistema lleva >50 días encendido | Reiniciar periódicamente el ESP32 (esperable en sistemas embebidos) |
| **String concatenation** | Puede fragmentar el heap con muchas páginas servidas | Aceptable para el uso esperado (1-2 usuarios simultáneos) |
| **TZ fijo** | Solo funciona para la zona horaria configurada en compilación | Modificar `setenv()` antes de compilar |
| **HTTP sin TLS** | Las credenciales viajan en texto plano en la red local | Aceptable para red local controlada |
| **Contraseña AP débil** | `12345678` es fácil de adivinar | Cambiar en `config.h` para producción |
