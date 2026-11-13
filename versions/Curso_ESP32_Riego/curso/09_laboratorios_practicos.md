# Laboratorios Prácticos

Guía detallada paso a paso para cada laboratorio del curso

---

## 📋 Índice de Laboratorios

| # | Laboratorio | Capítulo | Duración |
|---|------------|----------|----------|
| 1 | Primer Hotspot WiFi | Cap. 1 | 30 min |
| 2 | Captive Portal Funcional | Cap. 2 | 30 min |
| 3 | Web Responsive Dashboard | Cap. 3 | 45 min |
| 4 | EEPROM Persistente | Cap. 4 | 20 min |
| 5 | Programador de Horarios | Cap. 5 | 30 min |
| 6 | Clima API | Cap. 6 | 30 min |
| 7 | Control de Relé | Cap. 7 | 20 min |
| 8 | Proyecto Integrado | Cap. 8 | 45 min |

---

## 🔬 Laboratorio 1: Primer Hotspot WiFi

**Duración:** 30 minutos
**Objetivo:** Crear un hotspot WiFi personalizado desde el ESP32

### Material
- ESP32-S3
- Cable USB

### Instrucciones

**Paso 1: Configurar Arduino IDE**
1. Abre Arduino IDE
2. Ve a Archivo → Preferencias
3. En "URLs adicionales de gestor de placas" añade:
   ```
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
   ```
4. Ve a Herramientas → Placa → Gestor de placas
5. Busca "ESP32" e instala el paquete

**Paso 2: Seleccionar placa y puerto**
- Placa: `ESP32S3 Dev Module`
- Puerto: El que corresponda (COM3, /dev/ttyACM0, etc.)

**Paso 3: Escribir el código**
```cpp
#include <WiFi.h>

const char* ssid = "MiRiego-XXXX";
const char* password = "[REDACTED]";

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\nCreando hotspot...");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  
  Serial.print("IP del AP: ");
  Serial.println(WiFi.softAPIP());
  Serial.println("Hotspot creado exitosamente!");
}

void loop() {
  Serial.print("Clientes: ");
  Serial.println(WiFi.softAPgetStationNum());
  delay(5000);
}
```

**Paso 4: Subir y probar**
1. Conecta el ESP32 al PC
2. Presiona el botón de subida (→)
3. Abre el Monitor Serial (115200 baud)
4. Desde tu teléfono, busca la red "MiRiego-XXXX"
5. Conéctate con la contraseña "12345678"
6. Observa cómo el contador de clientes aumenta

### Verificación
- [ ] La red WiFi aparece en el teléfono
- [ ] La conexión es exitosa
- [ ] El Monitor Serial muestra clientes conectados
- [ ] La IP del AP es 192.168.4.1

---

## 🔬 Laboratorio 2: Captive Portal Funcional

**Duración:** 30 minutos
**Objetivo:** Implementar un captive portal completo

### Código Base
Usa el código del [Capítulo 2](02_captive_portal.md) sección 2.2.

### Pasos
1. Copia el código completo del portal cautivo
2. Modifica el SSID a tu gusto
3. Sube al ESP32
4. Conéctate al WiFi
5. Abre el navegador y escribe cualquier URL
6. Debes ser redirigido automáticamente

### Modificaciones Sugeridas
```cpp
// Añadir contador de visitas
int visitas = 0;

server.on("/", HTTP_GET, []() {
  visitas++;
  String page = mainPage;
  page.replace("{{visitas}}", String(visitas));
  server.send(200, "text/html", page);
});
```

### Verificación
- [ ] Te redirige automáticamente desde cualquier URL
- [ ] La página se ve correctamente
- [ ] Muestra la IP del cliente
- [ ] Las rutas no definidas redirigen al portal

---

## 🔬 Laboratorio 3: Web Responsive Dashboard

**Duración:** 45 minutos
**Objetivo:** Crear un dashboard moderno y responsive

### Diseño a Implementar

Crea un dashboard que muestre:

