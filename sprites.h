#pragma once
#include "display.h"

const float PI_F = 3.14159265f;

// ---------- Formas básicas ----------
// Corazón relleno: ancho = 4*r, alto ~ 3.6*r
void corazon(int cx, int cy, int r) {
  if (r < 1) r = 1;
  int oy = cy - r / 2;
  u8g2.drawDisc(cx - r, oy, r);
  u8g2.drawDisc(cx + r, oy, r);
  u8g2.drawTriangle(cx - 2 * r, oy, cx + 2 * r, oy, cx, oy + (r * 13) / 5);
}

// Estrella de 5 puntas rellena
void estrella(int cx, int cy, int R, int r, float rot) {
  int px[10], py[10];
  for (int i = 0; i < 10; i++) {
    float a = rot + i * PI_F / 5.0f - PI_F / 2.0f;
    float rad = (i % 2 == 0) ? R : r;
    px[i] = cx + (int)lroundf(rad * cosf(a));
    py[i] = cy + (int)lroundf(rad * sinf(a));
  }
  for (int i = 0; i < 10; i++) {
    int j = (i + 1) % 10;
    u8g2.drawTriangle(cx, cy, px[i], py[i], px[j], py[j]);
  }
}

// Destello en cruz (+)
void destello(int cx, int cy, int s) {
  u8g2.drawHLine(cx - s, cy, 2 * s + 1);
  u8g2.drawVLine(cx, cy - s, 2 * s + 1);
}

// ---------- Clima ----------
enum TipoClima : uint8_t { CL_SOL, CL_PARCIAL, CL_NUBE, CL_LLUVIA, CL_TORMENTA, CL_NIEVE, CL_NIEBLA, CL_LUNA };

uint8_t tipoClimaDeCodigo(int c, bool noche) {
  if (c == 0) return noche ? CL_LUNA : CL_SOL;
  if (c <= 2) return CL_PARCIAL;
  if (c == 3) return CL_NUBE;
  if (c == 45 || c == 48) return CL_NIEBLA;
  if ((c >= 51 && c <= 67) || (c >= 80 && c <= 82)) return CL_LLUVIA;
  if ((c >= 71 && c <= 77) || c == 85 || c == 86) return CL_NIEVE;
  if (c >= 95) return CL_TORMENTA;
  return CL_NUBE;
}

void nube(int cx, int cy, bool borde) {
  if (borde) {   // contorno negro para que se despegue de lo que hay detrás
    u8g2.setDrawColor(0);
    u8g2.drawDisc(cx - 8, cy + 3, 8);
    u8g2.drawDisc(cx, cy - 3, 10);
    u8g2.drawDisc(cx + 8, cy + 3, 8);
    u8g2.drawBox(cx - 16, cy + 3, 32, 8);
    u8g2.setDrawColor(1);
  }
  u8g2.drawDisc(cx - 8, cy + 3, 6);
  u8g2.drawDisc(cx, cy - 3, 8);
  u8g2.drawDisc(cx + 8, cy + 3, 6);
  u8g2.drawBox(cx - 14, cy + 3, 28, 6);
}

void sol(int cx, int cy, int r, uint32_t t) {
  u8g2.drawDisc(cx, cy, r);
  float giro = t / 900.0f;
  for (int i = 0; i < 8; i++) {
    float a = giro + i * PI_F / 4.0f;
    int x0 = cx + (int)lroundf((r + 3) * cosf(a)), y0 = cy + (int)lroundf((r + 3) * sinf(a));
    int x1 = cx + (int)lroundf((r + 6) * cosf(a)), y1 = cy + (int)lroundf((r + 6) * sinf(a));
    u8g2.drawLine(x0, y0, x1, y1);
  }
}

