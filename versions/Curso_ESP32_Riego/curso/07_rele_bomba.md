# Capítulo 7: Control de Relé y Bomba de Agua

**Duración:** 1 hora
**Objetivo:** Controlar de forma segura el relé que activa la bomba de agua

---

## 🎯 Objetivos del Capítulo

- Entender cómo funciona un relé y cómo se conecta al ESP32
- Implementar control de relé con temporización no bloqueante
- Configurar límites de seguridad (tiempo máximo de riego)
- Simular la bomba con un LED para pruebas seguras

---

## 📖 7.1 Teoría: Relés y Protección de Circuitos

### ¿Qué es un Relé?

Un relé es un interruptor electromecánico que permite controlar circuitos de alta potencia (bomba de agua 12V/24V) con una señal de baja potencia (GPIO 3.3V del ESP32).

```
ESP32 (3.3V) ──▶ Relé ──▶ Bomba (12V)
                      │
                      ├── Entrada: señal del ESP32
                      ├── VCC: 5V (alimentación del relé)
                      ├── GND: común
                      └── Contactos: NC - C - NO (para la bomba)
```

### Partes del Relé

| Terminal | Descripción |
|----------|-------------|
| **VCC** | Alimentación (5V típicamente) |
| **GND** | Tierra común |
| **IN** | Señal de control del ESP32 (HIGH = activa) |
| **NC** | Normalmente Cerrado (contacto cerrado en reposo) |
| **COM** | Común (el que se conecta) |
| **NO** | Normalmente Abierto (contacto se cierra al activar) |

### Esquema de Conexión

```
ESP32-S3                Módulo Relé 5V          Bomba 12V
┌─────────┐           ┌──────────────┐         ┌──────────┐
│ GPIO 4  │──────────▶│ IN           │         │          │
│         │           │              │         │   (+12V) │────▶ 12V+
│ 3.3V    │           │ VCC (5V)     │         │          │
│ GND     ├──────────▶│ GND          │    ┌────│   GND    │────◀ 12V-
│         │           │              │    │    └──────────┘
│ Vin (5V)├──────────▶│ VCC          │    │
└─────────┘           │ NO ──────────┘    │
                      │ COM ──────────────┘
                      │ NC  (sin usar)
                      └──────────────┘
```

### Protecciones Importantes

1. **Diodo flyback** — Los módulos relé modernos ya lo incluyen (protege contra picos de voltaje)
2. **Fuente separada** — La bomba debe tener su propia fuente. No alimentes la bomba desde el ESP32
3. **Optoacoplador** — Los buenos módulos relé tienen optoacoplador para aislar el ESP32 del circuito de potencia

> ⚠️ **ADVERTENCIA:** Nunca conectes una bomba de 12V directamente a un pin del ESP32. Siempre usa un módulo relé.

---

## 💻 7.2 Código: RelayControl — Temporización No Bloqueante

### ¿Por qué No Bloqueante?

```cpp
// ❌ MAL: Bloqueante
void activateRele(int minutes) {
  digitalWrite(RELAY_PIN, HIGH);
  delay(minutes * 60 * 1000); // ¡El ESP32 se congela!
  digitalWrite(RELAY_PIN, LOW);
}

// ✅ BIEN: No bloqueante
void activateRele(int minutes) {
  startTime = millis();
  duration = minutes * 60 * 1000;
  relayActive = true;
  digitalWrite(RELAY_PIN, HIGH);
}

void updateRele() {
  if (relayActive && (millis() - startTime >= duration)) {
    digitalWrite(RELAY_PIN, LOW);
    relayActive = false;
  }
}
```

### Clase Completa RelayControl

```cpp
class RelayControl {
private:
  uint8_t _pin;
  bool _active;
  unsigned long _startTime;
  unsigned long _durationMs;

public:
  RelayControl() : _pin(4), _active(false), 
                   _startTime(0), _durationMs(0) {}

  void begin(uint8_t pin) {
    _pin = pin;
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
    _active = false;
  }

  void activate(uint16_t durationMinutes) {
    // Límite de seguridad
    if (durationMinutes > MAX_IRRIGATION_MIN) {
      durationMinutes = MAX_IRRIGATION_MIN;
    }
    
    _active = true;
    _startTime = millis();
    _durationMs = (unsigned long)durationMinutes * 60 * 1000;
    digitalWrite(_pin, HIGH);
    
    Serial.printf("Relé ACTIVADO por %d minutos\n", durationMinutes);
  }

  void deactivate() {
    digitalWrite(_pin, LOW);
    _active = false;
    _startTime = 0;
    _durationMs = 0;
    Serial.println("Relé DESACTIVADO");
  }

  void update() {
    if (_active && (millis() - _startTime >= _durationMs)) {
      deactivate();
      Serial.println("Tiempo de riego completado");
    }
  }

  bool isActive() { return _active; }
  
  int getRemainingSeconds() {
    if (!_active) return 0;
    unsigned long elapsed = millis() - _startTime;
    if (elapsed >= _durationMs) return 0;
    return (int)((_durationMs - elapsed) / 1000);
  }
};
```

