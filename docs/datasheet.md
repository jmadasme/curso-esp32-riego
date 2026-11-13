# 📋 Datasheet Técnico del Sistema

## Riego Inteligente ESP32-S3

---

## 1. Especificaciones Generales

| Parámetro | Valor |
|-----------|-------|
| **Nombre del producto** | Riego Inteligente ESP32-S3 |
| **Versión firmware** | 1.0.0 |
| **Placa objetivo** | ESP32-S3 DevKit C1 (u otras compatibles) |
| **Framework** | Arduino (ESP32 core 2.x+) |
| **Lenguaje** | C++11 |
| **IDE recomendado** | Arduino IDE 2.x / PlatformIO |
| **Licencia** | MIT |

---

## 2. Especificaciones Eléctricas

### Microcontrolador ESP32-S3

| Parámetro | Mínimo | Típico | Máximo | Unidad |
|-----------|--------|--------|--------|--------|
| **Voltaje de alimentación** | 4.5 | 5.0 | 5.5 | V |
| **Corriente en reposo** | — | 80 | — | mA |
| **Corriente con WiFi activo** | — | 200 | 500 | mA |
| **Corriente con relé activo** | — | 300 | 600 | mA |
| **Temperatura de operación** | -40 | 25 | 85 | °C |

### Módulo Relé (recomendado)

| Parámetro | Valor | Unidad |
|-----------|-------|--------|
| **Voltaje de bobina** | 5 | V |
| **Corriente de bobina** | 70-100 | mA |
| **Carga máxima (contactos)** | 10A @ 250VAC / 10A @ 30VDC | — |
| **Tiempo de conmutación** | 10 | ms |
| **Aislamiento** | 1500 | Vrms |
| **Vida mecánica** | 10,000,000 | ciclos |
| **Vida eléctrica** | 100,000 | ciclos (a carga nominal) |

---

## 3. Consumo Energético

| Modo de operación | Consumo estimado | Descripción |
|-------------------|-----------------|-------------|
| **Reposo (sin WiFi, sin relé)** | ~80 mA | Modo mínimo (no aplica en este firmware, siempre hay WiFi AP) |
| **Hotspot AP activo** | ~200 mA | ESP32 como Access Point, sin cliente conectado |
| **AP + Cliente WiFi** | ~250 mA | AP activo + conectado a red WiFi local |
| **Riego activo (relé ON)** | ~350 mA | Relé energizado + WiFi AP+STA |
| **Consulta API clima** | ~300 mA (pico) | HTTP GET + parseo JSON, duración ~2-5 segundos |
| **LED indicador** | ~20 mA adicional | LED verde encendido (si se usa como indicador) |

### Autonomía con Batería (estimada)

| Capacidad batería | Modo AP | Modo AP+STA | Modo riego continuo |
|-------------------|---------|-------------|---------------------|
| **2000 mAh (Li-ion)** | ~10 h | ~8 h | ~5 h |
| **5000 mAh (power bank)** | ~25 h | ~20 h | ~14 h |
| **10000 mAh (power bank)** | ~50 h | ~40 h | ~28 h |

---

## 4. Conectividad

### WiFi

| Parámetro | Especificación |
|-----------|---------------|
| **Estándar** | IEEE 802.11 b/g/n |
| **Banda** | 2.4 GHz |
| **Modos** | AP (Access Point) + STA (Station) simultáneos |
| **Canales** | 1-13 (depende de región) |
| **Potencia TX** | Hasta 20 dBm |
| **Seguridad AP** | WPA2-PSK |
| **SSID por defecto** | `Riego-Hortaliza` |
| **Contraseña AP** | `12345678` |
| **Número máximo clientes AP** | 4 (limitación ESP32) |
| **IP del AP** | 192.168.4.1 |

### Protocolos Soportados