// Ícono de unos 32x32 centrado en (cx, cy). t = tiempo para animarlo.
void iconoClima(int cx, int cy, uint8_t tipo, uint32_t t) {
  switch (tipo) {
    case CL_SOL:
      sol(cx, cy, 8, t);
      break;
    case CL_LUNA:
      u8g2.drawDisc(cx, cy, 11);
      u8g2.setDrawColor(0);
      u8g2.drawDisc(cx + 6, cy - 4, 9);
      u8g2.setDrawColor(1);
      break;
    case CL_PARCIAL:
      sol(cx - 6, cy - 6, 6, t);
      nube(cx + 3, cy + 5, true);
      break;
    case CL_NUBE:
      nube(cx, cy, false);
      break;
    case CL_LLUVIA:
      nube(cx, cy - 6, false);
      for (int i = 0; i < 4; i++) {
        int x = cx - 11 + i * 7;
        int y = cy + 6 + (int)((t / 90 + i * 3) % 9);
        u8g2.drawLine(x, y, x - 2, y + 4);
      }
      break;
    case CL_TORMENTA:
      nube(cx, cy - 6, false);
      u8g2.drawTriangle(cx + 2, cy + 4, cx - 5, cy + 13, cx, cy + 11);
      u8g2.drawTriangle(cx, cy + 9, cx + 5, cy + 9, cx - 2, cy + 16);
      break;
    case CL_NIEVE:
      nube(cx, cy - 6, false);
      for (int i = 0; i < 4; i++) {
        int x = cx - 10 + i * 7;
        int y = cy + 6 + (int)((t / 140 + i * 4) % 10);
        u8g2.drawDisc(x, y, 1);
      }
      break;
    case CL_NIEBLA:
      for (int i = 0; i < 4; i++) {
        int off = (i % 2) * 6 + (int)((t / 400) % 2) * 2;
        u8g2.drawHLine(cx - 14 + off, cy - 9 + i * 6, 20);
      }
      break;
  }
}

// ---------- Flores (para el aniversario) ----------
// x: posición, yBase: suelo, h: altura del tallo, tipo: 0 margarita / 1 tulipán, sway: inclinación en píxeles
void flor(int x, int yBase, int h, int tipo, int sway) {
  if (h < 3) return;
  int hx = x + sway, hy = yBase - h;
  u8g2.drawLine(x, yBase, hx, hy);
  if (h > 8) {   // hojitas
    u8g2.drawLine(x, yBase - h / 3, x - 4, yBase - h / 3 - 3);
    u8g2.drawLine(x, yBase - h / 2, x + 4, yBase - h / 2 - 3);
  }
  if (h < 7) return;
  if (tipo == 0) {   // margarita
    for (int i = 0; i < 6; i++) {
      float a = i * PI_F / 3.0f;
      u8g2.drawDisc(hx + (int)lroundf(4 * cosf(a)), hy + (int)lroundf(4 * sinf(a)), 2);
    }
    u8g2.setDrawColor(0);
    u8g2.drawDisc(hx, hy, 2);
    u8g2.setDrawColor(1);
    u8g2.drawPixel(hx, hy);
  } else {           // tulipán
    u8g2.drawFilledEllipse(hx, hy, 4, 5);
    u8g2.setDrawColor(0);
    u8g2.drawTriangle(hx - 5, hy - 6, hx + 5, hy - 6, hx, hy - 1);
    u8g2.setDrawColor(1);
  }
}

// ---------- Barquito de papel ----------
void barquito(int x, int y) {
  // casco
  tri(x, y, x + 32, y, x + 26, y + 9);
  tri(x, y, x + 26, y + 9, x + 6, y + 9);
  // vela: mitad rellena, mitad solo contorno (papel doblado)
  tri(x + 16, y - 20, x + 3, y - 2, x + 16, y - 2);
  ln(x + 16, y - 20, x + 29, y - 2);
  ln(x + 29, y - 2, x + 16, y - 2);
  ln(x + 16, y - 20, x + 16, y - 2);
}

// ---------- Pantalla de apagado ----------
void dibujarChauchas(uint32_t t) {
  int bote = ((t / 350) % 2) ? 1 : 0;
  u8g2.clearBuffer();
  fuenteQueEntra("Chauchas", 124);
  textoCentrado("Chauchas", 38 + bote);
  u8g2.sendBuffer();
}
