#pragma once
#include "display.h"
#include "buttons.h"
#include "sprites.h"
#include "net.h"
#include "jokes.h"

// Una "app" es una pantalla con su propia lógica (hora, cronómetro, juegos...).
//   Un toque hace la acción de la pantalla. Mantener 1 s sale al menú,
//   salvo que la app tenga su propio "alMantener" (cronómetro y timer).
struct App {
  void (*iniciar)(uint32_t ahora);       // al abrirla
  bool (*actualizar)(uint32_t ahora);    // cada vuelta del loop; true = salir al menú
  bool (*alMantener)(uint32_t ahora);    // al mantener 1 s: true = salir; false = lo manejó la app (nullptr = salir)
  uint32_t timeoutMs;                    // 0 = sin límite; si no se toca nada, vuelve a los ojos
};

const char* const DIAS[]  = { "Domingo", "Lunes", "Martes", "Miércoles", "Jueves", "Viernes", "Sábado" };
const char* const MESES[] = { "ene", "feb", "mar", "abr", "may", "jun", "jul", "ago", "sep", "oct", "nov", "dic" };

// ======================= Chispas (festejo al tirar la moneda o el dado) =======================
struct Chispa { float x0, y0, vx, vy; };
Chispa chispas[24];
bool chispasOn = false;
uint32_t chispasT0 = 0;

void chispasLanzar(int cx, int cy, uint32_t ahora) {
  for (uint8_t i = 0; i < 24; i++) {
    float a = random(0, 628) / 100.0f;
    float v = random(8, 30) / 10.0f;
    chispas[i].x0 = (float)cx;
    chispas[i].y0 = (float)cy;
    chispas[i].vx = v * cosf(a);
    chispas[i].vy = v * sinf(a) - 1.2f;
  }
  chispasT0 = ahora;
  chispasOn = true;
}

void chispasDibujar(uint32_t ahora) {
  if (!chispasOn) return;
  float t = (ahora - chispasT0) / 33.0f;
  if (t > 38) { chispasOn = false; return; }
  for (uint8_t i = 0; i < 24; i++) {
    int x = (int)(chispas[i].x0 + chispas[i].vx * t);
    int y = (int)(chispas[i].y0 + chispas[i].vy * t + 0.06f * t * t);
    if (i % 3 == 0) bx(x, y, 2, 2); else px(x, y);
  }
}

// ======================= Hora =======================
void horaIniciar(uint32_t) {}
bool horaActualizar(uint32_t ahora) {
  if (!frameListo(ahora)) return false;
  Ahora a;
  u8g2.clearBuffer();
  if (!leerAhora(a)) {
    u8g2.setFont(u8g2_font_7x13_tf);
    textoCentrado("Sin hora todavía", 28);
    u8g2.setFont(u8g2_font_6x10_tf);
    textoCentrado(wifiConectado ? "Sincronizando..." : "Revisá el WiFi", 46);
  } else {
    char hm[8], f[24];
    snprintf(hm, sizeof(hm), "%02d:%02d", a.hora, a.min);
    u8g2.setFont(u8g2_font_logisoso28_tn);
    textoCentrado(hm, 30);
    u8g2.setFont(u8g2_font_6x10_tf);
    textoCentrado(DIAS[a.dsem], 46);
    snprintf(f, sizeof(f), "%d %s %d", a.dia, MESES[a.mes - 1], a.anio);
    textoCentrado(f, 59);
  }
  u8g2.sendBuffer();
  return false;
}
const App APP_HORA = { horaIniciar, horaActualizar, nullptr, 30000 };

