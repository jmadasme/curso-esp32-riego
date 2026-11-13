// ============================================================
//  Riego Inteligente ESP32-S3
//  Sistema Autónomo de Riego Agrícola con Verificación Climática
// ============================================================
//
//  Características:
//  - Hotspot WiFi con captive portal
//  - Interfaz web responsive para configuración
//  - Programador de riego con 4 horarios diarios
//  - Persistencia en EEPROM de toda la configuración
//  - Verificación climática vía OpenWeatherMap API
//  - Control de relé para bomba de agua
//  - Sincronización NTP
//
//  Framework: Arduino (ESP32-S3)
//  Licencia: MIT
// ============================================================

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include "config.h"
#include "Storage.h"
#include "RelayControl.h"
#include "Scheduler.h"
#include "WeatherCheck.h"
#include "CaptivePortal.h"

// ============================================================
// Variables globales
// ============================================================

SystemConfig config;              // Configuración del sistema
RelayControl relay;               // Control del relé
CaptivePortal captivePortal;      // Portal cautivo + servidor web

time_t now;                       // Timestamp actual
struct tm* timeinfo = nullptr;    // Estructura de tiempo actual

bool configLoaded = false;        // Indica si la configuración se cargó OK
bool ntpSynced = false;           // Indica si NTP está sincronizado

// Variables para control de ejecución de horarios
String lastAction = "Ninguna";    // Última acción registrada
int lastScheduleIdx = -1;         // Índice del último horario ejecutado
int lastScheduleDay = -1;         // Día del año del último horario ejecutado

// Control de reintentos WiFi STA
unsigned long wifiReconnectTimer = 0;
const unsigned long WIFI_RECONNECT_INTERVAL = 30000; // Cada 30 segundos

// ============================================================
// setup() - Inicialización del sistema
// ============================================================

void setup() {
  // 1. Inicializar Serial
  Serial.begin(115200);
  Serial.println(F("\n\n============================================"));
  Serial.println(F("  Riego Inteligente ESP32-S3 v1.0"));
  Serial.println(F("============================================"));

  // 2. Inicializar EEPROM
  EEPROM.begin(EEPROM_SIZE);
  Serial.printf("[Main] EEPROM iniciada (%d bytes)\n", EEPROM_SIZE);

  // 3. Cargar configuración desde EEPROM
  configLoaded = loadConfig(config);
  if (!configLoaded) {
    Serial.println(F("[Main] No se encontró configuración válida. Usando valores por defecto."));
    // Inicializar con valores por defecto
    memset(&config, 0, sizeof(SystemConfig));
    config.relayPin = RELAY_PIN;
    config.latitude = 0.0;
    config.longitude = 0.0;
    for (int i = 0; i < MAX_SCHEDULES; i++) {
      config.schedules[i].hour = 8 + i * 4; // 8:00, 12:00, 16:00, 20:00
      config.schedules[i].minute = 0;
      config.schedules[i].duration = 10;
      config.schedules[i].enabled = false;
    }
    // Guardar configuración por defecto
    saveConfig(config);
    configLoaded = true;
  } else {
    Serial.println(F("[Main] Configuración cargada exitosamente."));
    Serial.printf("[Main] WiFi SSID: %s\n", config.wifiSSID);
    Serial.printf("[Main] Lat/Lon: %.4f, %.4f\n", config.latitude, config.longitude);
  }

  // 4. Inicializar el portal cautivo (siempre inicia el AP)
  captivePortal.begin();

  // 5. Conectar a WiFi STA si hay configuración
  if (strlen(config.wifiSSID) > 0) {
    Serial.printf("[Main] Conectando a WiFi STA: %s ...\n", config.wifiSSID);
    WiFi.begin(config.wifiSSID, config.wifiPass);

    unsigned long startAttempt = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - startAttempt) < WIFI_TIMEOUT_MS) {
      delay(100);
      Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
      Serial.printf("[Main] WiFi STA conectado. IP: %s\n", WiFi.localIP().toString().c_str());
    } else {
      Serial.println(F("[Main] No se pudo conectar a WiFi STA. El sistema funciona en modo AP solamente."));
      Serial.println(F("[Main] La verificación climática no estará disponible hasta configurar WiFi."));
    }
  } else {
    Serial.println(F("[Main] No hay configuración WiFi STA. Modo solo AP."));
  }

  // 6. Inicializar control de relé
  relay.begin(config.relayPin);

  // 7. Sincronizar NTP si hay conexión a internet
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(F("[Main] Sincronizando hora vía NTP..."));
    configTime(-3 * 3600, 0, NTP_SERVER);  // UTC-3 por defecto (ajustable)

    // Esperar sincronización NTP
    time_t nowNtp = 0;
    int ntpAttempts = 0;
    while (nowNtp < 100000 && ntpAttempts < NTP_TIMEOUT_S) {
      delay(1000);
      nowNtp = time(nullptr);
      ntpAttempts++;
      Serial.print(".");
    }
    Serial.println();

    if (nowNtp > 100000) {
      ntpSynced = true;
      struct tm *ntpTime = localtime(&nowNtp);
      Serial.printf("[Main] Hora NTP sincronizada: %02d:%02d:%02d %02d/%02d/%04d\n",
                    ntpTime->tm_hour, ntpTime->tm_min, ntpTime->tm_sec,
                    ntpTime->tm_mday, ntpTime->tm_mon + 1, ntpTime->tm_year + 1900);
    } else {
      Serial.println(F("[Main] No se pudo sincronizar NTP. El sistema funcionará sin hora exacta."));
      ntpSynced = false;
    }
  } else {
    Serial.println(F("[Main] Sin WiFi STA. NTP no disponible."));
    ntpSynced = false;
  }

  // Configurar timezone para Chile/América (UTC-3, sin horario de verano por simplicidad)
  // En producción, usar setenv() con TZ adecuado
  setenv("TZ", "CLT", 1);  // Chile Standard Time (UTC-3)
  tzset();

  Serial.println(F("============================================"));
  Serial.println(F("[Main] Sistema listo. Conéctate al WiFi 'Riego-Hortaliza'"));
  Serial.println(F("[Main] y abre http://192.168.4.1 en tu navegador."));
  Serial.println(F("============================================\n"));
}