| Protocolo | Uso |
|-----------|-----|
| **TCP/IP** | Comunicación HTTP con clientes web |
| **DNS** | Captive portal (interceptación de consultas DNS) |
| **HTTP/1.1** | Servidor web interno (GET/POST) |
| **NTP** | Sincronización de hora (pool.ntp.org) |
| **HTTPS** | No implementado (el servidor web es HTTP; la consulta a OWM es HTTP) |

---

## 5. Pines y GPIO

| GPIO | Función por defecto | Configurable | Notas |
|------|--------------------|--------------|-------|
| **GPIO 4** | Control de relé | Sí (en EEPROM) | Salida digital, HIGH = activo |
| **GPIO 2** | LED indicador | No | Salida digital, HIGH = encendido |
| **GPIO 1 (UART0 TX)** | Serial TX | No | Depuración (115200 baud) |
| **GPIO 3 (UART0 RX)** | Serial RX | No | Depuración |
| **GPIO 8/9 (USB)** | USB Serial/JTAG | No | Programación y monitoreo |

> **Nota:** Si el LED incorporado de la placa interfiere con el GPIO 2, puedes cambiar el pin del relé vía web (Configuración) o modificar `RELAY_PIN` en `config.h`.

---

## 6. Memoria y Almacenamiento

| Tipo | Capacidad | Uso |
|------|-----------|-----|
| **Flash** | 16 MB (típico en ESP32-S3) | Código, PROGMEM (CSS, JS), sistema de archivos |
| **PSRAM** | 8 MB (si la placa lo incluye) | Variables dinámicas, heap |
| **SRAM interna** | 512 KB | Variables globales, stack, heap |
| **EEPROM (simulada)** | 512 bytes | Persistencia de configuración del sistema |

### Layout de EEPROM

| Offset | Tamaño | Contenido |
|--------|--------|-----------|
| 0 | 32 | WiFi SSID (char[32]) |
| 32 | 64 | WiFi Password (char[64]) |
| 96 | 48 | OpenWeatherMap API Key (char[48]) |
| 144 | 4 | Latitud (float) |
| 148 | 4 | Longitud (float) |
| 152 | 1 | GPIO pin del relé (uint8_t) |
| 153 | 160 | Schedules[4] (40 bytes c/u, con padding) |
| 313 | 197 | Reserva para futuras expansiones |
| 510 | 2 | Checksum (uint16_t) |
| **Total** | **512** | |

---

## 7. Formatos de Datos

### Estado del Sistema (JSON - `/status`)

```json
{
  "relayActive": false,
  "remainingSeconds": 0,
  "lastAction": "Riego iniciado - Programacion #1 (Lluvia: 15%)",
  "rainProb": 0.15,
  "weatherDesc": "cielo claro",
  "currentTime": "14:32:07"
}
```

### Log de Riego (JSON - `/irrigation-log`)

```json
{
  "lastAction": "Riego iniciado - Programacion #1 (Lluvia: 15%)",
  "lastScheduleIdx": 0,
  "lastScheduleDay": 158
}
```

### Configuración Guardada (EEPROM)

| Campo | Tipo | Rango |
|-------|------|-------|
| wifiSSID | char[32] | Hasta 31 caracteres ASCII |
| wifiPass | char[64] | Hasta 63 caracteres ASCII |
| owmApiKey | char[48] | Hasta 47 caracteres alfanuméricos |
| latitude | float | -90.0 a 90.0 |
| longitude | float | -180.0 a 180.0 |
| relayPin | uint8_t | 0-47 (pines GPIO disponibles) |
| schedules[4].hour | uint8_t | 0-23 |
| schedules[4].minute | uint8_t | 0-59 |
| schedules[4].duration | uint16_t | 1-120 (minutos) |
| schedules[4].enabled | bool | true/false |
| checksum | uint16_t | Suma aritmética de todos los bytes previos |

---

## 8. Compatibilidad

### Placas Compatibles