// ======================= Clima =======================
uint8_t climaFase = 0;   // 0 = hay que consultar, 1 = mostrar
void climaIniciar(uint32_t ahora) {
  bool fresco = clima.ok && (ahora - clima.tObtenido < CLIMA_CACHE_MS);
  climaFase = fresco ? 1 : 0;
}
bool climaActualizar(uint32_t ahora) {
  if (climaFase == 0) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_7x13_tf);
    textoCentrado("Consultando...", 36);
    u8g2.sendBuffer();
    climaObtener();           // bloquea unos segundos
    climaFase = 1;
    return false;
  }
  if (!frameListo(ahora)) return false;
  u8g2.clearBuffer();
  if (!clima.ok) {
    u8g2.setFont(u8g2_font_7x13_tf);
    textoCentrado("Sin datos", 28);
    u8g2.setFont(u8g2_font_6x10_tf);
    textoCentrado(wifiConectado ? "No pude consultar" : "Sin WiFi", 46);
  } else {
    Ahora a;
    bool noche = leerAhora(a) && (a.hora >= 20 || a.hora < 7);
    iconoClima(24, 28, tipoClimaDeCodigo(clima.codigo, noche), ahora);
    char buf[28];
    snprintf(buf, sizeof(buf), "%d°C", (int)lroundf(clima.temp));
    u8g2.setFont(u8g2_font_helvB18_tf);
    u8g2.drawUTF8(52, 32, buf);
    u8g2.setFont(u8g2_font_6x10_tf);
    snprintf(buf, sizeof(buf), "Mín %d°  Máx %d°", (int)lroundf(clima.tmin), (int)lroundf(clima.tmax));
    textoCentrado(buf, 52);
    snprintf(buf, sizeof(buf), "Lluvia %d%%", clima.lluvia);
    textoCentrado(buf, 62);
  }
  u8g2.sendBuffer();
  return false;
}
const App APP_CLIMA = { climaIniciar, climaActualizar, nullptr, 30000 };

// ======================= Días juntos =======================
void diasIniciar(uint32_t) {}
bool diasActualizar(uint32_t ahora) {
  if (!frameListo(ahora)) return false;
  Ahora a;
  u8g2.clearBuffer();
  if (!leerAhora(a)) {
    u8g2.setFont(u8g2_font_7x13_tf);
    textoCentrado("Sin hora todavía", 28);
    u8g2.setFont(u8g2_font_6x10_tf);
    textoCentrado(wifiConectado ? "Sincronizando..." : "Revisá el WiFi", 46);
  } else {
    long d = diasJuntos(a);
    if (d < 0) d = 0;
    char buf[16];
    snprintf(buf, sizeof(buf), "%ld", d);
    // corazones que suben a los costados
    const int xs[4] = { 12, 26, 102, 116 };
    for (int k = 0; k < 4; k++) {
      int y = 70 - (int)((ahora / 30 + k * 37) % 80);
      if (y > 8 && y < 62) corazon(xs[k], y, (k % 2) ? 2 : 3);
    }
    u8g2.setFont(u8g2_font_6x10_tf);
    textoCentrado("Juntos hace", 11);
    fuenteHoraQueEntra(buf, 70);
    textoCentrado(buf, 46);
    u8g2.setFont(u8g2_font_6x10_tf);
    textoCentrado(d == 1 ? "día" : "días", 61);
  }
  u8g2.sendBuffer();
  return false;
}
const App APP_DIAS = { diasIniciar, diasActualizar, nullptr, 30000 };

// ======================= Cronómetro =======================
// Toque: inicia / pausa.  Mantener 1 s: reinicia a cero; si ya está en cero, sale.
bool cronoCorre = false;
uint32_t cronoInicio = 0, cronoAcum = 0;

uint32_t cronoMs(uint32_t ahora) { return cronoCorre ? cronoAcum + (ahora - cronoInicio) : cronoAcum; }

