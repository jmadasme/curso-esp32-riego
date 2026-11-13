# 🔌 Diagrama de Conexión

## Guía Paso a Paso para el Cableado del Sistema

---

## 1. Lista de Materiales (BOM)

### Componentes Electrónicos

| # | Componente | Especificación | Cantidad | Costo aprox. |
|---|------------|---------------|:--------:|:------------:|
| 1 | **ESP32-S3 DevKit C1** | Cualquier placa ESP32-S3 con USB-C | 1 | $8-15 USD |
| 2 | **Módulo Relé 1 canal** | 5V, activo en HIGH, 10A | 1 | $3-5 USD |
| 3 | **Bomba de agua** | 12V DC, 3-10W (opcional) | 1 | $5-15 USD |
| 4 | **Fuente de poder 5V** | 5V DC, 2A, con conector USB o terminal | 1 | $5-8 USD |
| 5 | **Fuente de poder 12V** | 12V DC, 1A (solo si usas bomba) | 1 | $5-8 USD |
| 6 | **LED difuso 5mm** | Color verde o azul (simulación) | 1 | $0.10 USD |
| 7 | **Resistencia 220Ω** | 1/4W, carbon film | 1 | $0.05 USD |
| 8 | **Cables jumper** | Macho-hembra y macho-macho | 20 | $2-3 USD |
| 9 | **Protoboard** | 830 puntos (media+ tamaño) | 1 | $3-5 USD |
| 10 | **Caja estanca** | IP65, 150x100x70mm (opcional) | 1 | $5-10 USD |

**Total estimado:** $35-70 USD (dependiendo de la bomba y accesorios)

### Herramientas Necesarias

| Herramienta | Uso |
|-------------|-----|
| **Multímetro** | Verificar continuidad y voltajes |
| **Cautín + estaño** | Soldar conexiones permanentes |
| **Pelacables** | Preparar cables para conexión |
| **Destornillador pequeño** | Ajustar terminales del relé |
| **Cinta aislante / termoencogible** | Aislar conexiones |

---

## 2. Diagrama de Conexión General

```
                    ┌─────────────────────────────────────────────────┐
                    │              CAJA ESTANCA IP65                  │
                    │                                                 │
                    │  ┌─────────────────────────────────────────┐    │
                    │  │         PROTOBOARD / PCB                 │    │
                    │  │                                          │    │
                    │  │  ┌──────────────────────┐               │    │
                    │  │  │     ESP32-S3          │               │    │
                    │  │  │                       │               │    │
                    │  │  │  USB ◄──── 5V ────────┼───────────────┼───►┤
                    │  │  │  GND ◄──── GND ───────┼───────────────┼───►┤
                    │  │  │  GPIO4 ───────────────┼─────► IN Relé  │    │
                    │  │  │  GPIO2 ───────────────┼─────► LED      │    │
                    │  │  └──────────────────────┘               │    │
                    │  │                                          │    │
                    │  │  ┌──────────────────────┐               │    │
                    │  │  │   MÓDULO RELÉ 5V      │               │    │
                    │  │  │                       │               │    │
                    │  │  │  VCC ◄──── 5V ────────┼───────────────┼───►┤
                    │  │  │  GND ◄──── GND ───────┼───────────────┼───►┤
                    │  │  │  IN  ◄──── GPIO4      │               │    │
                    │  │  │                       │               │    │
                    │  │  │  COM ─────────────────┼──► 12V (+)    │    │
                    │  │  │  NO  ─────────────────┼──► Bomba (+)  │    │
                    │  │  │  NC  ── (sin usar)    │               │    │
                    │  │  └──────────────────────┘               │    │
                    │  │                                          │    │
                    │  │  ┌──────────────────────┐               │    │
                    │  │  │   LED + RESISTENCIA   │               │    │
                    │  │  │                       │               │    │
                    │  │  │  GPIO2 ──► R 220Ω ──►│ Ánodo (+)     │    │
                    │  │  │  GND ◄────────────────┤ Cátodo (-)    │    │
                    │  │  └──────────────────────┘               │    │
                    │  └─────────────────────────────────────────┘    │
                    └─────────────────────────────────────────────────┘
                                  │           │
                     ┌────────────┘           └────────────┐
                     ▼                                      ▼
              ┌───────────┐                         ┌──────────────┐
              │ Fuente 5V │                         │ Fuente 12V   │
              │   2A DC   │                         │   1A DC      │
              └───────────┘                         └──────────────┘
```

