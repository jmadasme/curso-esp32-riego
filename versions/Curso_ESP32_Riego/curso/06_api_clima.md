# Capítulo 6: Integración con API de Clima

**Duración:** 1.5 horas
**Objetivo:** Conectar el ESP32 a OpenWeatherMap para consultar la probabilidad de lluvia y decidir si regar

---

## 🎯 Objetivos del Capítulo

- Entender qué es una API REST y cómo funciona
- Realizar peticiones HTTP desde el ESP32
- Parsear respuestas JSON con ArduinoJson v6
- Extraer probabilidad de precipitación
- Implementar la lógica de decisión anti-lluvia

---

## 📖 6.1 Teoría: APIs REST y JSON

### ¿Qué es una API REST?

API (Application Programming Interface) es un conjunto de reglas que permite que dos programas se comuniquen. REST es un estilo de arquitectura muy usado en web.

```
ESP32 ─── GET http://api.openweathermap.org/... ───▶ Servidor
                                                       │
ESP32 ◀─── { "hourly": [ { "pop": 0.3 } ] } ────────┘
```

### Formato JSON

JSON (JavaScript Object Notation) es un formato ligero para intercambiar datos.

```json
{
  "hourly": [
    {
      "dt": 1700000000,
      "pop": 0.35,
      "weather": [
        {
          "description": "nubes dispersas"
        }
      ]
    }
  ]
}
```

**Elementos clave:**
- `{}` → Objeto
- `[]` → Arreglo
- `"clave": "valor"` → Propiedad
- `0.35` → Número (probabilidad 35%)

---

## 💻 6.2 Código: Cliente HTTP para OpenWeatherMap

### Configuración

