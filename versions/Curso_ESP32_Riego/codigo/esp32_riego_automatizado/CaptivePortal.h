#ifndef CAPTIVEPORTAL_H
#define CAPTIVEPORTAL_H

#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include "config.h"
#include "WebInterface.h"
#include "RelayControl.h"
#include "WeatherCheck.h"
#include "Storage.h"

// ============================================================
// Captive Portal - DNS Spoofing + Web Server
// ============================================================

extern SystemConfig config;      // Definida en el main
extern RelayControl relay;       // Definida en el main
extern String lastAction;        // Definida en el main
extern int lastScheduleIdx;      // Definida en el main
extern int lastScheduleDay;      // Definida en el main
extern bool ntpSynced;           // Definida en el main
extern struct tm* timeinfo;      // Definida en el main

/**
 * @brief Clase que implementa el portal cautivo con DNS spoofing
 *        y el servidor web de configuración.
 */
class CaptivePortal {
private:
  DNSServer _dnsServer;
  WebServer _server;
  const byte _dnsPort = 53;
  const byte _httpPort = 80;
  const IPAddress _apIP;

  /**
   * @brief Configura las rutas del servidor web.
   */
  void _setupRoutes() {
    // Página principal (Dashboard)
    _server.on("/", HTTP_GET, [this]() {
      String page = buildMainPage(config,
                                  relay.isActive(),
                                  relay.getRemainingSeconds(),
                                  lastAction);
      _server.send(200, "text/html", page);
    });

    // Página de configuración general
    _server.on("/config", HTTP_GET, [this]() {
      String page = buildConfigPage(config);
      _server.send(200, "text/html", page);
    });

    // Página de horarios de riego
    _server.on("/schedules", HTTP_GET, [this]() {
      String page = buildSchedulesPage(config);
      _server.send(200, "text/html", page);
    });

    // Guardar configuración general (POST)
    _server.on("/save-config", HTTP_POST, [this]() {
      String ssid = _server.arg("wifi_ssid");
      String pass = _server.arg("wifi_pass");
      String apiKey = [REDACTED]"owm_api_key");
      String latStr = _server.arg("latitude");
      String lonStr = _server.arg("longitude");

      // Copiar valores a la configuración
      ssid.toCharArray(config.wifiSSID, sizeof(config.wifiSSID));
      pass.toCharArray(config.wifiPass, sizeof(config.wifiPass));
      apiKey.toCharArray(config.owmApiKey, sizeof(config.owmApiKey));
      config.latitude = latStr.toFloat();
      config.longitude = lonStr.toFloat();

      saveConfig(config);

      String page = buildMainPage(config,
                                  relay.isActive(),
                                  relay.getRemainingSeconds(),
                                  "Configuración guardada. Reconectando WiFi...");
      _server.send(200, "text/html", page);

      // Intentar reconectar WiFi STA con nueva configuración
      if (strlen(config.wifiSSID) > 0) {
        WiFi.begin(config.wifiSSID, config.wifiPass);
      }
    });

    // Guardar horarios (POST)
    _server.on("/save-schedules", HTTP_POST, [this]() {
      for (int i = 0; i < MAX_SCHEDULES; i++) {
        String prefix = "s" + String(i);
        String hourStr = _server.arg(prefix + "_hour");
        String minStr = _server.arg(prefix + "_min");
        String durStr = _server.arg(prefix + "_dur");
        String enStr = _server.arg(prefix + "_en");

        config.schedules[i].hour = (uint8_t)hourStr.toInt();
        config.schedules[i].minute = (uint8_t)minStr.toInt();
        config.schedules[i].duration = (uint16_t)durStr.toInt();

        // Limitar duración máxima
        if (config.schedules[i].duration > MAX_IRRIGATION_MIN) {
          config.schedules[i].duration = MAX_IRRIGATION_MIN;
        }

        config.schedules[i].enabled = (enStr == "1" || enStr == "true" || enStr == "on");
      }

      saveConfig(config);

      String page = buildMainPage(config,
                                  relay.isActive(),
                                  relay.getRemainingSeconds(),
                                  "Horarios guardados correctamente.");
      _server.send(200, "text/html", page);
    });