| Placa | Funciona | Notas |
|-------|----------|-------|
| **ESP32-S3 DevKit C1** | ✅ Sí | Placa de referencia del proyecto |
| **ESP32 DevKit V1** | ✅ Sí | Requiere cambiar board en Arduino IDE |
| **ESP32-WROOM-32** | ✅ Sí | Mismas capacidades WiFi |
| **ESP32-C3** | ⚠️ Parcial | No tiene WiFi AP+STA simultáneo completo |
| **ESP8266** | ❌ No | RAM insuficiente para el servidor web completo |
| **ESP32-S2** | ⚠️ Parcial | No tiene WiFi STA+AP simultáneo |

### Navegadores Web Soportados (para la interfaz)

| Navegador | Funciona | Notas |
|-----------|----------|-------|
| **Chrome 90+** | ✅ Sí | Full soporte |
| **Firefox 88+** | ✅ Sí | Full soporte |
| **Edge 90+** | ✅ Sí | Full soporte |
| **Safari 14+** | ✅ Sí | Full soporte |
| **Chrome Android** | ✅ Sí | Experiencia óptima en móviles |
| **Safari iOS** | ✅ Sí | Diseño responsive adaptado |
| **Internet Explorer** | ❌ No | No soportado (falta de CSS Grid/Flexbox) |

---

## 9. Rangos de Operación

| Parámetro | Valor | Condición |
|-----------|-------|-----------|
| **Duración máxima de riego** | 120 minutos | Límite de seguridad en firmware |
| **Máximo de horarios** | 4 programas diarios | Definido en `MAX_SCHEDULES` |
| **Umbral de lluvia** | 90% probabilidad | Configurable en `RAIN_THRESHOLD` |
| **Timeout conexión WiFi** | 10 segundos | Antes de pasar a modo solo AP |
| **Timeout NTP** | 30 segundos | Antes de desactivar scheduler |
| **Timeout HTTP clima** | 5 segundos | Antes de modo seguro (regar) |
| **Intervalo reconexión WiFi** | 30 segundos | Reintento automático |
| **Tamaño EEPROM** | 512 bytes | Fijo, no configurable |
| **Clientes AP simultáneos** | 4 máx. | Límite del ESP32 en modo AP |

---

## 10. Indicadores LED

| Estado del LED (GPIO 2) | Significado |
|------------------------|-------------|
| **Apagado** | Sistema operando, relé inactivo |
| **Encendido fijo** | Relé activo (riegando) |
| **Parpadeo rápido** | Error de inicialización |
| **Parpadeo lento** | Configuración no encontrada (valores por defecto) |

> **Nota:** El LED del GPIO 2 es el LED azul integrado en la mayoría de placas ESP32 DevKit. En algunas placas ESP32-S3 el LED puede estar en otro GPIO.

---

## 11. Mensajes de Serial (Debug)

### Nivel de Información
El sistema imprime mensajes por Serial a **115200 baud** con el formato:
```
[Modulo] Mensaje descriptivo
```

### Prefijos por Módulo

| Prefijo | Módulo |
|---------|--------|
| `[Main]` | Sistema principal (setup/loop) |
| `[Captive]` | Captive portal / DNS |
| `[Relay]` | Control de relé |
| `[Scheduler]` | Programador de horarios |
| `[Weather]` | Consulta meteorológica |
| `[Storage]` | Persistencia EEPROM |

---

## 12. Dimensiones Físicas (Referencia)

| Componente | Largo | Ancho | Alto | Peso |
|------------|-------|-------|------|------|
| ESP32-S3 DevKit C1 | 54 mm | 26 mm | 12 mm | 8 g |
| Módulo relé 1 canal | 25 mm | 35 mm | 18 mm | 12 g |
| Fuente 5V 2A | 60 mm | 35 mm | 25 mm | 40 g |
| Bomba 12V (mini) | 80 mm | 40 mm | 50 mm | 100 g |
| Caja estanca IP65 | 150 mm | 100 mm | 70 mm | 150 g |
