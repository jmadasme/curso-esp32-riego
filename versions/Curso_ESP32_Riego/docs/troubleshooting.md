# 🛠️ Troubleshooting y FAQ

## Guía de Solución de Problemas — Riego Inteligente ESP32-S3

---

## 1. Problemas Comunes y Soluciones

### 1.1 El ESP32 No Enciende

| Síntoma | Posible causa | Solución |
|---------|---------------|----------|
| No se enciende ningún LED | Sin alimentación | Verificar que el cable USB esté conectado y la fuente funcione (probar con otro cable) |
| | Fuente insuficiente | Usar fuente de 5V y al menos 2A. No usar puertos USB de computadora viejos |
| | Placa dañada | Probar con otro ESP32 o verificar con multímetro voltaje entre 5V y GND |
| LED rojo encendido, pero no arranca | Bootloop o código defectuoso | Mantener presionado BOOT mientras se presiona RESET, luego soltar BOOT |
| | Puerto USB incorrecto | Algunas placas tienen dos puertos USB; usar el que está etiquetado como "UART" |

### 1.2 No Aparece la Red WiFi "Riego-Hortaliza"

| Síntoma | Posible causa | Solución |
|---------|---------------|----------|
| La red no aparece en la lista WiFi | ESP32 no inició el AP | Verificar por Serial que el mensaje "AP iniciado" aparezca. Si no, reiniciar |
| | WiFi del teléfono en 5GHz | El ESP32 solo emite en 2.4GHz. Asegúrate de que tu dispositivo busque redes 2.4GHz |
| | Interferencia | Alejar el ESP32 de fuentes de interferencia (microondas, routers cercanos) |
| | Placa en modo solo STA | Verificar que `WiFi.mode(WIFI_AP_STA)` se ejecute en CaptivePortal.begin() |
| La red aparece pero no conecta | Contraseña incorrecta | La contraseña por defecto es `12345678` (8 caracteres, sin espacios) |
| | Demasiados clientes | El ESP32 soporta máximo 4 clientes simultáneos en modo AP |

**Verificación por Serial:**
```
[Captive] AP iniciado: SSID='Riego-Hortaliza', PASS='12345678'
[Captive] IP del AP: 192.168.4.1
```
Si no ves este mensaje, hay un problema en `CaptivePortal.begin()`.

### 1.3 El Portal Web No Se Abre

| Síntoma | Posible causa | Solución |
|---------|---------------|----------|
| El navegador muestra "No se puede conectar" | No conectado al hotspot | Verificar que estás conectado a la red "Riego-Hortaliza" |
| | Captive portal no redirige | Abrir manualmente `http://192.168.4.1` en el navegador |
| | DNS no funciona | Algunos dispositivos cachean DNS. Probar en modo incógnito o reiniciar WiFi del dispositivo |
| La página carga pero sin estilos | CSS no cargó | Actualizar la página. Si persiste, verificar que el HTML se sirva completo (mirar Serial) |
| | Navegador antiguo | Usar Chrome, Firefox, Edge o Safari modernos (no Internet Explorer) |
| Error 404 al navegar | Ruta incorrecta | El ESP32 solo reconoce las rutas: `/`, `/config`, `/schedules`, `/status`. Cualquier otra redirige a `/` |

**Comando rápido de prueba desde terminal (Linux/macOS):**
```bash
curl http://192.168.4.1/status
# Debe responder con un JSON como:
# {"relayActive":false,"remainingSeconds":0,...}
```

### 1.4 El Relé No Se Activa

| Síntoma | Posible causa | Solución |
|---------|---------------|----------|
| El relé no hace "click" al iniciar riego | GPIO incorrecto | Verificar que `relayPin` en la configuración coincida con el GPIO conectado |
| | Pin no configurado como OUTPUT | Verificar que `pinMode(_pin, OUTPUT)` se ejecute en `RelayControl.begin()` |
| | Relé necesita 5V pero recibe 3.3V | La señal GPIO es 3.3V, pero el relé debe alimentarse con 5V (VCC) |
| | Relé dañado | Probar el relé independientemente: conectar VCC a 5V, GND a GND, IN a 5V (debe activarse) |
| El relé hace "click" pero la bomba no funciona | Sin alimentación 12V | Verificar que la fuente 12V esté conectada y funcione |
| | Bomba dañada | Probar la bomba directamente con 12V |
| | Conexión incorrecta | Verificar: COM -> fuente 12V+, NO -> bomba+, GND bomba -> fuente 12V- |
| El relé se activa pero no se desactiva | Límite de 120 minutos alcanzado? | El sistema fuerza desactivación a los 120 min máximo |
| | Error de software | Verificar Serial: debe aparecer "[Relay] Tiempo de riego completado" |

