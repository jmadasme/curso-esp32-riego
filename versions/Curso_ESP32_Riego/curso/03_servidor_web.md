# Capítulo 3: Servidor Web y HTML/CSS

**Duración:** 2 horas
**Objetivo:** Construir una interfaz web atractiva y responsive alojada en el ESP32

---

## 🎯 Objetivos del Capítulo

- Entender el protocolo HTTP y los métodos GET/POST
- Servir páginas HTML completas desde el ESP32
- Crear formularios y procesar datos
- Diseñar una interfaz responsive con CSS moderno
- Actualizar datos en tiempo real con JavaScript

---

## 📖 3.1 Teoría: HTTP y Servidores Web

### Protocolo HTTP

HTTP (Hypertext Transfer Protocol) es el lenguaje que usan los navegadores y servidores web para comunicarse.

**Estructura de una petición HTTP:**

```
GET /pagina.html HTTP/1.1
Host: 192.168.4.1
User-Agent: Mozilla/5.0
```

**Estructura de una respuesta HTTP:**

```
HTTP/1.1 200 OK
Content-Type: text/html

<html>...contenido...</html>
```

### Métodos HTTP

| Método | Propósito | Ejemplo |
|--------|-----------|---------|
| **GET** | Obtener datos | Cargar una página, pedir JSON |
| **POST** | Enviar datos | Guardar configuración, activar riego |

### Códigos de Estado

| Código | Significado | Uso |
|--------|-------------|-----|
| 200 | OK | Todo funcionó correctamente |
| 302 | Redirección | Redirigir a otra página |
| 404 | No encontrado | Ruta inexistente |
| 500 | Error interno | Algo salió mal |

---

## 💻 3.2 Código: Servidor con Formularios

### Servidor con GET y POST

```cpp
#include <WiFi.h>
#include <WebServer.h>

WebServer server(80);
String mensaje = "";

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_AP);
  WiFi.softAP("WebDemo", "12345678");

  // Ruta GET: mostrar formulario
  server.on("/", HTTP_GET, []() {
    String html = "<!DOCTYPE html><html>";
    html += "<head><meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<title>Demo Web ESP32</title>";
    html += "<style>";
    html += "body { font-family: 'Segoe UI', sans-serif; ";
    html += "  background: #f0f7f0; padding: 20px; text-align: center; }";
    html += ".card { background: white; border-radius: 12px; padding: 24px; ";
    html += "  max-width: 500px; margin: 0 auto; box-shadow: 0 2px 12px rgba(0,0,0,0.1); }";
    html += "h1 { color: #2e7d32; }";
    html += "input { padding: 10px; border: 2px solid #c8e6c9; border-radius: 8px; ";
    html += "  width: 80%; font-size: 1em; margin: 8px 0; }";
    html += "button { background: #2e7d32; color: white; border: none; ";
    html += "  padding: 12px 24px; border-radius: 8px; font-size: 1em; cursor: pointer; }";
    html += ".msg { background: #e8f5e9; padding: 12px; border-radius: 8px; ";
    html += "  margin-top: 16px; color: #1b5e20; }";
    html += "</style></head><body>";
    html += "<div class='card'>";
    html += "<h1>🌐 Demo Web ESP32</h1>";
    html += "<form action='/guardar' method='POST'>";
    html += "<input type='text' name='mensaje' placeholder='Escribe algo...'>";
    html += "<br><br><button type='submit'>Enviar</button>";
    html += "</form>";
    if (mensaje.length() > 0) {
      html += "<div class='msg'>📨 " + mensaje + "</div>";
    }
    html += "<p style='margin-top:20px;color:#888;'>";
    html += "<a href='/datos' style='color:#2e7d32;'>Ver datos en JSON</a></p>";
    html += "</div></body></html>";
    server.send(200, "text/html", html);
  });

  // Ruta POST: recibir datos del formulario
  server.on("/guardar", HTTP_POST, []() {
    mensaje = server.arg("mensaje");
    Serial.println("Mensaje recibido: " + mensaje);
    // Redirigir de vuelta a la página principal
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
  });

  // Ruta JSON
  server.on("/datos", HTTP_GET, []() {
    String json = "{\"mensaje\":\"" + mensaje + "\",";
    json += "\"uptime\":" + String(millis() / 1000) + "}";
    server.send(200, "application/json", json);
  });

  server.begin();
  Serial.println("Servidor iniciado en http://192.168.4.1");
}

void loop() {
  server.handleClient();
}
```

