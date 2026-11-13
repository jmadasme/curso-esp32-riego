# Capítulo 2: Captive Portal

**Duración:** 1.5 horas
**Objetivo:** Implementar un portal cautivo que redirija todo el tráfico web a nuestro ESP32

---

## 🎯 Objetivos del Capítulo

- Entender qué es un captive portal y cómo funciona
- Configurar el DNS server para hacer spoofing
- Implementar un servidor web básico en ESP32
- Redirigir todo el tráfico HTTP a la página del ESP32

---

## 📖 2.1 Teoría: DNS Spoofing y Portales Cautivos

### ¿Qué es un Captive Portal?

Un captive portal (portal cautivo) es una técnica que intercepta la navegación web del usuario y lo redirige a una página específica, normalmente para autenticación o mostrar términos de uso.

**Ejemplos cotidianos:**
- WiFi de hoteles que te piden aceptar términos
- Redes de aeropuertos que requieren login
- Cafeterías con WiFi gratuito

### ¿Cómo Funciona el DNS Spoofing?

```
Usuario escribe "google.com" en el navegador
               │
               ▼
Se hace una consulta DNS: "¿Cuál es la IP de google.com?"
               │
               ▼
El DNS Server del ESP32 intercepta y responde:
"google.com → 192.168.4.1" (la IP del ESP32)
               │
               ▼
El navegador carga la página del ESP32
```

**DNS Spoofing:** Consiste en responder a todas las consultas DNS con una IP falsa (en nuestro caso, la IP del ESP32).

### Componentes Necesarios

| Componente | Librería | Propósito |
|-----------|----------|-----------|
| **DNSServer** | `DNSServer.h` | Intercepta consultas DNS |
| **WebServer** | `WebServer.h` | Sirve páginas web |
| **WiFi AP** | `WiFi.h` | Crea el hotspot |

---

## 💻 2.2 Código: Portal Cautivo Básico

```cpp
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>

// Configuración
const char* ssid = "Riego-Hortaliza";
const char* password = "[REDACTED]";
const IPAddress apIP(192, 168, 4, 1);

// Servidores
DNSServer dnsServer;
WebServer server(80);

// Página HTML simple
const char* indexPage PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Portal Cautivo</title>
  <style>
    body { font-family: Arial; text-align: center; padding: 40px;
           background: #f0f7f0; }
    h1 { color: #2e7d32; }
    .card { background: white; padding: 30px; border-radius: 12px;
            max-width: 400px; margin: 0 auto; box-shadow: 0 2px 12px rgba(0,0,0,0.1); }
    .ip { font-size: 24px; color: #1b5e20; font-weight: bold; }
  </style>
</head>
<body>
  <div class="card">
    <h1>🌱 Portal Cautivo ESP32</h1>
    <p>¡Has sido redirigido al ESP32!</p>
    <p>Tu IP es: <span class="ip" id="clientIp"></span></p>
    <script>
      // Mostrar IP del cliente
      fetch('/api/ip')
        .then(r => r.text())
        .then(ip => document.getElementById('clientIp').textContent = ip);
    </script>
  </div>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  Serial.println("\n\n=== CAPTIVE PORTAL ===");

  // Configurar AP
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(ssid, password);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  // Iniciar DNS server
  dnsServer.start(53, "*", apIP);
  Serial.println("DNS Server iniciado");

  // Configurar rutas del servidor web
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", indexPage);
  });

  server.on("/api/ip", HTTP_GET, []() {
    server.send(200, "text/plain", server.client().remoteIP().toString());
  });

  // Redirigir cualquier otra URL al portal
  server.onNotFound([]() {
    server.sendHeader("Location", "http://192.168.4.1", true);
    server.send(302, "text/plain", "");
  });

  server.begin();
  Serial.println("Servidor web iniciado en puerto 80");
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
}
```

### Explicación

| Función | Propósito |
|---------|-----------|
| `WiFi.softAPConfig()` | Configurar IP fija del AP |
| `dnsServer.start(53, "*", apIP)` | DNS spoofing para cualquier dominio |
| `server.on("/")` | Ruta principal del portal |
| `server.onNotFound()` | Captura rutas no definidas y redirige |
| `dnsServer.processNextRequest()` | Atiende consultas DNS (llamar en loop) |

---

## 🔬 2.3 Laboratorio 2: Portal Cautivo Funcional

### Objetivo
Implementar un captive portal completo que redirija automáticamente.

### Paso a Paso

**Paso 1:** Crea un nuevo sketch en Arduino IDE.

**Paso 2:** Copia el código completo de la sección 2.2.

**Paso 3:** Sube el código al ESP32.

**Paso 4:** Conéctate al WiFi "Riego-Hortaliza" desde tu teléfono.

**Paso 5:** Abre el navegador e intenta navegar a **cualquier sitio** (google.com, facebook.com, etc.).

**Resultado:** Serás redirigido automáticamente al portal del ESP32.

### Verificación

1. ✅ Conectar al WiFi "Riego-Hortaliza"
2. ✅ Abrir navegador → aparece portal cautivo
3. ✅ El portal muestra tu IP de cliente
4. ✅ Cualquier URL redirige al portal
5. ✅ La página se ve bien en móvil (responsive)

---

## ✏️ 2.4 Ejercicio: Portal que Cuente Visitas

Modifica el portal para que muestre cuántas visitas ha tenido desde que se encendió el ESP32.

### Pistas

```cpp
int visitCount = 0;

server.on("/", HTTP_GET, []() {
  visitCount++;
  String page = "Has visitado esta pagina " + String(visitCount) + " veces.";
  server.send(200, "text/html", page);
});
```

---

## ✅ Resumen del Capítulo

| Concepto | Implementación |
|----------|---------------|
| ✅ DNS Spoofing | `dnsServer.start(53, "*", apIP)` |
| ✅ Servidor web | `WebServer server(80)` |
| ✅ Redirección automática | `server.onNotFound()` |
| ✅ Páginas desde PROGMEM | `R"rawliteral(...)"` |
| ✅ Manejo de rutas | `server.on("/ruta", método, handler)` |

### Diferencia Clave

Sin captive portal:
```
Usuario → debe escribir 192.168.4.1 en el navegador
```

Con captive portal:
```
Usuario → escribe cualquier URL → redirigido automáticamente
```

➡️ **Siguiente capítulo:** [03 - Servidor Web y HTML/CSS](03_servidor_web.md)