**Prueba manual del relé:**
```cpp
// Código de prueba independiente
void setup() {
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);  // Activar relé
  delay(5000);            // Esperar 5 segundos
  digitalWrite(4, LOW);   // Desactivar relé
}
void loop() {}
```

### 1.5 El Sistema No Riega en el Horario Programado

| Síntoma | Posible causa | Solución |
|---------|---------------|----------|
| No riega a la hora indicada | NTP no sincronizado | Verificar `ntpSynced = true` en Serial. Si es `false`, el scheduler no se ejecuta |
| | Horario deshabilitado | Ir a la web -> Horarios -> verificar que el toggle esté en "ON" |
| | Hora incorrecta | Verificar que la zona horaria esté correcta (`setenv("TZ", ...)` en el código) |
| | Ya se ejecutó hoy | El sistema no ejecuta el mismo horario dos veces el mismo día |
| Riega pero no debía (lluvia) | API Key no configurada | Sin API Key, el sistema riega siempre (modo seguro) |
| | WiFi STA desconectado | Sin internet no consulta clima, riega por precaución |
| | Umbral de lluvia muy alto | `RAIN_THRESHOLD = 90`: solo salta si >=90%. Bajar a 70 si quieres más sensibilidad |
| No riega aunque NTP funciona | Hora del sistema inválida | Verificar `timeinfo->tm_year > (2024 - 1900)` - el sistema descarta años anteriores |
| | Error en Serial | Buscar "[Scheduler]" o "[Main]" para ver la decisión tomada |

**Verificar NTP manualmente:**
Conecta el ESP32 a internet y espera 30 segundos. En el monitor Serial deberías ver:
```
[Main] Hora NTP sincronizada: 14:30:00 07/06/2026
```
Si no aparece tras 30 segundos, la conexión WiFi STA puede estar fallando.

### 1.6 La Configuración No Se Guarda

| Síntoma | Posible causa | Solución |
|---------|---------------|----------|
| Al reiniciar, se pierde la config | Checksum inválido | La EEPROM está protegida contra corrupción. Si el checksum falla, carga defaults |
| | Guardado incompleto | Asegurarse de que `EEPROM.commit()` se ejecute (revisar Storage.h) |
| | EEPROM_SIZE insuficiente | La estructura `SystemConfig` debe ser menor a 512 bytes. Verificar con `sizeof()` |
| Solo algunos campos se guardan | Struct padding | El compilador puede alinear miembros. Usar `#pragma pack(1)` si es necesario |
| | Múltiples writes sin commit | Llamar `EEPROM.commit()` después de cada escritura |

**Diagnóstico desde Serial:**
```
[Storage] Configuración guardada correctamente en EEPROM.
[Storage] Configuración cargada correctamente (checksum OK).
```
O si hay error:
```
[Storage] Error de checksum. Configuración corrupta o vacía.
```

### 1.7 La Consulta del Clima Falla (API OpenWeatherMap)

| Síntoma | Posible causa | Solución |
|---------|---------------|----------|
| "Weather: Error: No hay API Key" | API Key no configurada | Ir a Configuración en la web -> ingresar API Key de OpenWeatherMap |
| "Weather: Error: WiFi no conectado" | STA no conectado | Verificar que el ESP32 tenga internet (Configuración -> WiFi SSID) |
| "HTTP Response code: 401" | API Key inválida | Verificar que la API Key sea correcta. Obtener una gratis en openweathermap.org |
| "HTTP Response code: 404" | URL incorrecta | Verificar que `OWM_HOST = "api.openweathermap.org"` en config.h |
| "Error parseando JSON" | Límite de plan gratuito | La API OneCall gratuita tiene límite de 1000 llamadas/día. Esperar o actualizar plan |
| | JSON corrupto | Verificar conectividad. Si el JSON llega truncado, el parseo falla |
| "Probabilidad: -1.00" | Error genérico | Revisar Serial para más detalles del error específico |

**Prueba manual de la API:**
```bash
# Reemplaza {API_KEY} con tu clave real
curl "http://api.openweathermap.org/data/2.5/onecall?lat=-33.45&lon=-70.65&exclude=current,minutely,daily&appid={API_KEY}"
```
Debe devolver un JSON con `hourly[0].pop` (probabilidad de lluvia).

### 1.8 Errores de Compilación

| Error | Causa | Solución |
|-------|-------|----------|
| `'ArduinoJson.h' file not found` | Librería no instalada | Instalar ArduinoJson v6 desde el Gestor de Librerías |
| `'WebServer.h' file not found` | Core ESP32 no instalado | Instalar el core ESP32 en Arduino IDE |
| `'DNSServer.h' file not found` | Core ESP32 desactualizado | Actualizar a la última versión del core ESP32 |
| `Multiple definition of 'config'` | Variables globales duplicadas | Usar `extern` en los headers, definir solo en el .ino |
| `PROGMEM was not declared` | Placa incorrecta | Verificar que la placa seleccionada sea ESP32 (no Arduino Uno) |
| `Struct Schedule redefinition` | Inclusión múltiple | Asegurar `#ifndef`/`#define`/`#endif` en cada header |