// ============================================================
// loop() - Bucle principal del sistema
// ============================================================

void loop() {
  // 1. Atender el portal cautivo (DNS + HTTP)
  captivePortal.handleClient();

  // 2. Reintentar conexión WiFi STA periódicamente si está configurada pero desconectada
  if (strlen(config.wifiSSID) > 0 && WiFi.status() != WL_CONNECTED) {
    unsigned long nowMs = millis();
    if (nowMs - wifiReconnectTimer >= WIFI_RECONNECT_INTERVAL) {
      wifiReconnectTimer = nowMs;
      Serial.printf("[Main] Reintentando conexión WiFi a %s...\n", config.wifiSSID);
      WiFi.begin(config.wifiSSID, config.wifiPass);
    }
  }

  // 3. Verificar horarios de riego (solo si NTP está sincronizado y hay configuración)
  if (ntpSynced && configLoaded) {
    time(&now);
    timeinfo = localtime(&now);

    // Verificar que la hora es válida (tm_year > 2024)
    if (timeinfo != nullptr && timeinfo->tm_year > (2024 - 1900)) {
      int idx = checkSchedules(config, timeinfo);

      if (idx >= 0) {
        // Verificar que no sea el mismo horario ya ejecutado hoy
        bool alreadyExecuted = (idx == lastScheduleIdx && timeinfo->tm_yday == lastScheduleDay);

        if (!alreadyExecuted) {
          // Consultar probabilidad de lluvia
          bool shouldWater = true; // Por defecto, regar

          if (strlen(config.owmApiKey) > 0 && WiFi.status() == WL_CONNECTED) {
            String weatherDesc;
            float rainProb = getRainProbability(config.owmApiKey,
                                                  config.latitude,
                                                  config.longitude,
                                                  &weatherDesc);

            if (rainProb >= 0.0) {
              // Hay datos de clima: regar solo si prob < umbral
              if (rainProb < (RAIN_THRESHOLD / 100.0f)) {
                shouldWater = true;
                lastAction = "Riego iniciado - Programación #" + String(idx + 1) +
                             " (Lluvia: " + String((int)(rainProb * 100)) + "%)";
                Serial.printf("[Main] %s\n", lastAction.c_str());
              } else {
                shouldWater = false;
                lastAction = "Riego SALTADO - Programación #" + String(idx + 1) +
                             " (Lluvia: " + String((int)(rainProb * 100)) + "%)";
                Serial.printf("[Main] %s\n", lastAction.c_str());
              }
            } else {
              // Error en API: modo seguro, regar
              shouldWater = true;
              lastAction = "Riego por precaución - Programación #" + String(idx + 1) +
                           " (API clima falló)";
              Serial.printf("[Main] %s\n", lastAction.c_str());
            }
          } else {
            // Sin API key o sin internet: regar directamente
            shouldWater = true;
            lastAction = "Riego iniciado - Programación #" + String(idx + 1) +
                         " (sin verificación climática)";
            Serial.printf("[Main] %s\n", lastAction.c_str());
          }

          if (shouldWater && !relay.isActive()) {
            relay.activate(config.schedules[idx].duration);
          }

          // Registrar que ya se ejecutó este horario hoy
          lastScheduleIdx = idx;
          lastScheduleDay = timeinfo->tm_yday;
        }
      }
    }
  }

  // 4. Actualizar el estado del relé (timers no bloqueantes)
  relay.update();

  // 5. Pequeña pausa para no saturar la CPU
  delay(100);
}
