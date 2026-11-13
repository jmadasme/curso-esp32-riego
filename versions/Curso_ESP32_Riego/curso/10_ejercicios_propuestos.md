# Ejercicios Propuestos

Ejercicios con enunciado, pistas y solución para reforzar los conceptos del curso

---

## 📋 Índice de Ejercicios

| # | Ejercicio | Capítulo | Dificultad |
|---|-----------|----------|------------|
| 1 | Escáner de redes WiFi | Cap. 1 | ⭐ |
| 2 | Portal cautivo con contador | Cap. 2 | ⭐ |
| 3 | Formulario con validación | Cap. 3 | ⭐⭐ |
| 4 | Contador persistente de reinicios | Cap. 4 | ⭐ |
| 5 | Depuración de horarios por Serial | Cap. 5 | ⭐⭐ |
| 6 | Probador de coordenadas climáticas | Cap. 6 | ⭐⭐ |
| 7 | Secuencia ON/OFF programada | Cap. 7 | ⭐⭐⭐ |
| 8 | Dashboard con datos históricos | Cap. 8 | ⭐⭐⭐ |

---

## Ejercicio 1: Escáner de Redes WiFi

**Capítulo:** 1 - WiFi y Hotspot
**Dificultad:** ⭐ Fácil

### Enunciado
Crea un programa que, al iniciar, escanee todas las redes WiFi disponibles y las muestre por el Monitor Serial ordenadas por intensidad de señal (RSSI), de mejor a peor señal.

### Requisitos
1. Mostrar número total de redes encontradas
2. Para cada red: número, SSID, RSSI (dBm), tipo de cifrado
3. Ordenar de mejor señal (RSSI más cercano a 0) a peor

### Pistas
- `WiFi.scanNetworks()` devuelve el número de redes
- `WiFi.RSSI(i)` da la intensidad
- `WiFi.encryptionType(i)` da el tipo de cifrado
- Para ordenar, puedes usar un array simple o mostrarlas ya procesadas

### Solución

```cpp
#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("Escaneando redes WiFi...");
  
  // Escanear
  int n = WiFi.scanNetworks();
  
  if (n == 0) {
    Serial.println("No se encontraron redes.");
  } else {
    Serial.print("Se encontraron ");
    Serial.print(n);
    Serial.println(" redes:");
    
    // Crear array de índices ordenados por RSSI
    int indices[n];
    for (int i = 0; i < n; i++) indices[i] = i;
    
    // Ordenar burbuja simple por RSSI (mejor señal primero)
    for (int i = 0; i < n - 1; i++) {
      for (int j = 0; j < n - i - 1; j++) {
        if (WiFi.RSSI(indices[j]) < WiFi.RSSI(indices[j + 1])) {
          int temp = indices[j];
          indices[j] = indices[j + 1];
          indices[j + 1] = temp;
        }
      }
    }
    
    // Mostrar redes ordenadas
    for (int i = 0; i < n; i++) {
      int idx = indices[i];
      Serial.print("  ");
      Serial.print(i + 1);
      Serial.print(". ");
      Serial.print(WiFi.SSID(idx));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(idx));
      Serial.print(" dBm) ");
      
      // Tipo de cifrado
      switch (WiFi.encryptionType(idx)) {
        case WIFI_AUTH_OPEN: Serial.print("[Abierta]"); break;
        case WIFI_AUTH_WEP: Serial.print("[WEP]"); break;
        case WIFI_AUTH_WPA_PSK: Serial.print("[WPA]"); break;
        case WIFI_AUTH_WPA2_PSK: Serial.print("[WPA2]"); break;
        default: Serial.print("[Otro]"); break;
      }
      Serial.println();
    }
  }
  
  WiFi.scanDelete(); // Liberar memoria
}

void loop() {}
```

---

## Ejercicio 2: Portal Cautivo con Contador

**Capítulo:** 2 - Captive Portal
**Dificultad:** ⭐ Fácil

### Enunciado
Modifica el portal cautivo para que muestre:
1. Cuántas veces se ha visitado la página principal
2. La IP de cada visitante
3. Un listado de las últimas 5 IPs que visitaron el portal

