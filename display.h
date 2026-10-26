#pragma once
#include <U8g2lib.h>
#include <Wire.h>
#include <math.h>
#include <string.h>
#include "config.h"

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// ---------- Ayudas numéricas (evitan ambigüedades de min/max/abs) ----------
static inline int imin(int a, int b) { return a < b ? a : b; }
static inline int imax(int a, int b) { return a > b ? a : b; }
static inline int iabs(int a) { return a < 0 ? -a : a; }
static inline int iclamp(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

// ---------- Dibujo recortado ----------
// La librería U8g2 usa coordenadas SIN signo: un número negativo se "da vuelta" y dibuja
// en cualquier lado. Estas funciones recortan a la pantalla y aceptan negativos, así que se
// usan para todo lo que puede salirse por los bordes (barquito, globos, confeti, etc.).
void px(int x, int y) {
  if (x >= 0 && x < 128 && y >= 0 && y < 64) u8g2.drawPixel(x, y);
}
void hl(int x, int y, int w) {
  if (y < 0 || y >= 64 || w <= 0) return;
  if (x < 0) { w += x; x = 0; }
  if (x + w > 128) w = 128 - x;
  if (w > 0) u8g2.drawHLine(x, y, w);
}
void bx(int x, int y, int w, int h) {
  for (int j = 0; j < h; j++) hl(x, y + j, w);
}
void ln(int x0, int y0, int x1, int y1) {
  int dx = iabs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = -iabs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int err = dx + dy;
  for (int n = 0; n < 400; n++) {
    px(x0, y0);
    if (x0 == x1 && y0 == y1) break;
    int e2 = 2 * err;
    if (e2 >= dy) { err += dy; x0 += sx; }
    if (e2 <= dx) { err += dx; y0 += sy; }
  }
}
void disc(int cx, int cy, int r) {
  for (int dy = -r; dy <= r; dy++) {
    int dx = (int)lroundf(sqrtf((float)(r * r - dy * dy)));
    hl(cx - dx, cy + dy, 2 * dx + 1);
  }
}
void elip(int cx, int cy, int rx, int ry) {
  if (ry < 1) return;
  for (int dy = -ry; dy <= ry; dy++) {
    float q = 1.0f - (float)(dy * dy) / (float)(ry * ry);
    int dx = (int)lroundf(rx * sqrtf(q < 0 ? 0 : q));
    hl(cx - dx, cy + dy, 2 * dx + 1);
  }
}
void tri(int x0, int y0, int x1, int y1, int x2, int y2) {
  int ymin = imin(y0, imin(y1, y2)), ymax = imax(y0, imax(y1, y2));
  for (int y = ymin; y <= ymax; y++) {
    int xa = 10000, xb = -10000;
    int ex[3][4] = { {x0, y0, x1, y1}, {x1, y1, x2, y2}, {x2, y2, x0, y0} };
    for (int e = 0; e < 3; e++) {
      int ax = ex[e][0], ay = ex[e][1], bx2 = ex[e][2], by = ex[e][3];
      if (ay == by) { if (y == ay) { xa = imin(xa, imin(ax, bx2)); xb = imax(xb, imax(ax, bx2)); } continue; }
      if ((y < ay && y < by) || (y > ay && y > by)) continue;
      int xx = ax + (bx2 - ax) * (y - ay) / (by - ay);
      xa = imin(xa, xx); xb = imax(xb, xx);
    }
    if (xa <= xb) hl(xa, y, xb - xa + 1);
  }
}

// ---------- Brillo ----------
const uint8_t BRILLOS[] = { 20, 110, 207 };
const char* const NOMBRES_BRILLO[] = { "Brillo: bajo", "Brillo: medio", "Brillo: alto" };
uint8_t nivelBrillo = 1;

void aplicarBrillo()        { u8g2.setContrast(BRILLOS[nivelBrillo]); }
void aplicarBrilloDormido() { u8g2.setContrast(4); }
void aplicarBrilloFiesta()  { u8g2.setContrast(255); }

// ---------- Inicio ----------
void pantallaIniciar() {
  Wire.begin(PIN_SDA, PIN_SCL);
  u8g2.begin();
  u8g2.setBusClock(400000);   // sin esto las animaciones se ven entrecortadas
  u8g2.enableUTF8Print();
  aplicarBrillo();
}

// ---------- Texto ----------
// Texto centrado (UTF-8: soporta tildes y ñ con las fuentes _tf)
void textoCentrado(const char* txt, int y) {
  int x = (128 - u8g2.getUTF8Width(txt)) / 2;
  u8g2.drawUTF8(x, y, txt);
}

// Texto con contorno negro, para que se lea sobre fondos con dibujos
void textoConBorde(int x, int y, const char* txt) {
  u8g2.setDrawColor(0);
  for (int dx = -1; dx <= 1; dx++)
    for (int dy = -1; dy <= 1; dy++)
      if (dx != 0 || dy != 0) u8g2.drawUTF8(x + dx, y + dy, txt);
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(x, y, txt);
}
void textoCentradoBorde(const char* txt, int y) {
  int w = u8g2.getUTF8Width(txt);
  int x = (128 - w) / 2;
  int arriba = u8g2.getAscent() + 2;       // un poquito más de aire por las tildes
  int abajo  = -u8g2.getDescent() + 1;
  u8g2.setDrawColor(0);
  bx(x - 3, y - arriba, w + 6, arriba + abajo + 1);   // cajita negra detrás del texto
  u8g2.setDrawColor(1);
  u8g2.drawUTF8(x, y, txt);
}

// Elige la fuente más grande en la que el texto entra en maxW píxeles
const uint8_t* const FUENTES_GRANDES[] = {
  u8g2_font_helvB18_tf, u8g2_font_helvB14_tf, u8g2_font_helvB10_tf, u8g2_font_7x13_tf
};
void fuenteQueEntra(const char* txt, int maxW) {
  for (uint8_t i = 0; i < 4; i++) {
    u8g2.setFont(FUENTES_GRANDES[i]);
    if (u8g2.getUTF8Width(txt) <= maxW) return;
  }
}

// Fuentes de números grandes (hora): la más grande en la que entra
const uint8_t* const FUENTES_HORA[] = {
  u8g2_font_logisoso42_tn, u8g2_font_logisoso32_tn, u8g2_font_logisoso28_tn
};
uint8_t fuenteHoraQueEntra(const char* txt, int maxW) {   // devuelve el alto aproximado
  const uint8_t alto[] = { 42, 32, 28 };
  for (uint8_t i = 0; i < 3; i++) {
    u8g2.setFont(FUENTES_HORA[i]);
    if (u8g2.getStrWidth(txt) <= maxW || i == 2) return alto[i];
  }
  return 28;
}

// Texto en varias líneas, cortando por palabras. Devuelve la cantidad de líneas.
uint8_t textoEnvuelto(const char* txt, int x, int y, int maxW, int altoLinea, bool centrado) {
  char linea[64]; linea[0] = 0;
  char palabra[40];
  char prueba[110];
  uint8_t n = 0;
  int yy = y;
  const char* p = txt;
  while (*p) {
    uint8_t k = 0;
    while (*p && *p != ' ' && k < sizeof(palabra) - 1) palabra[k++] = *p++;
    palabra[k] = 0;
    while (*p == ' ') p++;
    if (linea[0]) snprintf(prueba, sizeof(prueba), "%s %s", linea, palabra);
    else          snprintf(prueba, sizeof(prueba), "%s", palabra);
    if (!linea[0] || u8g2.getUTF8Width(prueba) <= maxW) {
      strncpy(linea, prueba, sizeof(linea) - 1);
      linea[sizeof(linea) - 1] = 0;
    } else {
      int xx = centrado ? (128 - u8g2.getUTF8Width(linea)) / 2 : x;
      u8g2.drawUTF8(xx, yy, linea);
      yy += altoLinea; n++;
      strncpy(linea, palabra, sizeof(linea) - 1);
      linea[sizeof(linea) - 1] = 0;
    }
  }
  if (linea[0]) {
    int xx = centrado ? (128 - u8g2.getUTF8Width(linea)) / 2 : x;
    u8g2.drawUTF8(xx, yy, linea);
    n++;
  }
  return n;
}

// Limita las animaciones a ~30 cuadros por segundo
uint32_t tFrame = 0;
bool frameListo(uint32_t ahora) {
  if (ahora - tFrame < 30) return false;
  tFrame = ahora;
  return true;
}