### Procesamiento de Formularios

| Función | Descripción |
|---------|-------------|
| `server.arg("nombre")` | Obtiene el valor de un campo del formulario |
| `server.sendHeader("Location", url, true)` | Redirección HTTP |
| `server.send(302, ...)` | Enviar código de redirección |

---

## 🎨 3.3 CSS: Diseño Responsive y Colores Agrícolas

### Paleta de Colores del Proyecto

```
Verde oscuro  : #1b5e20  (títulos, énfasis)
Verde medio   : #2e7d32  (botones, headers)
Verde claro   : #4caf50  (acentos, bordes)
Fondo claro   : #f0f7f0  (fondo general)
Blanco        : #ffffff  (tarjetas)
Texto         : #2d3a2d  (texto principal)
```

### Técnicas CSS Clave

```css
/* Diseño responsive */
.status-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
  gap: 16px;
}

/* Adaptación a móviles */
@media (max-width: 600px) {
  .form-row { grid-template-columns: 1fr; }
}

/* Tarjetas con sombra */
.card {
  background: white;
  border-radius: 12px;
  padding: 24px;
  box-shadow: 0 2px 12px rgba(0,0,0,0.08);
  border-left: 4px solid #4caf50;
}
```

---

## 🔬 3.4 Laboratorio 3: Dashboard Interactivo

### Objetivo
Crear un dashboard web con:
- Estado del sistema (encendido/apagado)
- Contador de tiempo encendido
- Botón para simular activación
- Actualización automática cada 5 segundos

### Código del Dashboard

```cpp
// Fragmento del dashboard con JavaScript para actualización automática
server.on("/", HTTP_GET, []() {
  String html = "..."; // HTML con IDs para actualizar
  
  // JavaScript para polling cada 5s
  html += "<script>";
  html += "setInterval(() => {";
  html += "  fetch('/api/status').then(r=>r.json()).then(data => {";
  html += "    document.getElementById('uptime').textContent = data.uptime + 's';";
  html += "    const rel = document.getElementById('relay');";
  html += "    rel.textContent = data.active ? 'ACTIVO' : 'INACTIVO';";
  html += "    rel.className = data.active ? 'active' : 'inactive';";
  html += "  });";
  html += "}, 5000);";
  html += "</script>";
  
  server.send(200, "text/html", html);
});
```

---

## ✏️ 3.5 Ejercicio: Formulario que Devuelva Datos

Crea una página con:
1. Un campo para ingresar la duración del riego (en minutos)
2. Un botón "Simular Riego"
3. Al presionar, muestre un mensaje: "Riego activado por X minutos"
4. Un contador regresivo que se actualice cada segundo

---

## ✅ Resumen del Capítulo

| Concepto | Implementación |
|----------|---------------|
| ✅ Servir HTML | `server.send(200, "text/html", html)` |
| ✅ Formularios POST | `server.on("/ruta", HTTP_POST, handler)` |
| ✅ Obtener datos | `server.arg("nombre_campo")` |
| ✅ Redirección | `server.sendHeader("Location", ...)` |
| ✅ JSON API | `server.send(200, "application/json", json)` |
| ✅ CSS moderno | Grid, flexbox, responsive |
| ✅ JS en tiempo real | `fetch()` + `setInterval()` |

### Claves para un Buen Dashboard

1. **Información clara** — Muestra lo esencial primero
2. **Responsive** — Funciona en móvil y escritorio
3. **Tiempo real** — Actualización automática (AJAX)
4. **Colores significativos** — Verde = OK, Rojo = Alerta
5. **Acciones claras** — Botones con texto descriptivo

➡️ **Siguiente capítulo:** [04 - EEPROM y Persistencia de Datos](04_eeprom_persistencia.md)