### Pistas
- Usa un array circular para guardar las últimas 5 IPs
- El contador es un `int` que se incrementa en cada visita
- `server.client().remoteIP()` da la IP del cliente

### Solución

```cpp
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>

const char* ssid = "Riego-Hortaliza";
const char* password = "[REDACTED]";
const IPAddress apIP(192, 168, 4, 1);

DNSServer dnsServer;
WebServer server(80);

int visitCount = 0;
String lastIPs[5];
int ipIndex = 0;

void addIP(String ip) {
  lastIPs[ipIndex] = ip;
  ipIndex = (ipIndex + 1) % 5;
}

String buildPage() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>Captive Portal</title>";
  html += "<style>body{font-family:sans-serif;text-align:center;padding:30px;background:#f0f7f0;}";
  html += ".card{background:white;padding:24px;border-radius:12px;max-width:500px;margin:0 auto;}";
  html += "h1{color:#2e7d32;} .num{font-size:48px;color:#1b5e20;font-weight:bold;}";
  html += "li{text-align:left;padding:4px 0;}</style></head><body>";
  html += "<div class='card'>";
  html += "<h1>🌱 Portal Cautivo</h1>";
  html += "<p>Visitas totales:</p>";
  html += "<p class='num'>" + String(visitCount) + "</p>";
  html += "<h3>Últimas visitas:</h3><ol>";
  for (int i = 0; i < 5; i++) {
    if (lastIPs[i].length() > 0) {
      html += "<li>" + lastIPs[i] + "</li>";
    }
  }
  html += "</ol></div></body></html>";
  return html;
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255,255,255,0));
  WiFi.softAP(ssid, password);
  
  dnsServer.start(53, "*", apIP);
  
  server.on("/", HTTP_GET, []() {
    visitCount++;
    String clientIP = server.client().remoteIP().toString();
    addIP(clientIP);
    server.send(200, "text/html", buildPage());
  });
  
  server.onNotFound([]() {
    server.sendHeader("Location", "http://192.168.4.1", true);
    server.send(302, "text/plain", "");
  });
  
  server.begin();
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
}
```

---

## Ejercicio 3: Formulario con Validación

**Capítulo:** 3 - Servidor Web
**Dificultad:** ⭐⭐ Intermedio

### Enunciado
Crea una página web con un formulario que permita configurar:
- **SSID WiFi** (requerido, máximo 31 caracteres)
- **Contraseña** (requerido, mínimo 8 caracteres)
- **Tiempo de riego** (número entre 1 y 120)
- **Modo de operación** (Automático / Manual / Apagado)

El formulario debe validar los datos antes de enviarlos (tanto en cliente con JavaScript como en servidor).

### Pistas
- Usa `pattern` en HTML5 para validación básica
- JavaScript para validación más completa antes del submit
- En el servidor, verifica cada campo con `if` antes de guardar

### Solución

```cpp
// Servidor (fragmento)
server.on("/save-config", HTTP_POST, []() {
  String ssid = server.arg("ssid");
  String pass = server.arg("pass");
  String time = server.arg("time");
  String mode = server.arg("mode");
  
  String error = "";
  
  // Validar
  if (ssid.length() == 0) error += "SSID requerido. ";
  if (ssid.length() > 31) error += "SSID muy largo. ";
  if (pass.length() < 8) error += "Pass mínimo 8 caracteres. ";
  
  int t = time.toInt();
  if (t < 1 || t > 120) error += "Tiempo debe ser 1-120. ";
  
  if (error.length() > 0) {
    server.send(400, "text/html", "<h2>Error:</h2><p>" + error + "</p>");
  } else {
    // Guardar configuración
    server.send(200, "text/html", "<h2>Configuración guardada!</h2>");
  }
});
```

