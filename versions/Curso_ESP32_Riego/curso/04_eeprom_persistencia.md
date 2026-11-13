# Capítulo 4: EEPROM y Persistencia de Datos

**Duración:** 1 hora
**Objetivo:** Aprender a guardar y recuperar datos permanentemente en el ESP32

---

## 🎯 Objetivos del Capítulo

- Comprender la diferencia entre RAM, flash y EEPROM
- Usar la librería EEPROM para leer/escribir datos
- Guardar estructuras complejas (como horarios de riego)
- Implementar verificación de integridad con checksum
- Asegurar que los datos persistan tras reinicios

---

## 📖 4.1 Teoría: Memorias en el ESP32

### Tipos de Memoria

| Tipo | Características | Uso en ESP32 |
|------|----------------|--------------|
| **RAM** | Volátil, rápida, se borra al apagar | Variables en ejecución |
| **Flash** | No volátil, lenta, 4-16 MB | Programa almacenado |
| **EEPROM (simulada)** | No volátil, sector dedicado en flash | Configuración persistente |

### EEPROM en ESP32

El ESP32 **no tiene EEPROM real**. Utiliza un sector de la memoria flash para simularla mediante la librería `EEPROM.h`.

**Características:**
- Tamaño configurable (512 bytes en nuestro proyecto)
- ~100,000 ciclos de escritura por sector
- Lectura ilimitada
- Se escribe por bytes, pero se guarda con `commit()`

### Ciclo de Vida de Escritura EEPROM

```
EEPROM.begin(tamaño)  →  EEPROM.write(dir, valor)  →  EEPROM.commit()  →  EEPROM.end()
     │                          │                          │                    │
 Inicializa                 Escribe byte               Confirma en         Libera recursos
 el sistema                 en memoria                  memoria flash
```

---

## 💻 4.2 Código: Guardar y Leer Datos Básicos

### Escribir y Leer Bytes

```cpp
#include <EEPROM.h>

#define EEPROM_SIZE 64

void setup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);

  // Escribir un byte en la dirección 0
  EEPROM.write(0, 42);        // Escribir el valor 42
  EEPROM.commit();            // ¡IMPORTANTE! Guardar en flash

  // Leer el byte de la dirección 0
  int valor = EEPROM.read(0);
  Serial.print("Valor recuperado: ");
  Serial.println(valor);

  EEPROM.end();
}

void loop() {}
```

### Guardar Strings y Números

```cpp
void saveString(int address, String data, int maxLen) {
  for (int i = 0; i < maxLen; i++) {
    if (i < data.length()) {
      EEPROM.write(address + i, data[i]);
    } else {
      EEPROM.write(address + i, 0); // Rellenar con nulos
    }
  }
}

String loadString(int address, int maxLen) {
  String result = "";
  for (int i = 0; i < maxLen; i++) {
    char c = EEPROM.read(address + i);
    if (c == 0) break;
    result += c;
  }
  return result;
}
```

---

## 📦 4.3 Estructuras de Datos y Checksum

### Guardar Estructuras Completas

La forma más eficiente de guardar configuración es usar estructuras:

```cpp
struct WiFiConfig {
  char ssid[32];
  char password[64];
  uint16_t checksum;
};

void saveConfig(WiFiConfig &cfg) {
  EEPROM.begin(512);
  
  // Calcular checksum
  uint16_t sum = 0;
  uint8_t* bytes = (uint8_t*)&cfg;
  for (size_t i = 0; i < sizeof(WiFiConfig) - 2; i++) {
    sum += bytes[i];
  }
  cfg.checksum = sum;
  
  // Escribir toda la estructura
  for (size_t i = 0; i < sizeof(WiFiConfig); i++) {
    EEPROM.write(i, bytes[i]);
  }
  
  EEPROM.commit();
  EEPROM.end();
}

bool loadConfig(WiFiConfig &cfg) {
  EEPROM.begin(512);
  
  // Leer toda la estructura
  uint8_t* bytes = (uint8_t*)&cfg;
  for (size_t i = 0; i < sizeof(WiFiConfig); i++) {
    bytes[i] = EEPROM.read(i);
  }
  EEPROM.end();
  
  // Verificar checksum
  uint16_t savedSum = cfg.checksum;
  cfg.checksum = 0;
  
  uint16_t calcSum = 0;
  for (size_t i = 0; i < sizeof(WiFiConfig) - 2; i++) {
    calcSum += bytes[i];
  }
  
  cfg.checksum = savedSum;
  return (savedSum == calcSum);
}
```

