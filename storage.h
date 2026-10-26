#pragma once
#include <Preferences.h>
#include "display.h"

// Memoria que sobrevive a desenchufar el robot: récords y "ya vio" de las fechas.
Preferences prefs;

void almacenamientoIniciar() { prefs.begin("robotito", false); }

uint16_t leerU16(const char* clave) { return prefs.getUShort(clave, 0); }
void guardarU16(const char* clave, uint16_t v) { prefs.putUShort(clave, v); }

// Guarda el puntaje si supera el récord. Devuelve true si es récord nuevo.
bool registrarRecord(const char* clave, uint16_t puntaje) {
  if (puntaje > leerU16(clave)) {
    guardarU16(clave, puntaje);
    return true;
  }
  return false;
}