```html
<!-- Cliente: Validación JavaScript -->
<script>
function validateForm() {
  const ssid = document.getElementById('ssid').value;
  const pass = document.getElementById('pass').value;
  const time = document.getElementById('time').value;
  let errors = [];
  
  if (ssid.length === 0) errors.push('SSID es requerido');
  if (pass.length < 8) errors.push('Contraseña mínimo 8 caracteres');
  if (time < 1 || time > 120) errors.push('Tiempo entre 1 y 120');
  
  if (errors.length > 0) {
    alert('Errores:\n- ' + errors.join('\n- '));
    return false;
  }
  return true;
}
</script>

<form onsubmit="return validateForm()" action="/save-config" method="POST">
  <input type="text" name="ssid" id="ssid" maxlength="31" required>
  <input type="password" name="pass" id="pass" minlength="8" required>
  <input type="number" name="time" id="time" min="1" max="120" value="10">
  <select name="mode">
    <option value="auto">Automático</option>
    <option value="manual">Manual</option>
    <option value="off">Apagado</option>
  </select>
  <button type="submit">Guardar</button>
</form>
```

---

## Ejercicio 4: Contador Persistente de Reinicios

**Capítulo:** 4 - EEPROM
**Dificultad:** ⭐ Fácil

### Enunciado
Crea un programa que:
1. Almacene un contador de reinicios en EEPROM
2. Cada vez que se encienda, incremente el contador
3. Muestre un mensaje como: "Encendido #42 - ¡Bienvenido!"
4. Si se presiona el botón BOOT (GPIO 0), resetee el contador a 0

### Solución

```cpp
#include <EEPROM.h>

#define EEPROM_SIZE 64
#define COUNT_ADDR 0
#define BOOT_PIN 0

void setup() {
  Serial.begin(115200);
  pinMode(BOOT_PIN, INPUT_PULLUP);
  
  EEPROM.begin(EEPROM_SIZE);
  
  int count = EEPROM.read(COUNT_ADDR);
  
  // Si se presiona BOOT, resetear
  if (digitalRead(BOOT_PIN) == LOW) {
    count = 0;
    Serial.println("🔄 Contador reseteado!");
  } else {
    count++;
  }
  
  EEPROM.write(COUNT_ADDR, count);
  EEPROM.commit();
  EEPROM.end();
  
  Serial.print("🔌 Encendido #");
  Serial.print(count);
  Serial.println(" - ¡Bienvenido!");
  
  // Mostrar instrucciones
  Serial.println("\nPresiona BOOT y RESET para resetear el contador");
}

void loop() {}
```

---

## Ejercicio 5: Depuración de Horarios por Serial

**Capítulo:** 5 - Programador
**Dificultad:** ⭐⭐ Intermedio

### Enunciado
Crea un programa que simule la ejecución de horarios y muestre en el Monitor Serial información detallada de cada disparo, incluyendo:
- Fecha y hora del disparo
- Programa ejecutado
- Duración configurada
- Próximo horario programado
- Tiempo restante para el próximo riego

### Solución

```cpp
void logScheduleExecution(int idx, Schedule &sched, struct tm *timeinfo) {
  Serial.println("═══════════════════════════════════");
  Serial.println("⏰ HORARIO DE RIEGO ACTIVADO");
  Serial.printf("📋 Programa: #%d\n", idx + 1);
  Serial.printf("🕐 Hora programada: %02d:%02d\n", sched.hour, sched.minute);
  Serial.printf("⏱ Duración: %d minutos\n", sched.duration);
  Serial.printf("📅 Fecha: %02d/%02d/%04d\n",
                timeinfo->tm_mday,
                timeinfo->tm_mon + 1,
                timeinfo->tm_year + 1900);
  Serial.printf("🕐 Hora actual: %02d:%02d:%02d\n",
                timeinfo->tm_hour,
                timeinfo->tm_min,
                timeinfo->tm_sec);
  
  // Encontrar próximo horario
  int nextIdx = -1;
  int nextHour = 24, nextMin = 60;
  for (int i = 0; i < MAX_SCHEDULES; i++) {
    if (config.schedules[i].enabled && i != idx) {
      int totalMin = config.schedules[i].hour * 60 + config.schedules[i].minute;
      int currentMin = timeinfo->tm_hour * 60 + timeinfo->tm_min;
      if (totalMin > currentMin && totalMin < (nextHour * 60 + nextMin)) {
        nextHour = config.schedules[i].hour;
        nextMin = config.schedules[i].minute;
        nextIdx = i;
      }
    }
  }
  
  if (nextIdx >= 0) {
    int diffMin = (nextHour * 60 + nextMin) - 
                  (timeinfo->tm_hour * 60 + timeinfo->tm_min);
    Serial.printf("⏳ Próximo riego: #%d a las %02d:%02d (en %d min)\n",
                  nextIdx + 1, nextHour, nextMin, diffMin);
  } else {
    Serial.println("⏳ No hay más horarios programados hoy");
  }
  
  Serial.println("═══════════════════════════════════\n");
}
```