```
┌────────────────────────────────┐
│  🌱 Riego Inteligente          │
│  Dashboard de Control          │
├────────────────────────────────┤
│ ┌────────┐ ┌────────┐         │
│ │ Relé   │ │ Tiempo │         │
│ │ ACTIVO │ │ 5:30   │         │
│ └────────┘ └────────┘         │
│ ┌────────┐ ┌────────┐         │
│ │ Clima  │ │ Última │         │
│ │ 25°C   │ │ acción │         │
│ └────────┘ └────────┘         │
├────────────────────────────────┤
│ [Riego Manual] [Configuración] │
└────────────────────────────────┘
```

### Características
- CSS grid responsive
- Colores verdes/agrícolas
- Fuente legible (Segoe UI)
- Tarjetas con sombras
- Botones con hover effects

### Verificación
- [ ] El diseño se ve bien en móvil (responsive)
- [ ] Los colores son agradables
- [ ] Los botones tienen efectos hover
- [ ] La información está organizada claramente

---

## 🔬 Laboratorio 4: EEPROM Persistente

**Duración:** 20 minutos
**Objetivo:** Guardar y recuperar datos tras reinicios

### Código Base
```cpp
#include <EEPROM.h>

#define EEPROM_SIZE 64

void setup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);
  
  // Leer contador de reinicios
  int count = EEPROM.read(0);
  count++;
  
  Serial.print("Reinicio #");
  Serial.println(count);
  
  // Guardar nuevo contador
  EEPROM.write(0, count);
  EEPROM.commit();
  
  EEPROM.end();
}

void loop() {}
```

### Prueba
1. Sube el código
2. Abre Monitor Serial → "Reinicio #1"
3. Presiona RESET en el ESP32
4. Verás "Reinicio #2"
5. Desconecta y vuelve a conectar la alimentación
6. El contador continúa incrementándose

### Extensión: Guardar Estructura
```cpp
struct Config {
  char nombre[16];
  int valor;
};

void guardarConfig(Config &cfg) {
  EEPROM.begin(128);
  EEPROM.put(0, cfg);
  EEPROM.commit();
  EEPROM.end();
}

void cargarConfig(Config &cfg) {
  EEPROM.begin(128);
  EEPROM.get(0, cfg);
  EEPROM.end();
}
```

### Verificación
- [ ] Los datos persisten tras reinicio
- [ ] El contador se incrementa correctamente
- [ ] Las estructuras se guardan y recuperan

---

## 🔬 Laboratorio 5: Programador de Horarios

**Duración:** 30 minutos
**Objetivo:** Configurar horarios desde la web y ejecutarlos

### Pasos
1. Integra el Scheduler con la interfaz web
2. Crea el formulario de horarios (4 programas)
3. Implementa la lógica de coincidencia horaria
4. Prueba configurando un horario 2 minutos después de la hora actual
5. Verifica que el sistema lo ejecuta

### Código de Prueba Rápida
```cpp
// Simular hora para pruebas
int testHour = 14;
int testMin = 30;

// En lugar de NTP, usa valores fijos para probar
struct tm testTime;
testTime.tm_hour = testHour;
testTime.tm_min = testMin;

int idx = checkSchedules(config, &testTime);
if (idx >= 0) {
  Serial.printf("Horario #%d ejecutado!\n", idx + 1);
}
```

### Verificación
- [ ] Puedes configurar horarios desde la web
- [ ] Los horarios se guardan en EEPROM
- [ ] La lógica de coincidencia funciona
- [ ] No se ejecuta el mismo horario dos veces

---

## 🔬 Laboratorio 6: Clima API

**Duración:** 30 minutos
**Objetivo:** Consultar OpenWeatherMap y mostrar datos en la web

### Pasos
1. Regístrate en openweathermap.org
2. Obtén tu API Key gratuita
3. Implementa el cliente HTTP
4. Muestra los datos en el dashboard
5. Prueba la lógica antilluvia

### Prueba Manual
```cpp
void setup() {
  Serial.begin(115200);
  WiFi.begin("TuSSID", "TuPassword");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  float prob = getRainProbability("tu_api_key", -33.45, -70.66);
  Serial.printf("Probabilidad de lluvia: %.0f%%\n", prob * 100);
}

void loop() {}
```