void cronoIniciar(uint32_t) {}
bool cronoAlMantener(uint32_t) {
  if (cronoCorre || cronoAcum > 0) {      // primero reinicia
    cronoCorre = false;
    cronoAcum = 0;
    return false;
  }
  return true;                            // ya estaba en cero: sale
}
bool cronoActualizar(uint32_t ahora) {
  if (enter.corto) {
    if (cronoCorre) { cronoAcum += ahora - cronoInicio; cronoCorre = false; }
    else            { cronoInicio = ahora; cronoCorre = true; }
  }
  if (!frameListo(ahora)) return false;
  uint32_t ms = cronoMs(ahora);
  char hm[8], cs[4];
  snprintf(hm, sizeof(hm), "%02u:%02u", (unsigned)((ms / 60000) % 100), (unsigned)((ms / 1000) % 60));
  snprintf(cs, sizeof(cs), ".%02u", (unsigned)((ms / 10) % 100));

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_logisoso28_tn);
  int wHm = u8g2.getStrWidth(hm);
  u8g2.setFont(u8g2_font_7x13_tf);
  int wCs = u8g2.getUTF8Width(cs);
  int x0 = (128 - (wHm + 3 + wCs)) / 2;
  u8g2.setFont(u8g2_font_logisoso28_tn);
  u8g2.drawStr(x0, 34, hm);
  u8g2.setFont(u8g2_font_7x13_tf);
  u8g2.drawUTF8(x0 + wHm + 3, 34, cs);

  u8g2.setFont(u8g2_font_5x7_tf);
  if (cronoCorre) {
    textoCentrado("toque: pausa", 62);
  } else if (cronoAcum > 0) {
    textoCentrado("toque: sigue", 54);
    textoCentrado("mantener: reinicia", 62);
  } else {
    textoCentrado("toque: inicia", 54);
    textoCentrado("mantener: salir", 62);
  }
  u8g2.sendBuffer();
  return false;
}
const App APP_CRONO = { cronoIniciar, cronoActualizar, cronoAlMantener, 0 };

// ======================= Timer =======================
// Toque: cambia el tiempo (5, 10, 15, 25 min o Volver).  Mantener 1 s: lo inicia.
// Corriendo: toque = pausa, mantener = salir (sigue corriendo).  Pausado: toque = sigue, mantener = cancela.
const uint16_t TIMER_PRESETS[] = { 300, 600, 900, 1500 };
const uint8_t  N_PRESETS = 4;
struct TimerSt { uint8_t sel; bool corriendo; bool pausado; uint32_t finMs; uint32_t restanteMs; uint32_t totalMs; };
TimerSt tmr = { 0, false, false, 0, 0, 0 };

// Lo revisa el loop principal: devuelve true UNA vez cuando se terminó el tiempo
bool timerVencido(uint32_t ahora) {
  if (tmr.corriendo && (int32_t)(ahora - tmr.finMs) >= 0) {
    tmr.corriendo = false;
    tmr.pausado = false;
    return true;
  }
  return false;
}