1. Regístrate gratis en [OpenWeatherMap](https://openweathermap.org/api)
2. Ve a "API Keys" y copia tu clave
3. La API gratuita permite 60 llamadas/minuto

### Código Completo

```cpp
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

float getRainProbability(const char* apiKey, float lat, float lon) {
  if (strlen(apiKey) == 0) return -1.0;
  if (WiFi.status() != WL_CONNECTED) return -1.0;

  HTTPClient http;
  
  // Construir URL
  String url = "http://api.openweathermap.org/data/2.5/onecall?";
  url += "lat=" + String(lat, 6);
  url += "&lon=" + String(lon, 6);
  url += "&exclude=current,minutely,daily";
  url += "&appid=" + String(apiKey);

  Serial.println("Consultando: " + url);

  http.begin(url);
  http.setTimeout(5000); // 5 segundos de timeout

  int httpCode = http.GET();
  float rainProb = -1.0;

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    
    // Parsear JSON
    DynamicJsonDocument doc(16384);
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      // Obtener probabilidad de lluvia (0.0 a 1.0)
      rainProb = doc["hourly"][0]["pop"] | -1.0;
      
      // Obtener descripción del clima
      String weatherDesc = doc["hourly"][0]["weather"][0]["description"] | "";
      
      Serial.printf("Probabilidad de lluvia: %.0f%%\n", rainProb * 100);
      Serial.println("Clima: " + weatherDesc);
    } else {
      Serial.println("Error parseando JSON");
    }
  } else {
    Serial.printf("Error HTTP: %d\n", httpCode);
  }

  http.end();
  return rainProb;
}
```

### Desglose del Código

| Parte | Explicación |
|-------|-------------|
| `HTTPClient http` | Crea el cliente HTTP |
| `http.begin(url)` | Inicia la conexión |
| `http.setTimeout(5000)` | Timeout de 5 segundos |
| `int httpCode = http.GET()` | Ejecuta la petición GET |
| `http.getString()` | Obtiene el cuerpo de la respuesta |
| `deserializeJson(doc, payload)` | Parsea el JSON |
| `doc["hourly"][0]["pop"]` | Accede al campo de probabilidad |
| `http.end()` | Cierra la conexión |

---

## 🌧️ 6.3 Lógica de Decisión Anti-Lluvia

### El Algoritmo

```
¿Hay horario que ejecutar?
         │
         ▼
¿Tenemos API Key y WiFi?
   │            │
   Sí           No
   │            │
   ▼            ▼
Consultar API    │
   │            │
   ▼            ▼
¿Prob < 90%?   REGAR (modo seguro)
   │            │
   Sí    No     │
   │     │      │
   ▼     ▼      │
 REGAR  SALTAR  │
   │     │      │
   └──┬──┘      │
      └────┬────┘
           ▼
    ACTUALIZAR LOG
```

### Implementación

```cpp
// En el loop principal
if (idx >= 0) { // Hay horario para ejecutar
  bool shouldWater = true;

  if (strlen(config.owmApiKey) > 0 && WiFi.status() == WL_CONNECTED) {
    float rainProb = getRainProbability(config.owmApiKey, 
                                         config.latitude, 
                                         config.longitude);

    if (rainProb >= 0.0) {
      if (rainProb < (RAIN_THRESHOLD / 100.0f)) {
        shouldWater = true; // Regar (poca lluvia)
        lastAction = "Riego iniciado (Lluvia: " + 
                     String((int)(rainProb * 100)) + "%)";
      } else {
        shouldWater = false; // No regar (mucha lluvia)
        lastAction = "Riego saltado por lluvia (" + 
                     String((int)(rainProb * 100)) + "%)";
      }
    } else {
      // API falló: regar por precaución
      shouldWater = true;
      lastAction = "Riego por precaución (API falló)";
    }
  } else {
    // Sin API: regar directamente
    shouldWater = true;
    lastAction = "Riego sin verificación climática";
  }

  if (shouldWater) {
    relay.activate(config.schedules[idx].duration);
  }
}
```

---

## 🔬 6.4 Laboratorio 6: Ver Pronóstico en la Web

### Objetivo
Mostrar en el dashboard el pronóstico del tiempo y la probabilidad de lluvia obtenida de la API.

### Modificaciones en la Página Principal

```html
<!-- Tarjeta de clima en el dashboard -->
<div class="card">
  <h2>☁️ Clima Actual</h2>
  <div class="weather-info">
    <span class="weather-badge" id="weather-desc">
      Consultando...
    </span>
    <span class="weather-badge" id="rain-prob">
      🌧️ --%
    </span>
  </div>
</div>
```

### JavaScript para Actualizar Clima

```javascript
function updateWeather() {
  fetch('/status')
    .then(r => r.json())
    .then(data => {
      if (data.weatherDesc) {
        document.getElementById('weather-desc').textContent = data.weatherDesc;
      }
      if (data.rainProb >= 0) {
        const prob = (data.rainProb * 100).toFixed(0);
        document.getElementById('rain-prob').textContent = '🌧️ ' + prob + '%';
        
        // Cambiar color según probabilidad
        const el = document.getElementById('rain-prob');
        if (data.rainProb < 0.3) el.style.background = '#e8f5e9';
        else if (data.rainProb < 0.7) el.style.background = '#fff3e0';
        else el.style.background = '#ffebee';
      }
    });
}

// Actualizar cada 5 minutos (el clima cambia lento)
setInterval(updateWeather, 300000);
updateWeather();
```

---

## ✏️ 6.5 Ejercicio: Probar Diferentes Coordenadas y Umbrales

### Parte 1: Prueba con Diferentes Ubicaciones

Configura estas coordenadas y observa cómo cambia la probabilidad:

| Ubicación | Latitud | Longitud | Clima esperado |
|-----------|---------|----------|----------------|
| Santiago, Chile | -33.45 | -70.66 | Variable |
| Londres, UK | 51.51 | -0.13 | Lluvioso |
| Desierto de Atacama | -24.50 | -69.25 | Seco |
| Amazonas, Brasil | -3.47 | -62.58 | Lluvioso |

### Parte 2: Modificar el Umbral

Temporalmente cambia `RAIN_THRESHOLD` a diferentes valores:

```cpp
#define RAIN_THRESHOLD 50  // No regar si hay más de 50% de lluvia
```

### Preguntas para Reflexionar

1. ¿Qué umbral es más adecuado para un cultivo de tomates?
2. ¿En qué zonas el sistema regaría casi siempre?
3. ¿Qué pasaría si el umbral es 0%? (nunca regar)
4. ¿Y si es 100%? (regar siempre)

---

## ✅ Resumen del Capítulo

| Concepto | Implementación |
|----------|---------------|
| ✅ API REST | Petición HTTP GET a OpenWeatherMap |
| ✅ HTTP Client | `HTTPClient` con timeout de 5s |
| ✅ Parseo JSON | `ArduinoJson` v6, `deserializeJson()` |
| ✅ Probabilidad lluvia | `doc["hourly"][0]["pop"]` |
| ✅ Modo seguro | Si falla API → regar por precaución |
| ✅ Lógica anti-lluvia | `rainProb < RAIN_THRESHOLD` |
| ✅ Descripción clima | `weather[0].description` |

### Consideraciones

- La API gratuita tiene límite de 60 llamadas/minuto
- Usamos timeout de 5s para no bloquear el sistema
- Si la API falla, regamos por precaución (modo seguro)
- El clima se consulta solo cuando hay un horario a punto de ejecutarse

➡️ **Siguiente capítulo:** [07 - Control de Relé y Bomba de Agua](07_rele_bomba.md)
