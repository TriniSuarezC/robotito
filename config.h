#pragma once
#include <Arduino.h>

// ======================================================================
//   >>>>>>>>>>   ESCRIBÍ ACÁ EL WIFI (entre las comillas)   <<<<<<<<<<
// ======================================================================
const char* const WIFI_SSID = "NOMBRE_RED";
const char* const WIFI_PASS = "CLAVE_RED";
// Tiene que ser una red de 2.4 GHz (el ESP32-C3 no se conecta a 5 GHz).
// ======================================================================

// Si no logra conectarse aunque el nombre y la clave estén bien, dejá esto
// en true: baja un poco la potencia del WiFi y ayuda en varias Super Mini.
const bool WIFI_TX_REDUCIDO = true;

// ---------- Ubicación (para el clima) y zona horaria ----------
const float LATITUD  = -34.6037;          // ejemplo: Buenos Aires. Poné la de tu ciudad.
const float LONGITUD = -58.3816;
const long  GMT_OFFSET_S = -3 * 3600;     // Argentina: UTC-3 (sin horario de verano)

// ---------- Fechas especiales ----------
const int JUNTOS_ANIO = 2026, JUNTOS_MES = 4, JUNTOS_DIA = 11;   // desde cuándo están juntos
const int CUMPLE_MES  = 10, CUMPLE_DIA = 25;                     // "Feliz Cumpleaños"
const int ANIV_MES    = 4,  ANIV_DIA   = 11;                     // "Feliz Aniversario"

// ---------- Hora grande cada hora ----------
// Si el robot está dormido, la hora grande lo despierta, pero solo entre estas horas
// (de noche no se prende solo para no iluminar el cuarto).
const int ANUNCIO_DESDE_H = 8;     // desde las 8:00
const int ANUNCIO_HASTA_H = 22;    // hasta las 22:00 inclusive

// ---------- Para probar antes de regalarlo (dejar en false al final) ----------
// Simula que hoy es PRUEBA_ANIO/MES/DIA: se ven "Feliz Cumpleaños" o "Aniversario"
const bool MODO_PRUEBA_FECHA = false;
const int  PRUEBA_ANIO = 2026, PRUEBA_MES = 10, PRUEBA_DIA = 25;
// En vez de cada hora, muestra la hora grande cada minuto
const bool MODO_PRUEBA_ANUNCIO_CADA_MINUTO = false;

// ---------- Pines ----------
const uint8_t PIN_SDA   = 9;
const uint8_t PIN_SCL   = 8;
const uint8_t PIN_BOTON = 1;    // el único botón (entre este pin y GND)

// ---------- Botones (ms) ----------
const uint32_t DEBOUNCE_MS      = 30;
const uint32_t LARGO_MS         = 1000;   // mantener: elegir / salir
const uint32_t MUY_LARGO_MS     = 5000;   // apagar
const uint32_t REPEAT_INICIO_MS = 450;
const uint32_t REPEAT_CADA_MS   = 130;

// ---------- Comportamiento (ms) ----------
const uint32_t SUENO_MS         = 10UL * 60UL * 1000UL;  // sin tocar nada -> se duerme
const uint32_t MENU_TIMEOUT_MS  = 30UL * 1000UL;         // en el menú -> vuelve a los ojos
const uint32_t GOLPES_PAUSA_MS    = 700;                 // pausa que separa los grupos de golpecitos
const uint32_t GOLPES_REINICIO_MS = 1800;                // si pasa más que esto, se empieza de nuevo
const uint32_t CLIMA_CACHE_MS   = 10UL * 60UL * 1000UL;  // el clima se reutiliza 10 minutos
