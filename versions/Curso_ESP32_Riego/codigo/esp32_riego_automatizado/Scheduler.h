#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <Arduino.h>
#include <time.h>
#include "config.h"

// ============================================================
// Programador de horarios de riego
// ============================================================

/**
 * @brief Verifica si algún horario programado coincide con la hora actual.
 * 
 * Cada schedule tiene una ventana de 1 minuto para ser detectado.
 * El sistema debe evitar ejecutar el mismo horario más de una vez por día.
 * 
 * @param config   Configuración del sistema con los horarios
 * @param timeinfo Estructura tm con la hora actual (de NTP)
 * @return int     Índice del horario a ejecutar (0 a MAX_SCHEDULES-1),
 *                 o -1 si ningún horario coincide
 */
int checkSchedules(SystemConfig &config, struct tm *timeinfo) {
  if (timeinfo == nullptr) {
    return -1;
  }

  int currentHour = timeinfo->tm_hour;
  int currentMin = timeinfo->tm_min;

  for (int i = 0; i < MAX_SCHEDULES; i++) {
    if (config.schedules[i].enabled) {
      // Verificar coincidencia exacta de hora y minuto
      // Ventana de 1 minuto: se ejecuta cuando el reloj marca exactamente
      // la hora y minuto configurados
      if (config.schedules[i].hour == currentHour &&
          config.schedules[i].minute == currentMin) {
        Serial.printf("[Scheduler] Horario %d coincide: %02d:%02d (duración: %d min)\n",
                      i + 1,
                      config.schedules[i].hour,
                      config.schedules[i].minute,
                      config.schedules[i].duration);
        return i;
      }
    }
  }

  return -1; // Ningún horario coincide
}

#endif // SCHEDULER_H