### Verificación
- [ ] La API responde correctamente
- [ ] El JSON se parsea sin errores
- [ ] La probabilidad se muestra en la web
- [ ] Sin API Key, el sistema riega igual (modo seguro)

---

## 🔬 Laboratorio 7: Control de Relé

**Duración:** 20 minutos
**Objetivo:** Controlar un LED como simulador de bomba

### Conexión
```
ESP32 GPIO 4 ─── 220Ω ────┬─── LED Ánodo (+)
                          │
                      GND ──── LED Cátodo (-)
```

### Código
```cpp
#include "RelayControl.h"

RelayControl relay;

void setup() {
  Serial.begin(115200);
  relay.begin(4);
  
  Serial.println("Comandos: ON <min>, OFF, STATUS");
}

void loop() {
  relay.update();
  
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    
    if (cmd.startsWith("ON ")) {
      int min = cmd.substring(3).toInt();
      relay.activate(min);
    } else if (cmd == "OFF") {
      relay.deactivate();
    } else if (cmd == "STATUS") {
      Serial.printf("Activo: %d, Restante: %ds\n",
                    relay.isActive(),
                    relay.getRemainingSeconds());
    }
  }
}
```

### Verificación
- [ ] LED se enciende con comando ON
- [ ] LED se apaga con comando OFF
- [ ] LED se apaga solo tras el tiempo configurado
- [ ] El tiempo límite de 120 min se respeta
- [ ] STATUS muestra información correcta

---

## 🔬 Laboratorio 8: Proyecto Integrado

**Duración:** 45 minutos
**Objetivo:** Integrar todos los módulos y probar el sistema completo

### Pasos

**Fase 1: Preparación (10 min)**
1. Revisa que todos los archivos del proyecto están en su lugar
2. Verifica las conexiones: ESP32 → LED (simulando bomba)
3. Asegúrate de tener una red WiFi disponible para internet

**Fase 2: Carga (5 min)**
1. Abre `esp32_riego_automatizado.ino` en Arduino IDE
2. Selecciona la placa ESP32-S3 y el puerto correcto
3. Compila y sube el código

**Fase 3: Prueba de Hotspot (5 min)**
1. Espera 10 segundos tras el encendido
2. Busca la red "Riego-Hortaliza" desde tu teléfono
3. Conéctate con la contraseña "12345678"
4. Abre el navegador → debes ver el dashboard

**Fase 4: Configuración (5 min)**
1. Ve a Configuración
2. Ingresa tu red WiFi (con internet)
3. Ingresa API Key de OpenWeatherMap
4. Ingresa coordenadas de tu ciudad
5. Guarda

**Fase 5: Programar Horarios (5 min)**
1. Configura un horario 2 minutos después de la hora actual
2. Duración: 1 minuto
3. Habilita el horario
4. Guarda

**Fase 6: Prueba de Riego Automático (10 min)**
1. Espera a que llegue la hora programada
2. Observa cómo el LED se enciende (simula bomba)
3. Verifica que el dashboard muestra el estado activo
4. Espera que se apague automáticamente
5. Revisa el log de última acción

**Fase 7: Prueba de Persistencia (5 min)**
1. Desconecta la alimentación del ESP32
2. Vuelve a conectar
3. Verifica que la configuración y horarios persisten

### Checklist Final

- [ ] Hotspot funciona correctamente
- [ ] Captive portal redirige
- [ ] Dashboard se ve bien en móvil
- [ ] Configuración se guarda y persiste
- [ ] Horarios se ejecutan automáticamente
- [ ] Riego manual funciona
- [ ] Clima se consulta (si hay API Key)
- [ ] Sin API Key, el sistema funciona igual
- [ ] LED se apaga tras tiempo programado
- [ ] Todo funciona tras reinicio

---

> 💡 **Consejo:** Si algo no funciona, revisa el Monitor Serial. Los mensajes de depuración te ayudarán a identificar el problema.
