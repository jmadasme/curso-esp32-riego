#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>
#include <EEPROM.h>
#include "config.h"

// ============================================================
// Módulo de persistencia en EEPROM
// ============================================================

/**
 * @brief Calcula el checksum de 16 bits sumando todos los bytes de la config.
 * @param config Referencia a la estructura SystemConfig
 * @return Valor del checksum calculado
 */
uint16_t calculateChecksum(SystemConfig &config) {
  uint16_t sum = 0;
  uint8_t *bytes = (uint8_t *)&config;
  // Restamos 2 porque los últimos 2 bytes son el checksum mismo
  size_t len = sizeof(SystemConfig) - sizeof(uint16_t);
  for (size_t i = 0; i < len; i++) {
    sum += bytes[i];
  }
  return sum;
}

/**
 * @brief Guarda la configuración completa en EEPROM.
 * @param config Referencia a la estructura SystemConfig a guardar
 */
void saveConfig(SystemConfig &config) {
  // Calcular y actualizar checksum antes de guardar
  config.checksum = calculateChecksum(config);

  EEPROM.begin(EEPROM_SIZE);

  // Escribir byte a byte la estructura completa
  uint8_t *bytes = (uint8_t *)&config;
  for (size_t i = 0; i < sizeof(SystemConfig); i++) {
    EEPROM.write(i, bytes[i]);
  }

  EEPROM.commit();
  EEPROM.end();

  Serial.println(F("[Storage] ConfiguraciOn guardada correctamente en EEPROM."));
}

/**
 * @brief Carga la configuración desde EEPROM.
 * @param config Referencia donde se cargarán los datos
 * @return true si la carga fue exitosa y el checksum es válido, false en caso contrario
 */
bool loadConfig(SystemConfig &config) {
  EEPROM.begin(EEPROM_SIZE);

  // Leer byte a byte la estructura
  uint8_t *bytes = (uint8_t *)&config;
  for (size_t i = 0; i < sizeof(SystemConfig); i++) {
    bytes[i] = EEPROM.read(i);
  }

  EEPROM.end();

  // Verificar checksum
  uint16_t savedChecksum = config.checksum;
  config.checksum = 0; // Temporal para recalcular
  uint16_t calculated = calculateChecksum(config);
  config.checksum = savedChecksum; // Restaurar

  if (savedChecksum == calculated) {
    Serial.println(F("[Storage] ConfiguraciOn cargada correctamente (checksum OK)."));
    return true;
  }

  Serial.println(F("[Storage] Error de checksum. ConfiguraciOn corrupta o vacIa."));
  return false;
}

/**
 * @brief Limpia la configuración en EEPROM (resetea a ceros).
 */
void clearConfig() {
  EEPROM.begin(EEPROM_SIZE);

  for (int i = 0; i < EEPROM_SIZE; i++) {
    EEPROM.write(i, 0);
  }

  EEPROM.commit();
  EEPROM.end();

  Serial.println(F("[Storage] ConfiguraciOn eliminada de EEPROM."));
}

#endif // STORAGE_H
