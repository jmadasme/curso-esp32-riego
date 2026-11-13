#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <stdint.h>

// ============================================================
// Constantes globales del sistema
// ============================================================

#define EEPROM_SIZE         512       // Tamaño de la EEPROM simulada
#define MAX_SCHEDULES       4         // Máximo número de horarios de riego
#define RELAY_PIN           4         // GPIO por defecto para el relé
#define LED_PIN             2         // GPIO por defecto para LED indicador
#define MAX_IRRIGATION_MIN  120       // Duración máxima de riego en minutos (seguridad)
#define RAIN_THRESHOLD      90        // Porcentaje de lluvia para saltar riego
#define AP_SSID             "Riego-Hortaliza"
#define AP_PASS             "12345678"
#define NTP_SERVER          "pool.ntp.org"
#define OWM_HOST            "api.openweathermap.org"
#define WIFI_TIMEOUT_MS     10000     // Timeout para conexión WiFi STA
#define NTP_TIMEOUT_S       30        // Timeout para sincronización NTP

// ============================================================
// Estructuras de datos
// ============================================================

/**
 * @brief Representa un horario de riego programado
 */
struct Schedule {
  uint8_t hour;        // Hora (0-23)
  uint8_t minute;      // Minuto (0-59)
  uint16_t duration;   // Duración en minutos (1 - MAX_IRRIGATION_MIN)
  bool enabled;        // true = habilitado, false = deshabilitado
};

/**
 * @brief Configuración completa del sistema almacenada en EEPROM
 */
struct SystemConfig {
  char wifiSSID[32];                // SSID de la red WiFi cliente
  char wifiPass[64];                // Contraseña de la red WiFi cliente
  char owmApiKey[48];               // API Key de OpenWeatherMap
  float latitude;                   // Latitud del cultivo
  float longitude;                  // Longitud del cultivo
  Schedule schedules[MAX_SCHEDULES]; // Horarios de riego (máx. 4)
  uint8_t relayPin;                 // GPIO asignado al relé
  uint16_t checksum;                // Checksum de integridad
};

#endif // CONFIG_H