---

## 2. FAQ - Preguntas Frecuentes

### Generales

**P: Puedo usar este sistema con cualquier ESP32?**
R: Funciona en ESP32 y ESP32-S3. En ESP32-C3 y S2 no funciona el modo AP+STA simultáneo. No funciona en ESP8266.

**P: Necesito Internet para que funcione el riego?**
R: No. El sistema funciona en modo solo hotspot (AP) sin internet. En ese caso, el riego se ejecuta según los horarios sin verificación climática. Para la función anti-lluvia sí necesitas internet.

**P: Cuánta memoria RAM usa el sistema?**
R: Aproximadamente 20-40 KB de RAM dinámica, dejando suficiente espacio para el stack y otras operaciones. El ESP32-S3 tiene 512 KB de SRAM.

**P: Puedo agregar más de 4 horarios de riego?**
R: Sí, modificando `MAX_SCHEDULES` en `config.h`. Debes verificar que la estructura `SystemConfig` no exceda los 512 bytes de EEPROM.

**P: El sistema soporta múltiples bombas o zonas de riego?**
R: En la versión actual solo controla un relé (una zona). Para múltiples zonas, necesitarías modificar `RelayControl.h` para manejar varios relés y agregar configuración adicional en EEPROM.

### Conexión y Red

**P: Puedo cambiar el nombre de la red WiFi del hotspot?**
R: Sí, editando `AP_SSID` en `config.h`. Luego recompila y sube el firmware.

**P: La contraseña del hotspot es muy débil, puedo cambiarla?**
R: Sí, modifica `AP_PASS` en `config.h`. Debe tener al menos 8 caracteres para compatibilidad WPA2.

**P: Puedo acceder al portal web desde internet (no solo desde el hotspot)?**
R: No en la versión actual. El servidor web solo escucha en la interfaz del Access Point (192.168.4.1). Para acceso remoto necesitarías añadir un túnel o forwarding.

**P: Qué pasa si hay muchos dispositivos conectados al hotspot?**
R: El ESP32 soporta hasta 4 clientes simultáneos en modo AP. Si se excede, los nuevos clientes no podrán conectar.

### Clima y Riego

**P: Qué tan precisa es la probabilidad de lluvia?**
R: La API de OpenWeatherMap usa modelos meteorológicos. La precisión depende de tu región y del horizonte de pronóstico. Para la próxima hora suele ser bastante fiable.

**P: Puedo usar otro servicio meteorológico en lugar de OpenWeatherMap?**
R: Sí, puedes modificar `WeatherCheck.h` para que consulte otra API (WeatherAPI, AccuWeather, etc.). Deberás adaptar el parseo JSON.

**P: Qué significa "Modo seguro" en la verificación climática?**
R: Si la API falla (sin internet, timeout, error HTTP), el sistema **asume que no va a llover y riega por precaución**. Es mejor regar de más que dejar el cultivo sin agua por un error técnico.

**P: El riego manual respeta el límite de 120 minutos?**
R: Sí. El límite se aplica tanto a riego programado como manual.

### Mantenimiento

**P: Con qué frecuencia debo reiniciar el ESP32?**
R: No es necesario hacerlo periódicamente. Sin embargo, debido al overflow de `millis()` (~50 días), se recomienda un reinicio cada 30 días para garantizar el correcto funcionamiento del timer del relé.

**P: Cómo reseteo la configuración a valores de fábrica?**
R: Actualmente no hay un botón "Reset de fábrica" en la web. Puedes:
1. Modificar el código para llamar a `clearConfig()` y recompilar
2. O escribir un sketch temporal que solo ejecute `clearConfig()` y lo subas al ESP32

**P: Puedo actualizar el firmware OTA (sin cable)?**
R: No en la versión actual. Es una mejora planificada. Por ahora, usa el cable USB.

---

## 3. Debugging vía Serial

### Configuración del Monitor Serial

| Parámetro | Valor |
|-----------|-------|
| **Baud rate** | 115200 |
| **Data bits** | 8 |
| **Parity** | None |
| **Stop bits** | 1 |
| **Line ending** | Newline (NL) |

### Flujo de Mensajes de Arranque Normal