---

## 3. Conexión Paso a Paso

### Paso 1: Preparar la Protoboard

```
    ┌──────┬──────┬──────┬──────┬──────┬──────┐
    │ (+)  │      │      │      │      │      │  ← Riel positivo
    ├──────┼──────┼──────┼──────┼──────┼──────┤
    │      │      │      │      │      │      │
    │      │      │      │      │      │      │  ← Zona de componentes
    │      │      │      │      │      │      │
    ├──────┼──────┼──────┼──────┼──────┼──────┤
    │ (-)  │      │      │      │      │      │  ← Riel negativo (GND)
    └──────┴──────┴──────┴──────┴──────┴──────┘
```

Coloca la protoboard en una superficie estable y no conductora. Identifica los rieles positivo (+) y negativo (-).

### Paso 2: Colocar el ESP32-S3 en la Protoboard

```
    ┌──────────────────────────────────────┐
    │ USB-C                                │
    │  ┌────────────────────────────────┐  │
    │  │ 3V3   EN   VP  VN   D1   D2   │  │
    │  │                                │  │
    │  │ GND  D12  D13  D14  D15  D16  │  │
    │  │                                │  │
    │  │ D17  D18  D19  D20  D21  D22  │  │
    │  │                                │  │
    │  │ 5V   D4   D5   D6   D7   D8   │  │
    │  └────────────────────────────────┘  │
    └──────────────────────────────────────┘
```

- Coloca el ESP32 en la protoboard de modo que los pines queden en columnas separadas.
- Asegúrate de que la placa quede firme y alineada.

### Paso 3: Conectar la Alimentación (5V y GND)

```
Fuente 5V 2A
    │
    ├── (+) ──── Cable rojo ──── Riel (+) de protoboard
    │                          └── ESP32 5V pin
    │                          └── Relé VCC
    │
    └── (-) ──── Cable negro ──── Riel (-) de protoboard
                               └── ESP32 GND
                               └── Relé GND
                               └── LED cátodo (por resistencia)
```

**ADVERTENCIA:** Verifica la polaridad antes de conectar. Una inversión puede dañar los componentes.

### Paso 4: Conectar el Módulo Relé

```
    ESP32                    Módulo Relé 5V
    ─────                    ─────────────
    GPIO 4 ─────────────────► IN
    5V     ─────────────────► VCC
    GND    ─────────────────► GND
```

**Esquema del relé:**

```
    ┌─────────────────────┐
    │   MÓDULO RELÉ 5V    │
    │                     │
    │  ┌──┐ ┌──┐ ┌──┐   │
    │  │IN│ │VCC│ │GND│   │  ← Pines de control
    │  └──┘ └──┘ └──┘   │
    │                     │
    │  ┌──┐ ┌──┐ ┌──┐   │
    │  │NC│ │COM│ │NO│   │  ← Contactos (bornes tornillo)
    │  └──┘ └──┘ └──┘   │
    └─────────────────────┘

    NC → Normalmente Cerrado (conectado a COM cuando relé está apagado)
    COM → Común
    NO → Normalmente Abierto (conectado a COM cuando relé está activo)
```

Para la bomba de agua, conecta:
- **COM** → Positivo de la fuente de 12V
- **NO** → Positivo de la bomba
- Cuando el relé se activa, COM y NO se conectan, energizando la bomba.

### Paso 5: Conectar el LED Indicador (Simulación)

```
    ESP32                    LED + Resistencia
    ─────                    ────────────────
    GPIO 2 ──── R 220Ω ────► Ánodo (pata larga) del LED
    GND    ◄──────────────── Cátodo (pata corta) del LED
```

**Cálculo de la resistencia limitadora:**

