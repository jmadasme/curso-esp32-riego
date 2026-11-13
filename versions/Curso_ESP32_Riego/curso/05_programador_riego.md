# Capítulo 5: Programador de Riego

**Duración:** 1.5 horas
**Objetivo:** Implementar la lógica de programación horaria para el riego automatizado

---

## 🎯 Objetivos del Capítulo

- Diseñar el algoritmo de scheduling para horarios de riego
- Implementar la estructura Schedule con hora, minuto y duración
- Integrar la hora NTP para saber la hora actual
- Comparar hora actual con horarios programados
- Evitar ejecuciones duplicadas del mismo horario

---

## 📖 5.1 Teoría: Algoritmos de Scheduling

### ¿Qué es un Scheduler?

Un scheduler (programador) es un componente que ejecuta acciones en momentos específicos. En nuestro caso, activar el riego a ciertas horas del día.

### Tipos de Scheduling

| Tipo | Descripción | Ejemplo |
|------|-------------|---------|
| **One-shot** | Una sola vez | Riego único el 15 de marzo |
| **Diario** | Todos los días a la misma hora | Riego cada día a las 8:00 |
| **Semanal** | Días específicos de la semana | Lunes, miércoles, viernes |
| **Intervalo** | Cada cierto tiempo | Cada 6 horas |

En nuestro proyecto usaremos **scheduling diario** con 4 horarios configurables.

### Algoritmo de Comparación Temporal

```
Para cada horario habilitado:
  1. Comparar hora_actual con hora_programada
  2. Comparar minuto_actual con minuto_programado
  3. Si coinciden → ejecutar riego
  4. Marcar como ejecutado (para no repetir en el mismo minuto)
```

**Ventana de detección:** 1 minuto (para evitar saltos por micro-segundos)

---

## 💻 5.2 Código: Estructura Schedule y Lógica de Coincidencia

### La Estructura Schedule

```cpp
struct Schedule {
  uint8_t hour;      // Hora (0-23)
  uint8_t minute;    // Minuto (0-59)
  uint16_t duration; // Duración en minutos (1-120)
  bool enabled;      // true = habilitado
};
```

### Función de Verificación

```cpp
int checkSchedules(SystemConfig &config, struct tm *timeinfo) {
  if (timeinfo == nullptr) return -1;

  int currentHour = timeinfo->tm_hour;
  int currentMin = timeinfo->tm_min;

  for (int i = 0; i < MAX_SCHEDULES; i++) {
    if (config.schedules[i].enabled) {
      if (config.schedules[i].hour == currentHour &&
          config.schedules[i].minute == currentMin) {
        return i; // Horario coincide
      }
    }
  }

  return -1; // Ningún horario coincide
}
```

### Control de Ejecución Única por Día

Para evitar que un horario se ejecute múltiples veces en el mismo minuto:

```cpp
// Variables globales
int lastScheduleIdx = -1;
int lastScheduleDay = -1;

// En el loop:
if (idx >= 0) {
  bool alreadyExecuted = (idx == lastScheduleIdx && 
                          timeinfo->tm_yday == lastScheduleDay);
  
  if (!alreadyExecuted) {
    // Ejecutar riego
    relay.activate(config.schedules[idx].duration);
    
    // Marcar como ejecutado
    lastScheduleIdx = idx;
    lastScheduleDay = timeinfo->tm_yday;
  }
}
```

---

## ⏰ 5.3 Sincronización NTP

### ¿Qué es NTP?

NTP (Network Time Protocol) permite sincronizar la hora del ESP32 con servidores de tiempo en internet.

### Código NTP

```cpp
#include <time.h>

// Configurar NTP
configTime(-3 * 3600, 0, "pool.ntp.org");
// El primer parámetro es el offset en segundos (UTC-3 = -3*3600)

// Esperar sincronización
time_t now = 0;
int attempts = 0;
while (now < 100000 && attempts < 30) {
  delay(1000);
  now = time(nullptr);
  attempts++;
}

if (now > 100000) {
  Serial.println("NTP sincronizado!");
  struct tm* timeinfo = localtime(&now);
  Serial.printf("Hora: %02d:%02d:%02d\n",
                timeinfo->tm_hour,
                timeinfo->tm_min,
                timeinfo->tm_sec);
}
```

### Zonas Horarias (Timezone)