---

## Ejercicio 6: Probador de Coordenadas Climáticas

**Capítulo:** 6 - API Clima
**Dificultad:** ⭐⭐ Intermedio

### Enunciado
Crea un programa que permita probar diferentes coordenadas geográficas y muestre la probabilidad de lluvia para cada una. El usuario debe poder ingresar coordenadas por Serial.

### Formato de Entrada
```
lat,lon  →  -33.45,-70.66
```

### Solución

```cpp
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

float getRainProbability(const char* apiKey, float lat, float lon) {
  if (WiFi.status() != WL_CONNECTED) return -1.0;
  
  HTTPClient http;
  String url = "http://api.openweathermap.org/data/2.5/onecall?";
  url += "lat=" + String(lat, 6) + "&lon=" + String(lon, 6);
  url += "&exclude=current,minutely,daily&appid=" + String(apiKey);
  
  http.begin(url);
  http.setTimeout(5000);
  int code = http.GET();
  
  if (code == HTTP_CODE_OK) {
    String payload = http.getString();
    DynamicJsonDocument doc(16384);
    deserializeJson(doc, payload);
    float pop = doc["hourly"][0]["pop"] | -1.0;
    String desc = doc["hourly"][0]["weather"][0]["description"] | "";
    Serial.printf("📍 %.4f, %.4f → 🌧️ %.0f%% - %s\n", lat, lon, pop * 100, desc.c_str());
    http.end();
    return pop;
  }
  
  http.end();
  return -1.0;
}

void setup() {
  Serial.begin(115200);
  WiFi.begin("TU_SSID", "TU_PASS");
  while (WiFi.status() != WL_CONNECTED) delay(500);
  Serial.println("WiFi conectado. Ingresa coordenadas: lat,lon");
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    
    int comma = input.indexOf(',');
    if (comma > 0) {
      float lat = input.substring(0, comma).toFloat();
      float lon = input.substring(comma + 1).toFloat();
      getRainProbability("TU_API_KEY", lat, lon);
    }
  }
}
```

---

## Ejercicio 7: Secuencia ON/OFF Programada

**Capítulo:** 7 - Control de Relé
**Dificultad:** ⭐⭐⭐ Avanzado

### Enunciado
Implementa una secuencia de riego por pulsos (riego por goteo simulado):
- Ciclo: 5 segundos ON, 10 segundos OFF
- Repetir 10 veces
- Sin usar `delay()` (no bloqueante)
- Mostrar el progreso por Serial

### Solución

```cpp
#include "RelayControl.h"

RelayControl relay;

enum SequenceState { IDLE, ON_PHASE, OFF_PHASE };
SequenceState state = IDLE;
unsigned long phaseStart = 0;
int cycleCount = 0;
const int TOTAL_CYCLES = 10;
const int ON_TIME = 5000;   // 5 segundos
const int OFF_TIME = 10000; // 10 segundos

void startSequence() {
  state = ON_PHASE;
  phaseStart = millis();
  cycleCount = 0;
  relay.activate(1); // Activar relé (mín 1 minuto, lo apagamos manual)
  Serial.println("🚿 Secuencia de riego por pulsos iniciada");
}

void updateSequence() {
  unsigned long now = millis();
  
  switch (state) {
    case ON_PHASE:
      if (now - phaseStart >= ON_TIME) {
        relay.deactivate();
        phaseStart = now;
        state = OFF_PHASE;
        cycleCount++;
        Serial.printf("🔴 Ciclo %d/%d - OFF\n", cycleCount, TOTAL_CYCLES);
      }
      break;
      
    case OFF_PHASE:
      if (now - phaseStart >= OFF_TIME) {
        if (cycleCount < TOTAL_CYCLES) {
          relay.activate(1);
          phaseStart = now;
          state = ON_PHASE;
          Serial.printf("🟢 Ciclo %d/%d - ON\n", cycleCount + 1, TOTAL_CYCLES);
        } else {
          state = IDLE;
          relay.deactivate();
          Serial.println("✅ Secuencia completada");
        }
      }
      break;
      
    case IDLE:
      break;
  }
}

void setup() {
  Serial.begin(115200);
  relay.begin(4);
  Serial.println("Presiona 'S' para iniciar secuencia");
}

void loop() {
  relay.update();
  
  if (state != IDLE) {
    updateSequence();
  }
  
  if (Serial.available() && Serial.read() == 'S' && state == IDLE) {
    startSequence();
  }
}
```