```
============================================
  Riego Inteligente ESP32-S3 v1.0
============================================
[Main] EEPROM iniciada (512 bytes)
[Storage] Configuración cargada correctamente (checksum OK).
[Main] WiFi SSID: MiRedLocal
[Main] Lat/Lon: -33.4567, -70.6543

============================================
  Captive Portal - Riego Inteligente ESP32
============================================
[Captive] AP iniciado: SSID='Riego-Hortaliza', PASS='12345678'
[Captive] IP del AP: 192.168.4.1
[Captive] DNS Server iniciado en puerto 53
[Captive] Todo el tráfico DNS redirigido a 192.168.4.1
[Captive] Servidor web HTTP iniciado en puerto 80
============================================

[Main] Conectando a WiFi STA: MiRedLocal ...
.....
[Main] WiFi STA conectado. IP: 192.168.1.100
[Relay] Inicializado en GPIO 4
[Main] Sincronizando hora vía NTP...
..............................
[Main] Hora NTP sincronizada: 14:30:00 07/06/2026
============================================
[Main] Sistema listo. Conéctate al WiFi 'Riego-Hortaliza'
[Main] y abre http://192.168.4.1 en tu navegador.
============================================
```

### Mensajes Durante la Operación

**Cuando se detecta un horario:**
```
[Scheduler] Horario 1 coincide: 08:00 (duración: 15 min)
```

**Cuando se consulta el clima:**
```
[Weather] Consultando: http://api.openweathermap.org/...
[Weather] HTTP Response code: 200
[Weather] Probabilidad de lluvia: 0.15 (15%)
```

**Cuando se toma la decisión de riego:**
```
[Main] Riego iniciado - Programación #1 (Lluvia: 15%)
[Relay] ACTIVADO por 15 minutos (GPIO 4)
```

**Cuando se salta el riego por lluvia:**
```
[Main] Riego SALTADO - Programación #1 (Lluvia: 95%)
```

**Cuando el riego programado termina:**
```
[Relay] Tiempo de riego completado. Desactivando.
[Relay] DESACTIVADO
```

### Códigos de Error en Serial

| Mensaje en Serial | Significado | Acción recomendada |
|-------------------|-------------|-------------------|
| `ERROR: No se pudo iniciar el AP` | El hotspot WiFi falló | Reiniciar el ESP32. Si persiste, el módulo WiFi podría estar dañado |
| `No se pudo conectar a WiFi STA` | No hay conexión a internet | Verificar SSID/contraseña en la configuración web |
| `Error de checksum. Configuración corrupta o vacía` | EEPROM dañada o primera vez | El sistema carga valores por defecto. Reconfigurar desde la web |
| `No se pudo sincronizar NTP` | Sin acceso a internet o servidor NTP bloqueado | Verificar WiFi STA. Si el problema persiste, el scheduler no funcionará |
| `Error: No hay API Key configurada` | Clima no disponible | Configurar API Key en la web o ignorar (el sistema regará sin verificación) |
| `Duración X excede el máximo (120 min). Limitando.` | Intento de riego demasiado largo | El sistema limita automáticamente a 120 min |
| `[Weather] HTTP Response code: 401` | API Key inválida | Obtener una nueva API Key en openweathermap.org |
| `[Weather] Error parseando JSON` | Problema con la respuesta de la API | Verificar conectividad. Si el JSON llega incompleto, el parseo falla |

---

## 4. Checklist de Diagnóstico Rápido

Si el sistema no funciona como esperas, sigue esta secuencia de verificación:

```
1. Alimentación
   [ ] El ESP32 enciende (LED de power)
   [ ] Se ve la red "Riego-Hortaliza" en el celular

2. Conexión
   [ ] Te conectas al hotspot con contraseña 12345678
   [ ] Abres http://192.168.4.1 en el navegador
   [ ] Ves el dashboard del sistema

3. Serial (si tienes cable USB)
   [ ] Conectas USB y abres monitor Serial a 115200 baud
   [ ] Ves mensajes de inicio del sistema
   [ ] Ves "AP iniciado" y "Servidor web HTTP iniciado"

4. Configuración
   [ ] Configuras WiFi STA (si tienes internet)
   [ ] Configuras API Key de OpenWeatherMap (opcional)
   [ ] Configuras coordenadas GPS (opcional)
   [ ] Configuras al menos 1 horario de riego habilitado

5. Riego
   [ ] Esperas la hora programada
   [ ] O activas riego manual desde la web
   [ ] Escuchas el "click" del relé
   [ ] Ves el LED indicador encenderse
```

---

## 5. Recursos de Ayuda Adicional

| Recurso | Enlace / Descripción |
|---------|---------------------|
| **Código fuente completo** | Ver directorio `codigo/esp32_riego_automatizado/` |
| **Material del curso** | Ver directorio `curso/` (15 horas de contenido) |
| **OpenWeatherMap API** | https://openweathermap.org/api (registro gratuito) |
| **Documentación ESP32** | https://docs.espressif.com/projects/arduino-esp32/ |
| **Foro de ayuda** | Reporta issues en el repositorio de GitHub |
| **ArduinoJson** | https://arduinojson.org/ (documentación y asistente de cálculo) |