```cpp
// Configurar timezone para Chile (UTC-3)
setenv("TZ", "CLT", 1);
tzset();

// Otras zonas horarias comunes:
// España:      setenv("TZ", "CET-1CEST-2,M3.5.0/2,M10.5.0/3", 1);
// México:      setenv("TZ", "CST6CDT,M4.1.0/2,M10.5.0/2", 1);
// Argentina:   setenv("TZ", "ART3", 1);
// Colombia:    setenv("TZ", "COT5", 1);
```

---

## 🔬 5.4 Laboratorio 5: Programar 4 Horarios desde la Web

### Objetivo
Crear un sistema que permita configurar 4 horarios desde la interfaz web y los ejecute automáticamente.

### Estructura de la Página de Horarios

```html
<form action="/save-schedules" method="POST">
  <!-- Programa 1 -->
  <div class="schedule-card">
    <h3>Programa #1</h3>
    <label>Hora:
      <select name="s0_hour">
        <option value="0">00</option>
        <!-- ... horas 0-23 ... -->
      </select>
    </label>
    <label>Minuto:
      <select name="s0_min">
        <option value="0">00</option>
        <option value="5">05</option>
        <!-- ... cada 5 minutos ... -->
      </select>
    </label>
    <label>Duración (min):
      <input type="number" name="s0_dur" value="10" min="1" max="120">
    </label>
    <label>
      <input type="checkbox" name="s0_en" value="1"> Habilitado
    </label>
  </div>
  <!-- Repetir para s1, s2, s3 -->
  <button type="submit">Guardar Horarios</button>
</form>
```

### Procesamiento en el ESP32

```cpp
server.on("/save-schedules", HTTP_POST, [this]() {
  for (int i = 0; i < MAX_SCHEDULES; i++) {
    String prefix = "s" + String(i);
    
    config.schedules[i].hour = (uint8_t)server.arg(prefix + "_hour").toInt();
    config.schedules[i].minute = (uint8_t)server.arg(prefix + "_min").toInt();
    config.schedules[i].duration = (uint16_t)server.arg(prefix + "_dur").toInt();
    
    // Limitar duración máxima
    if (config.schedules[i].duration > MAX_IRRIGATION_MIN) {
      config.schedules[i].duration = MAX_IRRIGATION_MIN;
    }
    
    config.schedules[i].enabled = (server.arg(prefix + "_en") == "1");
  }
  
  saveConfig(config);
  server.send(200, "text/html", "Horarios guardados");
});
```

---

## ✏️ 5.5 Ejercicio: Depurar con Serial los Disparos

Modifica el sistema para que cuando un horario se active, muestre en el Monitor Serial:

```
═══════════════════════════════════
⏰ HORARIO DE RIEGO ACTIVADO
📋 Programa: #2
🕐 Hora: 18:00
⏱ Duración: 15 minutos
📅 Fecha: 15/03/2026
═══════════════════════════════════
```

### Pistas

Usa `Serial.printf()` para formatear la salida:

```cpp
Serial.println("═══════════════════════════════════");
Serial.println("⏰ HORARIO DE RIEGO ACTIVADO");
Serial.printf("📋 Programa: #%d\n", idx + 1);
Serial.printf("🕐 Hora: %02d:%02d\n", 
              config.schedules[idx].hour, 
              config.schedules[idx].minute);
Serial.printf("⏱ Duración: %d minutos\n", config.schedules[idx].duration);
Serial.printf("📅 Fecha: %02d/%02d/%04d\n",
              timeinfo->tm_mday,
              timeinfo->tm_mon + 1,
              timeinfo->tm_year + 1900);
Serial.println("═══════════════════════════════════");
```

---

## ✅ Resumen del Capítulo

| Concepto | Implementación |
|----------|---------------|
| ✅ Estructura Schedule | `hour`, `minute`, `duration`, `enabled` |
| ✅ Comparación horaria | Comparar hora/minuto actual con programado |
| ✅ Ventana de 1 minuto | Coincidencia exacta de hora y minuto |
| ✅ Anti-repetición | Verificar `lastScheduleIdx` y `lastScheduleDay` |
| ✅ NTP | `configTime()`, `time()`, `localtime_r()` |
| ✅ Límite de seguridad | `MAX_IRRIGATION_MIN = 120` |

### Flujo del Programador

```
NTP → hora actual → checkSchedules() → ¿Coincide?
                                           │
                              ┌────────────┴────────────┐
                              ▼                         ▼
                          Sí (riego)                 No (esperar)
                              │
                    ¿Ya ejecutado hoy?
                              │
                    ┌─────────┴─────────┐
                    ▼                   ▼
                  No (ejecutar)      Sí (saltar)
```

➡️ **Siguiente capítulo:** [06 - Integración con API de Clima](06_api_clima.md)