### ¿Por qué Checksum?

```
Sin checksum:
  [SSID][PASS][???]  ← Si se corrompe, no lo sabremos

Con checksum:
  [SSID][PASS][CHECKSUM]  ← Verificamos integridad al cargar
  Si CHECKSUM no coincide → usamos valores por defecto
```

---

## 🔬 4.4 Laboratorio 4: Guardar y Cargar Configuración

### Objetivo
Crear un programa que guarde un nombre y un número en EEPROM, los recupere al reiniciar, y los muestre por Serial.

### Código Base del Laboratorio

```cpp
#include <EEPROM.h>
#include <string.h>

#define EEPROM_SIZE 128

struct MiConfig {
  char nombre[20];
  int contador;
  uint16_t checksum;
};

MiConfig config;

uint16_t calcChecksum(MiConfig &cfg) {
  uint16_t sum = 0;
  uint8_t* bytes = (uint8_t*)&cfg;
  for (size_t i = 0; i < sizeof(MiConfig) - 2; i++) {
    sum += bytes[i];
  }
  return sum;
}

void guardar() {
  EEPROM.begin(EEPROM_SIZE);
  config.checksum = calcChecksum(config);
  uint8_t* bytes = (uint8_t*)&config;
  for (size_t i = 0; i; i++) {
    EEPROM.write(i, bytes[i]);
  }
  EEPROM.commit();
  EEPROM.end();
  Serial.println("Config guardada!");
}

void cargar() {
  EEPROM.begin(EEPROM_SIZE);
  uint8_t* bytes = (uint8_t*)&config;
  for (size_t i = 0; i < sizeof(MiConfig); i++) {
    bytes[i] = EEPROM.read(i);
  }
  EEPROM.end();

  uint16_t savedCk = config.checksum;
  config.checksum = 0;
  if (savedCk == calcChecksum(config)) {
    config.checksum = savedCk;
    Serial.println("Config cargada OK");
  } else {
    Serial.println("Config corrupta, usando defaults");
    strcpy(config.nombre, "Usuario");
    config.contador = 0;
  }
}

void setup() {
  Serial.begin(115200);
  cargar();
  
  config.contador++;
  Serial.print("Hola ");
  Serial.print(config.nombre);
  Serial.print(", reinicio #");
  Serial.println(config.contador);
  
  guardar();
}

void loop() {}
```

### Prueba
1. Sube el código al ESP32
2. Abre el Monitor Serial → verás "Hola Usuario, reinicio #1"
3. Presiona el botón RESET del ESP32
4. Verás "Hola Usuario, reinicio #2"
5. ¡El contador persiste!

---

## ✏️ 4.5 Ejercicio: Contador Persistente de Reinicios

Crea un programa que:
1. Almacene un contador de reinicios en EEPROM
2. Cada vez que se encienda el ESP32, incremente el contador
3. Muestre un mensaje como: "Encendido #42 - ¡Bienvenido!"
4. Si se presiona un botón en GPIO 0, resetee el contador a 0

### Pistas
- Usa `digitalRead(0)` para leer el botón BOOT del ESP32
- El botón BOOT está en GPIO 0 y hace LOW al presionarlo
- Lee el contador al inicio de `setup()`, increméntalo, guárdalo

---

## ✅ Resumen del Capítulo

| Concepto | Código Clave |
|----------|-------------|
| ✅ Inicializar EEPROM | `EEPROM.begin(tamaño)` |
| ✅ Escribir byte | `EEPROM.write(dirección, valor)` |
| ✅ Leer byte | `EEPROM.read(dirección)` |
| ✅ Confirmar escritura | `EEPROM.commit()` |
| ✅ Guardar estructuras | Cast a `uint8_t*` y escribir byte a byte |
| ✅ Checksum | Suma de todos los bytes excepto el checksum |
| ✅ Verificar integridad | Calcular checksum al cargar y comparar |

### En el Proyecto Final

Usaremos EEPROM para guardar:
- 📡 SSID y contraseña WiFi
- 🔑 API Key de OpenWeatherMap
- 📍 Coordenadas GPS del cultivo
- ⏰ 4 horarios de riego
- 🔌 Pin del relé
- ✅ Checksum de integridad

➡️ **Siguiente capítulo:** [05 - Programador de Riego](05_programador_riego.md)
