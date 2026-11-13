#ifndef RELAYCONTROL_H
#define RELAYCONTROL_H

#include <Arduino.h>
#include "config.h"

// ============================================================
// Control del relé de la bomba de agua
// ============================================================

/**
 * @brief Clase para el control no bloqueante del relé de riego.
 * 
 * Utiliza millis() para temporización sin bloquear el programa.
 * Incluye límite de seguridad de MAX_IRRIGATION_MIN minutos.
 */
class RelayControl {
private:
  uint8_t _pin;               // GPIO del relé
  bool _active;               // Estado actual (activo/inactivo)
  unsigned long _startTime;   // Timestamp de activación (millis)
  unsigned long _durationMs;  // Duración configurada en milisegundos
  unsigned long _maxDurationMs; // Duración máxima permitida

public:
  /**
   * @brief Constructor: inicializa valores por defecto.
   */
  RelayControl()
    : _pin(RELAY_PIN)
    , _active(false)
    , _startTime(0)
    , _durationMs(0)
    , _maxDurationMs((unsigned long)MAX_IRRIGATION_MIN * 60 * 1000UL)
  {}

  /**
   * @brief Inicializa el pin del relé y lo deja en estado inactivo.
   * @param pin Número del GPIO al que está conectado el relé
   */
  void begin(uint8_t pin) {
    _pin = pin;
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);  // Relé apagado (lógica activa en HIGH)
    _active = false;
    _startTime = 0;
    _durationMs = 0;
    Serial.printf("[Relay] Inicializado en GPIO %d\n", _pin);
  }

  /**
   * @brief Activa el relé por una duración determinada.
   * @param durationMinutes Duración en minutos (se limita a MAX_IRRIGATION_MIN)
   */
  void activate(uint16_t durationMinutes) {
    // Limitar duración al máximo seguro
    if (durationMinutes > MAX_IRRIGATION_MIN) {
      Serial.printf("[Relay] Duración %d min excede el máximo (%d min). Limitando.\n",
                    durationMinutes, MAX_IRRIGATION_MIN);
      durationMinutes = MAX_IRRIGATION_MIN;
    }

    _active = true;
    _startTime = millis();
    _durationMs = (unsigned long)durationMinutes * 60 * 1000UL;
    digitalWrite(_pin, HIGH);

    Serial.printf("[Relay] ACTIVADO por %d minutos (GPIO %d)\n", durationMinutes, _pin);
  }

  /**
   * @brief Desactiva el relé inmediatamente.
   */
  void deactivate() {
    if (_active) {
      digitalWrite(_pin, LOW);
      _active = false;
      _startTime = 0;
      _durationMs = 0;
      Serial.println(F("[Relay] DESACTIVADO"));
    }
  }

  /**
   * @brief Actualiza el estado del relé. Debe llamarse periódicamente desde loop().
   * Si el tiempo transcurrido supera la duración configurada, desactiva el relé.
   */
  void update() {
    if (!_active) return;

    unsigned long now = millis();
    unsigned long elapsed = now - _startTime;

    // Verificar si ha transcurrido el tiempo configurado
    if (elapsed >= _durationMs) {
      Serial.println(F("[Relay] Tiempo de riego completado. Desactivando."));
      deactivate();
    }
  }

  /**
   * @brief Indica si el relé está activo actualmente.
   * @return true si está activo, false si no
   */
  bool isActive() {
    return _active;
  }

  /**
   * @brief Devuelve los segundos restantes de la activación actual.
   * @return Segundos restantes, o 0 si no está activo
   */
  int getRemainingSeconds() {
    if (!_active) return 0;
    unsigned long elapsed = millis() - _startTime;
    if (elapsed >= _durationMs) return 0;
    return (int)((_durationMs - elapsed) / 1000UL);
  }

  /**
   * @brief Devuelve el pin GPIO configurado.
   * @return Número del pin
   */
  uint8_t getPin() const {
    return _pin;
  }
};

#endif // RELAYCONTROL_H