```
V = 3.3V (GPIO)
V_LED = 2V (LED verde típico)
I = 10mA

R = (V - V_LED) / I = (3.3 - 2) / 0.01 = 130Ω

→ Usar 220Ω (valor comercial más cercano, seguro)
```

### Paso 6: Conectar la Bomba de Agua (Opcional)

```
    ┌─────────────────────────────────────┐
    │                                     │
    │   Fuente 12V DC                     │
    │                                     │
    │   (+) ──────── Relé COM ───────────┤
    │                       │             │
    │                       └── Relé NO ──┼── Bomba (+) ──┐
    │                                     │               │
    │   (-) ──────────────────────────────┼── Bomba (-) ──┤
    │                                     │               │
    └─────────────────────────────────────┘               │
                                                           │
    ┌─────────────────────────────────────────────────────┘
    │
    ▼  Cuando el relé se activa:
       COM conecta con NO → 12V llega a la bomba → Bomba funciona
```

### Paso 7: Conexión del Cable Serial (Depuración)

```
    ESP32                    PC / Laptop
    ─────                    ──────────
    USB-C ──── Cable USB ───► Puerto USB
```

El ESP32-S3 tiene un puerto USB-C que sirve tanto para **programación** como para **monitoreo Serial** a 115200 baud.

---

## 4. Esquema Eléctrico Completo (ASCII Art Detallado)

```
                                    ┌────── 5V ──────┐
                                    │                 │
                                    ▼                 ▼
                            ┌──────────────┐  ┌──────────────┐
                            │ ESP32-S3     │  │ Módulo Relé  │
                            │              │  │              │
    ┌── 5V ────────────────►│ 5V           │  │ VCC          │
    │             GND ─────►│ GND          │  │ GND          │
    │                       │              │  │              │
    │              ┌────────┤ GPIO 4 ──────┼──► IN           │
    │              │        │              │  │              │
    │              │  ┌─────┤ GPIO 2       │  │              │
    │              │  │     │              │  │              │
    │              │  │     └──────────────┘  └──────┬───────┘
    │              │  │                              │
    │              │  │    ┌────────┐                │
    │              │  └────┤ 220Ω   │                │
    │              │       └───┬────┘                │
    │              │           │                     │
    │              │       ┌───▼───┐                 │
    │              │       │  LED  │                 │
    │              │       │  (ánodo)                │
    │              │       └───┬───┘                 │
    │              │           │                     │
    │              └───────────┤                     │
    │                          │ GND                 │
    │                          ▼                     │
    │                                               │
    │         ┌─────────────────────────────────────┤
    │         │                                     │
    │         │   ┌─────── 12V (+) ◄── Fuente 12V   │
    │         │   │                                     │
    │         ▼   ▼                                     │
    │    ┌────────────┐                                 │
    │    │  BOMBA     │   (-) ◄──── GND 12V             │
    │    │  AGUA      │                                 │
    │    │  12V DC    │                                 │
    │    └────────────┘                                 │
    │                                                   │
    └───────────────────────────────────────────────────┘
```

---

## 5. Conexión Alternativa: Solo Simulación (LED en lugar de Bomba)

Para desarrollo, pruebas o el curso, puedes reemplazar la bomba por un LED:

```
ESP32 GPIO 4 ──── R 220Ω ────► LED Ánodo (+)
ESP32 GND       ◄────────────── LED Cátodo (-)
```

Cuando el sistema active el "riego", este LED se encenderá, simulando la bomba.

**Comparativa:**

| Componente real | Simulador | Mismo comportamiento |
|----------------|-----------|---------------------|
| Bomba de agua 12V | LED en GPIO 4 | Se enciende/apaga con el relé |
| Relé 5V | LED en GPIO 4 (directo) | El resto del firmware funciona igual |
| Sensor de flujo | (no necesario) | — |
| Fuente 12V | No necesaria | Solo 5V por USB |

---

## 6. Verificación de Conexiones

### Checklist de Verificación