void timerIniciar(uint32_t) {}
bool timerAlMantener(uint32_t ahora) {
  if (tmr.corriendo) return true;                      // sale; el timer sigue
  if (tmr.pausado)   { tmr.pausado = false; return false; }   // cancela
  if (tmr.sel >= N_PRESETS) return true;               // "Volver"
  tmr.totalMs = TIMER_PRESETS[tmr.sel] * 1000UL;       // inicia
  tmr.finMs = ahora + tmr.totalMs;
  tmr.corriendo = true;
  return false;
}
bool timerActualizar(uint32_t ahora) {
  if (enter.corto) {
    if (tmr.corriendo) {
      tmr.restanteMs = tmr.finMs - ahora;
      tmr.corriendo = false;
      tmr.pausado = true;
    } else if (tmr.pausado) {
      tmr.finMs = ahora + tmr.restanteMs;
      tmr.corriendo = true;
      tmr.pausado = false;
    } else {
      tmr.sel = (uint8_t)((tmr.sel + 1) % (N_PRESETS + 1));
    }
  }

  if (!frameListo(ahora)) return false;
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(2, 8, "TIMER");
  const char* l1;
  const char* l2;
  if (!tmr.corriendo && !tmr.pausado && tmr.sel >= N_PRESETS) {
    u8g2.setFont(u8g2_font_helvB18_tf);
    textoCentrado("Volver", 38);
    l1 = "toque: cambia";
    l2 = "mantener: volver";
  } else {
    uint32_t restMs = tmr.corriendo ? (tmr.finMs - ahora) : (tmr.pausado ? tmr.restanteMs : TIMER_PRESETS[tmr.sel] * 1000UL);
    uint32_t seg = (restMs + 999) / 1000;
    char t[8];
    snprintf(t, sizeof(t), "%02u:%02u", (unsigned)(seg / 60), (unsigned)(seg % 60));
    bool ocultar = tmr.pausado && ((ahora / 500) % 2);
    if (!ocultar) {
      u8g2.setFont(u8g2_font_logisoso28_tn);
      textoCentrado(t, 38);
    }
    if (tmr.corriendo || tmr.pausado) {
      u8g2.drawFrame(4, 40, 120, 7);
      int w = (int)((uint64_t)116 * restMs / (tmr.totalMs ? tmr.totalMs : 1));
      bx(6, 42, imin(w, 116), 3);
    }
    if (tmr.corriendo)     { l1 = "toque: pausa";  l2 = "mantener: salir"; }
    else if (tmr.pausado)  { l1 = "toque: sigue";  l2 = "mantener: cancela"; }
    else                   { l1 = "toque: cambia"; l2 = "mantener: inicia"; }
  }
  u8g2.setFont(u8g2_font_5x7_tf);
  textoCentrado(l1, 55);
  textoCentrado(l2, 63);
  u8g2.sendBuffer();
  return false;
}
const App APP_TIMER = { timerIniciar, timerActualizar, timerAlMantener, 60000 };

// ======================= Cara o cruz =======================
uint8_t monFase = 0;      // 0 listo, 1 girando, 2 resultado
bool monCara = true;
uint32_t monT0 = 0;

void dibujarMoneda(int cx, int cy, int rx, bool cara) {
  elip(cx, cy, rx, 18);
  if (rx < 9) return;
  u8g2.setDrawColor(0);
  if (cara) {
    disc(cx - rx / 3, cy - 5, 2);
    disc(cx + rx / 3, cy - 5, 2);
    int r = imax(3, 9 * rx / 22);
    for (int a = 0; a <= 180; a += 8) {
      float rad = a * PI_F / 180.0f;
      px(cx + (int)lroundf(r * cosf(rad)), cy + 1 + (int)lroundf(r * sinf(rad)));
    }
  } else {
    bx(cx - 1, cy - 9, 3, 18);
    bx(cx - rx / 2, cy - 1, rx, 3);
  }
  u8g2.setDrawColor(1);
}

void monedaIniciar(uint32_t) { monFase = 0; chispasOn = false; }
bool monedaActualizar(uint32_t ahora) {
  if (enter.corto && monFase != 1) {
    monFase = 1;
    monT0 = ahora;
    monCara = (random(2) == 0);
    chispasOn = false;
  }
  if (!frameListo(ahora)) return false;
  u8g2.clearBuffer();
  const int cx = 36, cy = 36;
  if (monFase == 1) {
    uint32_t tt = ahora - monT0;
    if (tt >= 1800) {
      monFase = 2;
      chispasLanzar(cx, cy, ahora);
    } else {
      float u = tt / 1800.0f;
      float ease = 1.0f - (1.0f - u) * (1.0f - u) * (1.0f - u);
      float theta = ease * (6.0f * PI_F + (monCara ? 0.0f : PI_F));
      float c = cosf(theta);
      int rx = imax(3, (int)lroundf(fabsf(c) * 22.0f));
      int yy = cy - (int)lroundf(10.0f * sinf(PI_F * u));
      dibujarMoneda(cx, yy, rx, c >= 0);
    }
  }
  if (monFase == 0) {
    dibujarMoneda(cx, cy, 22, true);
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawUTF8(66, 30, "Tocá el");
    u8g2.drawUTF8(66, 44, "botón");
  } else if (monFase == 2) {
    dibujarMoneda(cx, cy, 22, monCara);
    u8g2.setFont(u8g2_font_helvB18_tf);
    u8g2.drawUTF8(66, 40, monCara ? "CARA" : "CRUZ");
    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.drawUTF8(58, 60, "tocá: otra vez");
    chispasDibujar(ahora);
  }
  u8g2.sendBuffer();
  return false;
}
const App APP_MONEDA = { monedaIniciar, monedaActualizar, nullptr, 60000 };

