#pragma once
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <time.h>
#include <string.h>
#include "config.h"

// ======================= WiFi y hora (NTP) =======================
bool wifiConectado = false;
bool sntpIniciado = false;

void redIniciar() {
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  if (WIFI_TX_REDUCIDO) WiFi.setTxPower(WIFI_POWER_8_5dBm);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
}

// Llamar seguido desde el loop (internamente corre una vez por segundo)
void redActualizar(uint32_t ahora) {
  static uint32_t tUlt = 0, tIntento = 0;
  if (ahora - tUlt < 1000) return;
  tUlt = ahora;
  wifiConectado = (WiFi.status() == WL_CONNECTED);
  if (wifiConectado) {
    if (!sntpIniciado) {
      configTime(GMT_OFFSET_S, 0, "pool.ntp.org", "time.google.com", "time.cloudflare.com");
      sntpIniciado = true;
    }
  } else if (ahora - tIntento > 30000) {   // si se cayó, reintenta cada 30 s
    tIntento = ahora;
    WiFi.begin(WIFI_SSID, WIFI_PASS);
  }
}

// ======================= Fecha y hora locales =======================
struct Ahora { int anio, mes, dia, hora, min, seg, dsem; };   // dsem: 0 = domingo

int diaSemana(int y, int m, int d) {
  static const int t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
  if (m < 3) y -= 1;
  return (y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7;
}

// Devuelve false mientras todavía no se sincronizó la hora por internet
bool leerAhora(Ahora& a) {
  struct tm t;
  if (!getLocalTime(&t, 0)) return false;
  a.anio = t.tm_year + 1900;
  a.mes  = t.tm_mon + 1;
  a.dia  = t.tm_mday;
  a.hora = t.tm_hour;
  a.min  = t.tm_min;
  a.seg  = t.tm_sec;
  a.dsem = t.tm_wday;
  if (MODO_PRUEBA_FECHA) {
    a.anio = PRUEBA_ANIO; a.mes = PRUEBA_MES; a.dia = PRUEBA_DIA;
    a.dsem = diaSemana(a.anio, a.mes, a.dia);
  }
  return true;
}

// Días desde 1970-01-01 para una fecha civil
long diasDesdeCivil(int y, int m, int d) {
  y -= (m <= 2);
  long era = (y >= 0 ? y : y - 399) / 400;
  long yoe = y - era * 400;
  long doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
  long doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return era * 146097 + doe - 719468;
}

long diasJuntos(const Ahora& a) {
  return diasDesdeCivil(a.anio, a.mes, a.dia) - diasDesdeCivil(JUNTOS_ANIO, JUNTOS_MES, JUNTOS_DIA);
}

// ======================= Clima (Open-Meteo, sin clave) =======================
struct Clima { bool ok; float temp, tmax, tmin; int codigo; int lluvia; uint32_t tObtenido; };
Clima clima = { false, 0, 0, 0, 0, 0, 0 };

// Busca "clave": a partir de 'desde' y lee el número que sigue
bool leerNumero(const String& js, int desde, const char* clave, float& out) {
  char k[48];
  snprintf(k, sizeof(k), "\"%s\":", clave);
  int i = js.indexOf(String(k), desde);
  if (i < 0) return false;
  i += (int)strlen(k);
  while (js.charAt(i) == '[' || js.charAt(i) == ' ') i++;
  out = js.substring(i, i + 12).toFloat();
  return true;
}

// Consulta (bloquea unos segundos). Devuelve true si logró datos nuevos.
bool climaObtener() {
  if (WiFi.status() != WL_CONNECTED) return false;
  char url[330];
  snprintf(url, sizeof(url),
           "https://api.open-meteo.com/v1/forecast?latitude=%.4f&longitude=%.4f"
           "&current=temperature_2m,weather_code"
           "&daily=temperature_2m_max,temperature_2m_min,precipitation_probability_max"
           "&timezone=auto&forecast_days=1",
           LATITUD, LONGITUD);
  WiFiClientSecure cli;
  cli.setInsecure();
  HTTPClient http;
  if (!http.begin(cli, String(url))) return false;
  http.setTimeout(6000);
  int code = http.GET();
  bool ok = false;
  if (code == 200) {
    String js = http.getString();
    int ic = js.indexOf(String("\"current\":{"));
    int id = js.indexOf(String("\"daily\":{"));
    float t, c, mx, mn, ll;
    if (ic >= 0 && id >= 0 &&
        leerNumero(js, ic, "temperature_2m", t) &&
        leerNumero(js, ic, "weather_code", c) &&
        leerNumero(js, id, "temperature_2m_max", mx) &&
        leerNumero(js, id, "temperature_2m_min", mn)) {
      if (!leerNumero(js, id, "precipitation_probability_max", ll)) ll = 0;
      clima.temp = t; clima.codigo = (int)c; clima.tmax = mx; clima.tmin = mn;
      clima.lluvia = (int)ll; clima.ok = true; clima.tObtenido = millis();
      ok = true;
    }
  }
  http.end();
  return ok;
}