| # | Verificación | Método | ¿OK? |
|---|-------------|--------|:----:|
| 1 | Polaridad 5V correcta | Multímetro: 5V entre VCC y GND del ESP32 | ⬜ |
| 2 | Polaridad 12V correcta (si aplica) | Multímetro: 12V entre COM y GND de bomba | ⬜ |
| 3 | Relé no conectado a 3.3V | Verificar VCC del relé va a 5V, no a 3.3V | ⬜ |
| 4 | LED con resistencia | Verificar resistencia de 220Ω en serie con LED | ⬜ |
| 5 | GPIO4 sin corto | Multímetro en modo continuidad: GPIO4-GND debe dar abierto | ⬜ |
| 6 | Bomba con diodo flyback | Diodo 1N4007 en paralelo inverso a bornes de bomba | ⬜ |
| 7 | Fuente 5V suficiente | Mínimo 2A para ESP32 + relé | ⬜ |
| 8 | Cables firmes | Tironcito suave a cada conexión | ⬜ |

### Prueba Rápida de Encendido

1. **Sin programar:** Alimenta solo el ESP32 por USB. El LED azul (GPIO2) debe encenderse si el firmware está cargado.
2. **Serial:** Abre el monitor Serial a 115200 baud. Debes ver los mensajes de inicio del sistema.
3. **WiFi:** Busca la red "Riego-Hortaliza" con tu teléfono. Debe aparecer.
4. **Relé:** Desde la web, activa riego manual por 1 minuto. Debes escuchar el "click" del relé.

---

## 7. Código de Colores de Cables (Recomendación)

| Color | Señal |
|-------|-------|
| 🔴 Rojo | Alimentación positiva (5V, 12V, 3.3V) |
| ⚫ Negro | Tierra (GND) |
| 🟡 Amarillo | Señal de control (GPIO) |
| 🟢 Verde | Datos / comunicación |
| 🔵 Azul | Señales auxiliares |

---

## 8. Protecciones Recomendadas

### Diodo Flyback para la Bomba

Las bombas de agua son cargas inductivas. Al desconectarlas, generan un pico de voltaje inverso que puede dañar el relé.

```
          Bomba 12V
        ┌──────────┐
        │          │
   (+)──┤   M1    ├──(-)
        │          │
        └──┬───────┘
           │
          ┌┴┐
          │ │ 1N4007 (diodo flyback)
          │ │
          └┬┘
           │
           └──────── GND
```

**Conexión:** Ánodo del diodo a GND, cátodo al positivo de la bomba.

### Fusible de Protección

```
Fuente 5V ──── [Fusible 2A] ──── ESP32 + Relé
Fuente 12V ─── [Fusible 1A] ──── Bomba
```

Un fusible de vidrio o PTC (rearmable) protege contra cortocircuitos.

---

## 9. Dimensiones y Distribución Física (Sugerida)

```
Vista superior de la caja estanca (150x100x70mm):

┌─────────────────────────────────────────────────────┐
│                                                     │
│  ┌──────────────────┐  ┌────────────────────┐       │
│  │   ESP32-S3       │  │   Fuente 5V 2A      │       │
│  │                  │  │   (adaptador USB)   │       │
│  │                  │  │                      │       │
│  └──────────────────┘  └────────────────────┘       │
│                                                     │
│  ┌──────────────────┐  ┌────────────────────┐       │
│  │   Módulo Relé     │  │   Borneras /        │       │
│  │   1 canal 5V      │  │   Conexiones        │       │
│  │                   │  │   externas          │       │
│  └──────────────────┘  └────────────────────┘       │
│                                                     │
│  ┌────────────────────────────────────────────┐     │
│  │    Prensaestopas para cables de salida      │     │
│  │    (bomba, sensores, alimentación)          │     │
│  └────────────────────────────────────────────┘     │
│                                                     │
└─────────────────────────────────────────────────────┘
```

**Recomendaciones de montaje:**
- Usa separadores de nylon para evitar cortocircuitos con la caja
- Perfora la caja solo donde sea necesario (prensaestopas)
- Deja espacio para ventilación pasiva
- Etiqueta cada cable en ambos extremos