// ======================= Dado =======================
uint8_t dadoFase = 0;     // 0 listo, 1 girando, 2 resultado
uint8_t dadoVal = 1, dadoMuestra = 1;
uint32_t dadoT0 = 0, dadoTCambio = 0;

void dibujarDado(int cx, int cy, int val, int jx, int jy) {
  int x = cx - 22 + jx, y = cy - 22 + jy;
  u8g2.drawRBox(x, y, 44, 44, 8);
  u8g2.setDrawColor(0);
  const int p[3] = { -12, 0, 12 };
  // cada bit de PIPS es un lugar de la grilla 3x3 (bit = fila*3 + columna)
  static const uint16_t PIPS[7] = { 0, 0x010, 0x101, 0x111, 0x145, 0x155, 0x16D };
  for (int i = 0; i < 9; i++) {
    if (PIPS[val] & (1 << i)) disc(cx + jx + p[i % 3], cy + jy + p[i / 3], 3);
  }
  u8g2.setDrawColor(1);
}

void dadoIniciar(uint32_t) { dadoFase = 0; chispasOn = false; }
bool dadoActualizar(uint32_t ahora) {
  if (enter.corto && dadoFase != 1) {
    dadoFase = 1;
    dadoT0 = ahora;
    dadoTCambio = ahora;
    dadoVal = (uint8_t)(random(6) + 1);
    chispasOn = false;
  }
  if (!frameListo(ahora)) return false;
  u8g2.clearBuffer();
  int jx = 0, jy = 0;
  if (dadoFase == 1) {
    uint32_t tt = ahora - dadoT0;
    if (tt >= 1500) {
      dadoFase = 2;
      dadoMuestra = dadoVal;
      chispasLanzar(64, 28, ahora);
    } else {
      uint32_t intervalo = 60 + tt / 12;   // va frenando
      if (ahora - dadoTCambio >= intervalo) {
        dadoTCambio = ahora;
        uint8_t nuevo;
        do { nuevo = (uint8_t)(random(6) + 1); } while (nuevo == dadoMuestra);
        dadoMuestra = nuevo;
      }
      jx = (int)random(-2, 3);
      jy = (int)random(-2, 3);
    }
  }
  if (dadoFase == 0) dadoMuestra = 1;
  dibujarDado(64, 28, dadoMuestra, jx, jy);
  u8g2.setFont(u8g2_font_6x10_tf);
  textoCentrado(dadoFase == 0 ? "Tocá para tirar" : (dadoFase == 2 ? "Tocá: otra vez" : "..."), 62);
  chispasDibujar(ahora);
  u8g2.sendBuffer();
  return false;
}
const App APP_DADO = { dadoIniciar, dadoActualizar, nullptr, 60000 };

// ======================= Bola mágica 8 =======================
uint8_t bolaFase = 0;     // 0 listo, 1 agitando, 2 respuesta
uint8_t bolaResp = 0;
uint32_t bolaT0 = 0;

