#ifndef WEATHERCHECK_H
#define WEATHERCHECK_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "config.h"

// ============================================================
// Consulta meteorológica a OpenWeatherMap OneCall API
// ============================================================

/**
 * @brief Realiza una consulta a la API OneCall de OpenWeatherMap para obtener
 *        la probabilidad de lluvia y la descripción del clima.
 * 
 * @param apiKey   API Key de OpenWeatherMap
 * @param lat      Latitud de la ubicación
 * @param lon      Longitud de la ubicación
 * @param weatherDesc (opcional) Puntero para devolver la descripción del clima
 * @return float   Probabilidad de precipitación (0.0 - 1.0), o -1.0 si hay error
 * 
 * @note En caso de error (sin conexión, timeout, parseo), retorna -1.0
 *       para que el sistema opere en modo seguro (regar por precaución).
 */
float getRainProbability(const char* apiKey, float lat, float lon, String* weatherDesc = nullptr) {
  // Validar que tengamos API key
  if (apiKey == nullptr || strlen(apiKey) == 0) {
    Serial.println(F("[Weather] Error: No hay API Key configurada."));
    return -1.0;
  }

  // Verificar que WiFi esté conectado a internet
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(F("[Weather] Error: WiFi no conectado a internet."));
    return -1.0;
  }

  HTTPClient http;
  String url = String("http://") + OWM_HOST +
               "/data/2.5/onecall?lat=" + String(lat, 6) +
               "&lon=" + String(lon, 6) +
               "&exclude=current,minutely,daily&appid=" + String(apiKey);

  Serial.printf("[Weather] Consultando: %s\n", url.c_str());

  http.begin(url);
  http.setTimeout(5000); // 5 segundos de timeout

  int httpCode = http.GET();
  float rainProb = -1.0;

  if (httpCode > 0) {
    Serial.printf("[Weather] HTTP Response code: %d\n", httpCode);

    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();

      // Parsear JSON con ArduinoJson v6
      DynamicJsonDocument doc(16384); // Buffer generoso para la respuesta
      DeserializationError error = deserializeJson(doc, payload);

      if (error) {
        Serial.printf("[Weather] Error parseando JSON: %s\n", error.c_str());
        rainProb = -1.0;
      } else {
        // Extraer probabilidad de precipitación del primer bloque horario
        // hourly[0].pop → probabilidad 0.0 - 1.0
        JsonArray hourly = doc["hourly"];
        if (!hourly.isNull() && hourly.size() > 0) {
          JsonObject hour0 = hourly[0];
          rainProb = hour0["pop"] | -1.0;
          Serial.printf("[Weather] Probabilidad de lluvia: %.2f (%.0f%%)\n",
                        rainProb, rainProb * 100.0);

          // Extraer descripción del clima si se solicitó
          if (weatherDesc != nullptr) {
            JsonArray weather = hour0["weather"];
            if (!weather.isNull() && weather.size() > 0) {
              *weatherDesc = weather[0]["description"].as<String>();
            } else {
              *weatherDesc = "No disponible";
            }
          }
        } else {
          Serial.println(F("[Weather] No se encontraron datos horarios en la respuesta."));
          rainProb = -1.0;
        }
      }
    } else {
      Serial.printf("[Weather] Error HTTP: %d\n", httpCode);
      rainProb = -1.0;
    }
  } else {
    Serial.printf("[Weather] FallO en la conexiOn HTTP: %s\n", http.errorToString(httpCode).c_str());
    rainProb = -1.0;
  }

  http.end();
  return rainProb;
}

#endif // WEATHERCHECK_H
