#ifndef WEBINTERFACE_H
#define WEBINTERFACE_H

#include <Arduino.h>
#include "config.h"

// ============================================================
// Interfaz Web - HTML + CSS + JS embebido en PROGMEM
// ============================================================

// ---------- CSS COMPARTIDO (PROGMEM) ----------
static const char STYLE_CSS[] PROGMEM = R"rawliteral(
* {
  margin: 0;
  padding: 0;
  box-sizing: border-box;
}

body {
  font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
  background: linear-gradient(135deg, #f0f7f0 0%, #e0f0e0 100%);
  color: #2d3a2d;
  min-height: 100vh;
  padding: 20px;
}

.container {
  max-width: 960px;
  margin: 0 auto;
}

.header {
  background: linear-gradient(135deg, #2e7d32 0%, #1b5e20 100%);
  color: white;
  padding: 24px 32px;
  border-radius: 16px;
  margin-bottom: 24px;
  box-shadow: 0 4px 20px rgba(46,125,50,0.3);
  text-align: center;
}

.header h1 {
  font-size: 1.8em;
  margin-bottom: 4px;
}

.header p {
  opacity: 0.9;
  font-size: 0.95em;
}

.card {
  background: white;
  border-radius: 12px;
  padding: 24px;
  margin-bottom: 20px;
  box-shadow: 0 2px 12px rgba(0,0,0,0.08);
  border-left: 4px solid #4caf50;
}

.card h2 {
  color: #2e7d32;
  font-size: 1.3em;
  margin-bottom: 16px;
  padding-bottom: 8px;
  border-bottom: 2px solid #e8f5e9;
}

.card h3 {
  color: #388e3c;
  font-size: 1.1em;
  margin-bottom: 12px;
}

.status-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
  gap: 16px;
}

.status-item {
  background: #f9fdf9;
  border: 1px solid #e0f0e0;
  border-radius: 10px;
  padding: 16px;
  text-align: center;
}

.status-item .label {
  font-size: 0.85em;
  color: #558b2f;
  margin-bottom: 6px;
  text-transform: uppercase;
  letter-spacing: 0.5px;
}

.status-item .value {
  font-size: 1.5em;
  font-weight: 700;
  color: #1b5e20;
}

.status-item .value.active {
  color: #f44336;
}

.status-item .value.inactive {
  color: #66bb6a;
}

.status-item .value.manual {
  color: #ff8f00;
}

.form-group {
  margin-bottom: 16px;
}

.form-group label {
  display: block;
  font-weight: 600;
  color: #2e7d32;
  margin-bottom: 6px;
  font-size: 0.95em;
}

.form-group input,
.form-group select {
  width: 100%;
  padding: 10px 14px;
  border: 2px solid #c8e6c9;
  border-radius: 8px;
  font-size: 1em;
  transition: border-color 0.3s;
  background: #fafffa;
}

.form-group input:focus,
.form-group select:focus {
  outline: none;
  border-color: #4caf50;
  box-shadow: 0 0 0 3px rgba(76,175,80,0.15);
}

.form-group .hint {
  font-size: 0.82em;
  color: #777;
  margin-top: 4px;
}

.form-row {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 12px;
}

.btn {
  display: inline-block;
  padding: 12px 24px;
  border: none;
  border-radius: 8px;
  font-size: 1em;
  font-weight: 600;
  cursor: pointer;
  transition: all 0.3s;
  text-decoration: none;
}

.btn-primary {
  background: linear-gradient(135deg, #43a047 0%, #2e7d32 100%);
  color: white;
  box-shadow: 0 2px 8px rgba(46,125,50,0.3);
}

.btn-primary:hover {
  transform: translateY(-2px);
  box-shadow: 0 4px 16px rgba(46,125,50,0.4);
}

.btn-danger {
  background: linear-gradient(135deg, #ef5350 0%, #c62828 100%);
  color: white;
  box-shadow: 0 2px 8px rgba(198,40,40,0.3);
}

.btn-danger:hover {
  transform: translateY(-2px);
  box-shadow: 0 4px 16px rgba(198,40,40,0.4);
}

.btn-warning {
  background: linear-gradient(135deg, #ffa726 0%, #ef6c00 100%);
  color: white;
  box-shadow: 0 2px 8px rgba(239,108,0,0.3);
}

.btn-warning:hover {
  transform: translateY(-2px);
  box-shadow: 0 4px 16px rgba(239,108,0,0.4);
}

.btn-secondary {
  background: linear-gradient(135deg, #78909c 0%, #546e7a 100%);
  color: white;
}

.btn-secondary:hover {
  transform: translateY(-2px);
}

.btn-small {
  padding: 8px 16px;
  font-size: 0.85em;
}

.nav-links {
  display: flex;
  gap: 12px;
  flex-wrap: wrap;
  justify-content: center;
  margin-top: 16px;
}

.alert {
  padding: 14px 18px;
  border-radius: 8px;
  margin-bottom: 16px;
  font-weight: 500;
}

.alert-success {
  background: #e8f5e9;
  color: #1b5e20;
  border: 1px solid #a5d6a7;
}

.alert-info {
  background: #e3f2fd;
  color: #0d47a1;
  border: 1px solid #90caf9;
}

.alert-warning {
  background: #fff3e0;
  color: #e65100;
  border: 1px solid #ffcc80;
}

.alert-danger {
  background: #ffebee;
  color: #b71c1c;
  border: 1px solid #ef9a9a;
}

.schedule-card {
  background: #f9fdf9;
  border: 1px solid #c8e6c9;
  border-radius: 10px;
  padding: 16px;
  margin-bottom: 12px;
}

.schedule-card.enabled {
  border-left: 4px solid #4caf50;
}

.schedule-card.disabled {
  border-left: 4px solid #bdbdbd;
  opacity: 0.7;
}

.schedule-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 10px;
}

.schedule-header h3 {
  margin: 0;
}

.schedule-toggle {
  display: flex;
  align-items: center;
  gap: 8px;
}

.toggle-switch {
  position: relative;
  width: 48px;
  height: 24px;
  background: #bdbdbd;
  border-radius: 12px;
  cursor: pointer;
  transition: background 0.3s;
}

.toggle-switch.active {
  background: #4caf50;
}

.toggle-switch::after {
  content: '';
  position: absolute;
  width: 20px;
  height: 20px;
  border-radius: 50%;
  background: white;
  top: 2px;
  left: 2px;
  transition: transform 0.3s;
}

.toggle-switch.active::after {
  transform: translateX(24px);
}

.hidden {
  display: none;
}

.text-center {
  text-align: center;
}

.mt-16 {
  margin-top: 16px;
}

.mb-16 {
  margin-bottom: 16px;
}

.footer {
  text-align: center;
  color: #888;
  font-size: 0.85em;
  padding: 20px;
}

.weather-info {
  display: flex;
  align-items: center;
  gap: 12px;
  justify-content: center;
  flex-wrap: wrap;
}

.weather-badge {
  background: #e3f2fd;
  color: #1565c0;
  padding: 6px 14px;
  border-radius: 20px;
  font-size: 0.9em;
  font-weight: 500;
}

.last-action {
  font-style: italic;
  color: #555;
  font-size: 0.95em;
}

/* Responsive */
@media (max-width: 600px) {
  .header h1 { font-size: 1.3em; }
  .form-row { grid-template-columns: 1fr; }
  .status-grid { grid-template-columns: 1fr 1fr; }
  .nav-links { flex-direction: column; align-items: stretch; }
  .nav-links .btn { text-align: center; }
}
)rawliteral";

// ---------- JAVASCRIPT COMPARTIDO (PROGMEM) ----------
static const char SHARED_JS[] PROGMEM = R"rawliteral(
// Actualización automática del estado vía AJAX
function updateStatus() {
  fetch('/status')
    .then(response => response.json())
    .then(data => {
      // Actualizar estado del relé
      const relayEl = document.getElementById('relay-status');
      if (relayEl) {
        if (data.relayActive) {
          relayEl.className = 'value active';
          relayEl.textContent = 'ACTIVO';
        } else {
          relayEl.className = 'value inactive';
          relayEl.textContent = 'INACTIVO';
        }
      }

      // Actualizar tiempo restante
      const remainingEl = document.getElementById('remaining-time');
      if (remainingEl) {
        if (data.relayActive && data.remainingSeconds > 0) {
          const mins = Math.floor(data.remainingSeconds / 60);
          const secs = data.remainingSeconds % 60;
          remainingEl.textContent = mins + 'm ' + secs + 's';
        } else {
          remainingEl.textContent = '---';
        }
      }

      // Actualizar última acción
      const actionEl = document.getElementById('last-action');
      if (actionEl) {
        actionEl.textContent = data.lastAction || 'Ninguna';
      }

      // Actualizar clima
      const weatherEl = document.getElementById('weather-info');
      if (weatherEl && data.weatherDesc) {
        const prob = (data.rainProb >= 0) ? (data.rainProb * 100).toFixed(0) + '%' : 'N/A';
        weatherEl.innerHTML = '<span class="weather-badge">' + data.weatherDesc + '</span> ' +
                              '<span class="weather-badge">Lluvia: ' + prob + '</span>';
      }

      // Actualizar hora actual
      const timeEl = document.getElementById('current-time');
      if (timeEl && data.currentTime) {
        timeEl.textContent = data.currentTime;
      }
    })
    .catch(err => console.log('Error actualizando estado:', err));
}

// Actualizar cada 5 segundos
setInterval(updateStatus, 5000);
)rawliteral";

// ============================================================
// FUNCIONES QUE CONSTRUYEN LAS PÁGINAS HTML
// ============================================================

/**
 * @brief Construye la página principal (dashboard).
 */
String buildMainPage(SystemConfig &config, bool relayActive,
                     int remainingSeconds, String lastAction) {
  String page = F("<!DOCTYPE html><html lang='es'><head><meta charset='UTF-8'>");
  page += F("<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
  page += F("<title>Riego Inteligente ESP32</title>");
  page += F("<style>");
  page += FPSTR(STYLE_CSS);
  page += F("</style></head><body><div class='container'>");

  // Header
  page += F("<div class='header'>");
  page += F("<h1>🌱 Riego Inteligente ESP32</h1>");
  page += F("<p>Sistema Autónomo de Riego Agrícola</p>");
  page += F("</div>");

  // Última acción
  if (lastAction.length() > 0 && lastAction != "Ninguna") {
    page += F("<div class='alert alert-info'>");
    page += F("<span id='last-action-display'>");
    page += lastAction;
    page += F("</span></div>");
  }

  // Estado del sistema
  page += F("<div class='card'>");
  page += F("<h2>📊 Estado del Sistema</h2>");
  page += F("<div class='status-grid'>");

  // Estado del relé
  page += F("<div class='status-item'><div class='label'>Relé Bomba</div>");
  page += F("<div id='relay-status' class='value ");
  page += relayActive ? "active" : "inactive";
  page += F("'>");
  page += relayActive ? "ACTIVO" : "INACTIVO";
  page += F("</div></div>");

  // Tiempo restante
  page += F("<div class='status-item'><div class='label'>Tiempo Restante</div>");
  page += F("<div id='remaining-time' class='value' style='font-size:1.2em;'>");
  if (relayActive && remainingSeconds > 0) {
    int mins = remainingSeconds / 60;
    int secs = remainingSeconds % 60;
    page += String(mins) + "m " + String(secs) + "s";
  } else {
    page += "---";
  }
  page += F("</div></div>");

  // Hora actual
  page += F("<div class='status-item'><div class='label'>Hora Actual</div>");
  page += F("<div id='current-time' class='value' style='font-size:1.1em;'>");
  page += F("Conectando NTP...</div></div>");

  // Clima
  page += F("<div class='status-item'><div class='label'>Clima / Lluvia</div>");
  page += F("<div id='weather-info' class='value' style='font-size:1em;'>");
  page += F("<span class='weather-badge'>Consultando...</span>");
  page += F("</div></div>");

  page += F("</div></div>"); // fin status-grid y card

  // Riego manual
  page += F("<div class='card'>");
  page += F("<h2>💧 Riego Manual</h2>");
  page += F("<form action='/manual-irrigate' method='POST' style='display:flex;gap:12px;align-items:center;flex-wrap:wrap;'>");
  page += F("<label for='manual-dur' style='font-weight:600;'>Duración (min):</label>");
  page += F("<input type='number' id='manual-dur' name='duration' value='5' min='1' max='");
  page += String(MAX_IRRIGATION_MIN);
  page += F("' style='width:80px;padding:8px;border:2px solid #c8e6c9;border-radius:8px;'>");
  page += F("<button type='submit' class='btn btn-warning'>🚿 Iniciar Riego Manual</button>");
  page += F("</form>");
  page += F("<p class='last-action mt-16'><strong>Última acción:</strong> <span id='last-action'>");
  page += lastAction.length() > 0 ? lastAction : "Ninguna";
  page += F("</span></p>");
  page += F("</div>");

  // Próximo riego programado
  page += F("<div class='card'>");
  page += F("<h2>📅 Próximos Riegos</h2>");
  bool hasEnabled = false;
  for (int i = 0; i < MAX_SCHEDULES; i++) {
    if (config.schedules[i].enabled) {
      if (!hasEnabled) {
        hasEnabled = true;
        page += F("<ul style='list-style:none;padding:0;'>");
      }
      char timeStr[6];
      sprintf(timeStr, "%02d:%02d", config.schedules[i].hour, config.schedules[i].minute);
      page += F("<li style='padding:8px 12px;border-bottom:1px solid #e8f5e9;'>");
      page += F("🕐 <strong>");
      page += String(timeStr);
      page += F("</strong> — Duración: ");
      page += String(config.schedules[i].duration);
      page += F(" min</li>");
    }
  }
  if (!hasEnabled) {
    page += F("<p style='color:#888;'>No hay horarios habilitados. ");
    page += F("<a href='/schedules'>Configurar horarios</a></p>");
  } else {
    page += F("</ul>");
  }
  page += F("</div>");

  // Navegación
  page += F("<div class='card text-center'>");
  page += F("<div class='nav-links'>");
  page += F("<a href='/config' class='btn btn-primary'>⚙️ Configuración</a>");
  page += F("<a href='/schedules' class='btn btn-primary'>📋 Horarios de Riego</a>");
  page += F("</div>");
  page += F("</div>");

  // Footer
  page += F("<div class='footer'>");
  page += F("Riego Inteligente ESP32-S3 &copy; 2026");
  page += F("</div>");

  page += F("</div>"); // fin container

  // JavaScript
  page += F("<script>");
  page += FPSTR(SHARED_JS);

  // Inline: inicializar hora si está disponible
  page += F(
    "// Función para obtener hora actual del sistema\n"
    "function fetchInitialTime() {\n"
    "  fetch('/status').then(r=>r.json()).then(d=>{\n"
    "    if(d.currentTime) document.getElementById('current-time').textContent = d.currentTime;\n"
    "  }).catch(()=>{});\n"
    "}\n"
    "fetchInitialTime();\n"
  );
  page += F("</script>");

  page += F("</body></html>");
  return page;
}

/**
 * @brief Construye la página de configuración general.
 */
String buildConfigPage(SystemConfig &config) {
  String page = F("<!DOCTYPE html><html lang='es'><head><meta charset='UTF-8'>");
  page += F("<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
  page += F("<title>Configuración - Riego Inteligente</title>");
  page += F("<style>");
  page += FPSTR(STYLE_CSS);
  page += F("</style></head><body><div class='container'>");

  page += F("<div class='header'>");
  page += F("<h1>⚙️ Configuración General</h1>");
  page += F("</div>");

  page += F("<div class='card'>");
  page += F("<form action='/save-config' method='POST'>");

  // WiFi Cliente
  page += F("<h2>🌐 Conexión WiFi (Cliente)</h2>");
  page += F("<p style='margin-bottom:16px;color:#666;'>Configura la red WiFi local para que el ESP32 acceda a internet y consulte el clima.</p>");

  page += F("<div class='form-group'>");
  page += F("<label for='wifi_ssid'>Nombre de la Red (SSID)</label>");
  page += F("<input type='text' id='wifi_ssid' name='wifi_ssid' maxlength='31' value='");
  page += String(config.wifiSSID);
  page += F("' placeholder='Ej: MiFibra-XXXX'>");
  page += F("</div>");

  page += F("<div class='form-group'>");
  page += F("<label for='wifi_pass'>Contraseña</label>");
  page += F("<input type='password' id='wifi_pass' name='wifi_pass' maxlength='63' value='");
  page += String(config.wifiPass);
  page += F("' placeholder='Contraseña de la red WiFi'>");
  page += F("</div>");

  // OpenWeatherMap
  page += F("<h2 class='mt-16'>☁️ OpenWeatherMap API</h2>");
  page += F("<p style='margin-bottom:16px;color:#666;'>Obtén tu API Key gratuita en <a href='https://openweathermap.org/api' target='_blank'>openweathermap.org</a></p>");

  page += F("<div class='form-group'>");
  page += F("<label for='owm_api_key'>API Key</label>");
  page += F("<input type='text' id='owm_api_key' name='owm_api_key' maxlength='47' value='");
  page += String(config.owmApiKey);
  page += F("' placeholder='Ej: a1b2c3d4e5f6...'>");
  page += F("<div class='hint'>Deja vacío si no deseas usar verificación climática</div>");
  page += F("</div>");

  // Coordenadas GPS
  page += F("<h2 class='mt-16'>📍 Ubicación del Cultivo</h2>");
  page += F("<div class='form-row'>");

  page += F("<div class='form-group'>");
  page += F("<label for='latitude'>Latitud</label>");
  page += F("<input type='text' id='latitude' name='latitude' value='");
  page += String(config.latitude, 6);
  page += F("' placeholder='Ej: -33.456789'>");
  page += F("<div class='hint'>Negativa para el hemisferio sur</div>");
  page += F("</div>");

  page += F("<div class='form-group'>");
  page += F("<label for='longitude'>Longitud</label>");
  page += F("<input type='text' id='longitude' name='longitude' value='");
  page += String(config.longitude, 6);
  page += F("' placeholder='Ej: -70.654321'>");
  page += F("<div class='hint'>Negativa para el oeste</div>");
  page += F("</div>");

  page += F("</div>"); // fin form-row

  page += F("<div class='text-center mt-16'>");
  page += F("<button type='submit' class='btn btn-primary'>💾 Guardar Configuración</button>");
  page += F("<a href='/' class='btn btn-secondary' style='margin-left:8px;'>⬅ Volver</a>");
  page += F("</div>");

  page += F("</form>");
  page += F("</div>");

  page += F("</div></body></html>");
  return page;
}

/**
 * @brief Construye la página de horarios de riego.
 */
String buildSchedulesPage(SystemConfig &config) {
  String page = F("<!DOCTYPE html><html lang='es'><head><meta charset='UTF-8'>");
  page += F("<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
  page += F("<title>Horarios de Riego</title>");
  page += F("<style>");
  page += FPSTR(STYLE_CSS);
  page += F("</style></head><body><div class='container'>");

  page += F("<div class='header'>");
  page += F("<h1>📋 Horarios de Riego</h1>");
  page += F("<p>Configura hasta 4 horarios diarios de riego automatizado</p>");
  page += F("</div>");

  page += F("<div class='card'>");
  page += F("<form action='/save-schedules' method='POST'>");

  for (int i = 0; i < MAX_SCHEDULES; i++) {
    String prefix = "s" + String(i);
    bool enabled = config.schedules[i].enabled;

    page += F("<div class='schedule-card ");
    page += (enabled ? "enabled" : "disabled");
    page += F("'>");

    page += F("<div class='schedule-header'>");
    page += F("<h3>Programa #");
    page += String(i + 1);
    page += F("</h3>");

    // Toggle habilitado/deshabilitado usando checkbox oculto + JS
    page += F("<div class='schedule-toggle'>");
    page += F("<label style='font-size:0.9em;color:#666;'>Habilitado</label>");
    page += F("<input type='checkbox' id='");
    page += prefix; page += F("_en' name='");
    page += prefix; page += F("_en' value='1' ");
    page += (enabled ? "checked" : "");
    page += F(" onchange='this.form.submit()'>");
    page += F("</div>");
    page += F("</div>"); // fin schedule-header

    page += F("<div class='form-row'>");

    page += F("<div class='form-group'>");
    page += F("<label for='"); page += prefix; page += F("_hour'>Hora</label>");
    page += F("<select id='"); page += prefix; page += F("_hour' name='"); page += prefix; page += F("_hour'>");
    for (int h = 0; h < 24; h++) {
      page += F("<option value='"); page += String(h); page += F("'");
      if (config.schedules[i].hour == h) page += F(" selected");
      page += F(">"); page += (h < 10 ? "0" : ""); page += String(h); page += F(":00</option>");
    }
    page += F("</select>");
    page += F("</div>");

    page += F("<div class='form-group'>");
    page += F("<label for='"); page += prefix; page += F("_min'>Minuto</label>");
    page += F("<select id='"); page += prefix; page += F("_min' name='"); page += prefix; page += F("_min'>");
    for (int m = 0; m < 60; m += 5) {
      page += F("<option value='"); page += String(m); page += F("'");
      if (config.schedules[i].minute == m) page += F(" selected");
      page += F(">"); page += (m < 10 ? "0" : ""); page += String(m); page += F("</option>");
    }
    page += F("</select>");
    page += F("</div>");

    page += F("</div>"); // fin form-row

    page += F("<div class='form-group'>");
    page += F("<label for='"); page += prefix; page += F("_dur'>Duración (minutos, máx. ");
    page += String(MAX_IRRIGATION_MIN);
    page += F(")</label>");
    page += F("<input type='number' id='"); page += prefix;
    page += F("_dur' name='"); page += prefix;
    page += F("_dur' value='"); page += String(config.schedules[i].duration);
    page += F("' min='1' max='"); page += String(MAX_IRRIGATION_MIN); page += F("' required>");
    page += F("</div>");

    page += F("</div>"); // fin schedule-card
  }

  page += F("<div class='text-center mt-16'>");
  page += F("<button type='submit' class='btn btn-primary'>💾 Guardar Horarios</button>");
  page += F("<a href='/' class='btn btn-secondary' style='margin-left:8px;'>⬅ Volver</a>");
  page += F("</div>");

  page += F("</form>");
  page += F("</div>");

  page += F("</div></body></html>");
  return page;
}

/**
 * @brief Construye el JSON de estado para las actualizaciones AJAX.
 */
String buildStatusJSON(SystemConfig &config, bool relayActive,
                        int remainingSeconds, String lastAction,
                        float rainProb, String weatherDesc) {
  String json = "{";

  json += "\"relayActive\":" + String(relayActive ? "true" : "false") + ",";
  json += "\"remainingSeconds\":" + String(remainingSeconds) + ",";
  json += "\"lastAction\":\"" + lastAction + "\",";
  json += "\"rainProb\":" + String(rainProb, 2) + ",";
  json += "\"weatherDesc\":\"" + weatherDesc + "\",";

  // Hora actual si NTP está sincronizado
  time_t now = time(nullptr);
  struct tm *ti = localtime(&now);
  if (ti != nullptr && ti->tm_year > (2024 - 1900)) {
    char timeStr[20];
    sprintf(timeStr, "%02d:%02d:%02d", ti->tm_hour, ti->tm_min, ti->tm_sec);
    json += "\"currentTime\":\"" + String(timeStr) + "\"";
  } else {
    json += "\"currentTime\":\"--:--:--\"";
  }

  json += "}";
  return json;
}

#endif // WEBINTERFACE_H