void bolaIniciar(uint32_t) { bolaFase = 0; }
bool bolaActualizar(uint32_t ahora) {
  if (enter.corto && bolaFase != 1) {
    bolaFase = 1;
    bolaT0 = ahora;
    bolaResp = (uint8_t)random(N_BOLA8);
  }
  if (bolaFase == 1 && ahora - bolaT0 >= 2000) bolaFase = 2;
  if (!frameListo(ahora)) return false;
  u8g2.clearBuffer();
  if (bolaFase == 2) {
    uint8_t tipo = BOLA8[bolaResp].tipo;
    // ojitos que reaccionan
    if (tipo == 0) {
      for (int e = 0; e < 2; e++) {
        int cx = e ? 74 : 54;
        for (int g = 0; g < 2; g++) u8g2.drawCircle(cx, 13, 5 - g, U8G2_DRAW_UPPER_LEFT | U8G2_DRAW_UPPER_RIGHT);
      }
    } else if (tipo == 1) {
      u8g2.drawCircle(54, 10, 4); u8g2.drawCircle(74, 10, 4);
    } else {
      bx(48, 9, 12, 3); bx(68, 9, 12, 3);
    }
    u8g2.setFont(u8g2_font_6x12_tf);
    textoEnvuelto(BOLA8[bolaResp].texto, 0, 36, 122, 14, true);
  } else {
    int jx = 0, jy = 0;
    if (bolaFase == 1) { jx = (int)random(-3, 4); jy = (int)random(-3, 4); }
    const int cx = 32 + jx, cy = 32 + jy;
    u8g2.drawDisc(cx, cy, 26);
    u8g2.setDrawColor(0);
    u8g2.drawDisc(cx, cy, 12);
    u8g2.setDrawColor(1);
    u8g2.setFont(u8g2_font_helvB14_tf);
    u8g2.drawStr(cx - 5, cy + 6, "8");
    if (bolaFase == 0) {
      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.drawUTF8(66, 24, "Pensá una");
      u8g2.drawUTF8(66, 36, "pregunta y");
      u8g2.drawUTF8(66, 48, "tocá");
    }
  }
  u8g2.sendBuffer();
  return false;
}
const App APP_BOLA = { bolaIniciar, bolaActualizar, nullptr, 60000 };

// ======================= Chiste nerd =======================
uint8_t chFase = 0, chIdx = 0;
uint64_t chUsados = 0;
uint32_t chT0 = 0;

void chisteSiguiente() {
  if (__builtin_popcountll(chUsados) >= N_CHISTES) chUsados = 0;
  uint8_t idx;
  do { idx = (uint8_t)random(N_CHISTES); } while (chUsados & (1ULL << idx));
  chUsados |= (1ULL << idx);
  chIdx = idx;
  chFase = 0;
}
void chisteIniciar(uint32_t) { chisteSiguiente(); }
bool chisteActualizar(uint32_t ahora) {
  if (enter.corto) {
    if (chFase == 0) { chFase = 1; chT0 = ahora; }
    else chisteSiguiente();
  }
  if (!frameListo(ahora)) return false;
  u8g2.clearBuffer();
  if (chFase == 0) {
    u8g2.setFont(u8g2_font_6x10_tf);
    textoEnvuelto(CHISTES[chIdx].pregunta, 2, 12, 124, 12, false);
  } else {
    int s = (((ahora - chT0) / 90) % 2) ? -1 : 1;   // la risita tiembla un ratito
    if (ahora - chT0 > 1500) s = 0;
    for (int e = 0; e < 2; e++) {
      int cx = e ? 74 : 54;
      for (int g = 0; g < 2; g++) u8g2.drawCircle(cx, 13 + s, 5 - g, U8G2_DRAW_UPPER_LEFT | U8G2_DRAW_UPPER_RIGHT);
    }
    u8g2.setFont(u8g2_font_6x12_tf);
    textoEnvuelto(CHISTES[chIdx].remate, 0, 32, 122, 14, true);
  }
  u8g2.sendBuffer();
  return false;
}
const App APP_CHISTE = { chisteIniciar, chisteActualizar, nullptr, 60000 };