    // Riego manual (POST)
    _server.on("/manual-irrigate", HTTP_POST, [this]() {
      String durStr = _server.arg("duration");
      uint16_t manualDuration = (durStr.length() > 0) ? (uint16_t)durStr.toInt() : 5;

      if (manualDuration < 1) manualDuration = 1;
      if (manualDuration > MAX_IRRIGATION_MIN) manualDuration = MAX_IRRIGATION_MIN;

      relay.activate(manualDuration);
      lastAction = "Riego manual iniciado - " + String(manualDuration) + " min";

      String page = buildMainPage(config,
                                  relay.isActive(),
                                  relay.getRemainingSeconds(),
                                  lastAction);
      _server.send(200, "text/html", page);
    });

    // Endpoint JSON de estado (para actualización AJAX)
    _server.on("/status", HTTP_GET, [this]() {
      // Consultar clima si hay API Key
      float rainProb = -1.0;
      String weatherDesc = "";
      if (strlen(config.owmApiKey) > 0 && WiFi.status() == WL_CONNECTED) {
        rainProb = getRainProbability(config.owmApiKey,
                                       config.latitude,
                                       config.longitude,
                                       &weatherDesc);
      }

      String json = buildStatusJSON(config,
                                    relay.isActive(),
                                    relay.getRemainingSeconds(),
                                    lastAction,
                                    rainProb,
                                    weatherDesc);
      _server.send(200, "application/json", json);
    });

    // Log de últimas acciones
    _server.on("/irrigation-log", HTTP_GET, [this]() {
      String json = "{\"lastAction\":\"" + lastAction + "\",";
      json += "\"lastScheduleIdx\":" + String(lastScheduleIdx) + ",";
      json += "\"lastScheduleDay\":" + String(lastScheduleDay) + "}";
      _server.send(200, "application/json", json);
    });

    // Redirigir cualquier otra ruta al dashboard
    _server.onNotFound([this]() {
      _server.sendHeader("Location", String("http://") + _apIP.toString(), true);
      _server.send(302, "text/plain", "");
    });
  }

public:
  /**
   * @brief Constructor del portal cautivo.
   */
  CaptivePortal()
    : _server(80)
    , _apIP(192, 168, 4, 1)
  {}

  /**
   * @brief Inicializa el AP WiFi, el DNS server y el web server.
   * 
   * Configura el ESP32 como Access Point con las credenciales definidas
   * en config.h, inicia el DNS spoofing para redirigir todo el tráfico
   * a la IP local, y arranca el servidor web con todas las rutas.
   */
  void begin() {
    Serial.println(F("\n============================================"));
    Serial.println(F("  Captive Portal - Riego Inteligente ESP32"));
    Serial.println(F("============================================"));

    // Iniciar Access Point
    WiFi.mode(WIFI_AP_STA);
    bool apStarted = WiFi.softAP(AP_SSID, AP_PASS);
    if (apStarted) {
      Serial.printf("[Captive] AP iniciado: SSID='%s', PASS='%s'\n", AP_SSID, AP_PASS);
      Serial.printf("[Captive] IP del AP: %s\n", WiFi.softAPIP().toString().c_str());
    } else {
      Serial.println(F("[Captive] ERROR: No se pudo iniciar el AP"));
    }

    // Iniciar DNS server (puerto 53)
    _dnsServer.start(_dnsPort, "*", _apIP);
    Serial.printf("[Captive] DNS Server iniciado en puerto %d\n", _dnsPort);
    Serial.printf("[Captive] Todo el tráfico DNS redirigido a %s\n", _apIP.toString().c_str());

    // Configurar rutas del servidor web
    _setupRoutes();

    // Iniciar servidor web
    _server.begin();
    Serial.println(F("[Captive] Servidor web HTTP iniciado en puerto 80"));
    Serial.println(F("============================================\n"));
  }

  /**
   * @brief Atiende las peticiones DNS y HTTP. Debe llamarse periódicamente.
   */
  void handleClient() {
    _dnsServer.processNextRequest();
    _server.handleClient();
  }
};

#endif // CAPTIVEPORTAL_H