### Explicación

| Método | Descripción |
|--------|-------------|
| `begin(pin)` | Configura el GPIO como salida y apaga el relé |
| `activate(min)` | Activa el relé por X minutos con límite seguro |
| `deactivate()` | Apaga el relé inmediatamente |
| `update()` | Verifica si el tiempo expiró (llamar en loop) |
| `isActive()` | Consulta si el relé está activo |
| `getRemainingSeconds()` | Tiempo restante en segundos |

---

## 🔬 7.3 Laboratorio 7: Control de LED como Simulador de Bomba

### Objetivo
Usar un LED como simulador de la bomba para probar el control del relé de forma segura.

### Material para la Práctica
- LED (cualquier color)
- Resistencia de 220Ω
- Protoboard y cables

### Esquema de Conexión (Simulación)

```
ESP32                LED (simula bomba)
GPIO 4 ────┬─── 220Ω ────┬─── (ánodo, pata larga)
           │              │
           │         ┌────┴──── (cátodo, pata corta)
           │         │
          GND ───────┘
```

### Código de Prueba

```cpp
#include "RelayControl.h"

RelayControl relay;

void setup() {
  Serial.begin(115200);
  relay.begin(4); // GPIO 4 = LED simulador
  Serial.println("Sistema listo. Comandos:");
  Serial.println("  ON <min> - Activar relé por N minutos");
  Serial.println("  OFF      - Desactivar relé");
  Serial.println("  STATUS   - Ver estado");
}

void loop() {
  relay.update();

  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    
    if (cmd.startsWith("ON ")) {
      int minutes = cmd.substring(3).toInt();
      if (minutes < 1) minutes = 1;
      relay.activate(minutes);
    } else if (cmd == "OFF") {
      relay.deactivate();
    } else if (cmd == "STATUS") {
      Serial.print("Estado: ");
      Serial.println(relay.isActive() ? "ACTIVO" : "INACTIVO");
      if (relay.isActive()) {
        Serial.print("Tiempo restante: ");
        Serial.print(relay.getRemainingSeconds());
        Serial.println(" segundos");
      }
    }
  }
}
```

### Prueba
1. Escribe `ON 1` en el monitor serial → LED se enciende por 1 minuto
2. Escribe `STATUS` → muestra el tiempo restante
3. Escribe `OFF` → LED se apaga inmediatamente
4. Deja que el tiempo expire solo → LED se apaga automáticamente

---

## ✏️ 7.4 Ejercicio: Secuencia ON/OFF Programada

Crea un programa que ejecute la siguiente secuencia:

```
1. LED encendido 5 segundos
2. LED apagado 3 segundos
3. LED encendido 5 segundos
4. LED apagado 3 segundos
5. Repetir 3 veces
```

**Requisitos:** No usar `delay()`. Debe ser no bloqueante usando `millis()`.

### Pistas
```cpp
unsigned long previousMillis = 0;
int step = 0;
int repeatCount = 0;

void loop() {
  unsigned long now = millis();
  
  switch (step) {
    case 0: // Encender
      digitalWrite(4, HIGH);
      previousMillis = now;
      step = 1;
      break;
    case 1: // Esperar 5s
      if (now - previousMillis >= 5000) {
        digitalWrite(4, LOW);
        previousMillis = now;
        step = 2;
      }
      break;
    case 2: // Esperar 3s
      if (now - previousMillis >= 3000) {
        repeatCount++;
        if (repeatCount < 3) {
          step = 0; // Repetir
        } else {
          step = 3; // Terminar
        }
      }
      break;
    case 3: // Fin
      // No hacer nada
      break;
  }
}
```

---

## ✅ Resumen del Capítulo

| Concepto | Implementación |
|----------|---------------|
| ✅ Relé | Interruptor controlado por GPIO |
| ✅ Control no bloqueante | `millis()` en lugar de `delay()` |
| ✅ Límite de seguridad | `MAX_IRRIGATION_MIN = 120 min` |
| ✅ Timer automático | Se desactiva solo al vencer el tiempo |
| ✅ Simulación con LED | Puerto seguro para pruebas |
| ✅ Reporte de estado | `isActive()`, `getRemainingSeconds()` |

### Conexión Real vs Simulación

| Componente | Simulación (LED) | Real (Bomba) |
|-----------|------------------|--------------|
| Señal | GPIO → 220Ω → LED → GND | GPIO → Relé IN |
| Potencia | 5mA, 3.3V | 12V/24V, 1-5A |
| Alimentación | Del ESP32 | Fuente externa |
| Riesgo | Ninguno | ¡Cuidado! Alta tensión |

➡️ **Siguiente capítulo:** [08 - Integración y Proyecto Final](08_proyecto_integracion.md)