---

## Ejercicio 8: Dashboard con Datos Históricos

**Capítulo:** 8 - Integración
**Dificultad:** ⭐⭐⭐ Avanzado

### Enunciado
Amplía el dashboard para que muestre:
1. Historial de las últimas 10 activaciones de riego
2. Duración de cada riego
3. Fecha y hora de cada activación
4. Si fue automático o manual
5. Si se regó o se saltó por lluvia

Los datos deben persistir en EEPROM.

### Solución (Fragmento)

```cpp
struct RiegoLog {
  char timestamp[20];   // "2026-03-15 18:00:00"
  uint16_t duracion;     // minutos
  bool fueManual;        // true = manual, false = automático
  bool seRego;           // true = regó, false = saltado por lluvia
  char motivo[30];       // "Programado", "Manual", "Lluvia 85%"
};

#define MAX_LOGS 10
RiegoLog historial[MAX_LOGS];
int logIndex = 0;

void addLog(uint16_t duracion, bool manual, bool rego, String motivo) {
  // Obtener timestamp
  time_t now = time(nullptr);
  struct tm *t = localtime(&now);
  sprintf(historial[logIndex].timestamp, "%04d-%02d-%02d %02d:%02d:%02d",
          t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
          t->tm_hour, t->tm_min, t->tm_sec);
  
  historial[logIndex].duracion = duracion;
  historial[logIndex].fueManual = manual;
  historial[logIndex].seRego = rego;
  motivo.toCharArray(historial[logIndex].motivo, 30);
  
  logIndex = (logIndex + 1) % MAX_LOGS;
  
  // Guardar en EEPROM
  saveHistorial();
}

String buildHistorialHTML() {
  String html = "<div class='card'><h2>📜 Historial de Riegos</h2>";
  html += "<table style='width:100%;border-collapse:collapse;'>";
  html += "<tr style='background:#e8f5e9;'><th>Fecha</th><th>Duración</th><th>Tipo</th><th>Resultado</th></tr>";
  
  for (int i = 0; i < MAX_LOGS; i++) {
    int idx = (logIndex - 1 - i + MAX_LOGS) % MAX_LOGS;
    if (historial[idx].timestamp[0] == 0) continue;
    
    html += "<tr style='border-bottom:1px solid #c8e6c9;'>";
    html += "<td style='padding:8px;'>" + String(historial[idx].timestamp) + "</td>";
    html += "<td style='padding:8px;'>" + String(historial[idx].duracion) + " min</td>";
    html += "<td style='padding:8px;'>" + String(historial[idx].fueManual ? "Manual" : "Auto") + "</td>";
    html += "<td style='padding:8px;'>" + String(historial[idx].seRego ? "✅ Regó" : "⛔ Saltado") + "</td>";
    html += "</tr>";
  }
  
  html += "</table></div>";
  return html;
}
```

---

## 🌟 Bonus: Proyecto Final Integrador

Si completaste todos los ejercicios, ¡felicidades! Ahora tienes un sistema completo. Como bonus:

1. **Añade un sensor de flujo** para medir cuánta agua se ha usado
2. **Implementa un modo "vacaciones"** que riegue más seguido
3. **Crea una app web progresiva (PWA)** que se pueda instalar en el teléfono

---

> 💡 **Recuerda:** La mejor forma de aprender es experimentar. No tengas miedo de modificar el código y probar cosas nuevas.
