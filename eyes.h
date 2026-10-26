#pragma once
#include "display.h"
#include "sprites.h"

// ---------- Geometría de los ojos ----------
const int OJO_W = 38, OJO_H = 46;
const int CX_IZQ = 36, CX_DER = 92, CY = 32;

enum Expr : uint8_t {
  EX_NEUTRAL, EX_MIRAR, EX_RISITA, EX_GUINIO, EX_SORPRESA, EX_SUENO,
  EX_VERGUENZA, EX_ENAMORADO, EX_ENOJADO, EX_TRISTE, EX_SOSPECHA, EX_MAREO, EX_ESTRELLAS
};

const uint16_t DURACION_EXPR[] = { 0, 2400, 1400, 1200, 1500, 3500, 2600, 2200, 1800, 2600, 2400, 2200, 2000 };
const Expr ELEGIBLES[] = {
  EX_MIRAR, EX_MIRAR, EX_RISITA, EX_GUINIO, EX_SORPRESA, EX_SUENO, EX_VERGUENZA,
  EX_ENAMORADO, EX_ENOJADO, EX_TRISTE, EX_SOSPECHA, EX_MAREO, EX_ESTRELLAS
};
const uint8_t N_ELEGIBLES = sizeof(ELEGIBLES) / sizeof(ELEGIBLES[0]);

// Animaciones por fotogramas clave: { milisegundos, valor }
const int16_t KF_MIRAR[][2]     = { {0,0}, {300,0}, {500,-12}, {1000,-12}, {1300,12}, {1900,12}, {2200,0}, {2400,0} };
const int16_t KF_SORPRESA[][2]  = { {0,22}, {120,27}, {1200,27}, {1500,22} };
const int16_t KF_SUENO[][2]     = { {0,46}, {600,24}, {1100,10}, {1700,24}, {2300,8}, {3000,8}, {3500,46} };
const int16_t KF_DESPERTAR[][2] = { {0,3}, {300,3}, {800,16}, {1100,16}, {1250,5}, {1500,5}, {1900,34}, {2100,46} };
const int16_t KF_VERG_X[][2]    = { {0,0}, {300,-10}, {1100,-10}, {1500,0}, {1800,0}, {2200,-10}, {2600,0} };
const int16_t KF_VERG_Y[][2]    = { {0,0}, {300,5}, {1100,5}, {1500,0}, {1800,0}, {2200,5}, {2600,0} };
const int16_t KF_SOSP_X[][2]    = { {0,0}, {500,-8}, {1400,8}, {2000,0}, {2400,0} };
#define N_KF(a) ((uint8_t)(sizeof(a) / sizeof((a)[0])))

// Interpola linealmente entre fotogramas clave
int keyframe(const int16_t kf[][2], uint8_t n, uint32_t t) {
  if (t <= (uint32_t)kf[0][0]) return kf[0][1];
  for (uint8_t i = 1; i < n; i++) {
    if (t <= (uint32_t)kf[i][0]) {
      int t0 = kf[i - 1][0], t1 = kf[i][0];
      int v0 = kf[i - 1][1], v1 = kf[i][1];
      return v0 + (v1 - v0) * ((int)t - t0) / (t1 - t0);
    }
  }
  return kf[n - 1][1];
}

// ---------- Estado de los ojos ----------
struct EstadoOjos {
  Expr     expr, ultima;
  uint32_t t0, dur;          // inicio y duración de la expresión actual
  uint32_t tProx;            // cuándo arranca la próxima expresión
  uint32_t tParp, tFinParp;  // próximo parpadeo / fin del parpadeo en curso
};
EstadoOjos ojos;

void ojosIniciar(uint32_t ahora) {
  ojos.expr = EX_NEUTRAL;
  ojos.ultima = EX_NEUTRAL;
  ojos.t0 = ahora;
  ojos.dur = 0;
  ojos.tProx = ahora + random(1500, 3500);
  ojos.tParp = ahora + random(1500, 3500);
  ojos.tFinParp = 0;
}

void ojosForzar(Expr e, uint32_t ahora) {
  ojos.expr = e;
  ojos.t0 = ahora;
  ojos.dur = DURACION_EXPR[e];
}

// ---------- Piezas de dibujo ----------
void ojoRect(int cx, int cy, int w, int h) {
  if (h < 2) h = 2;
  int r = imin(10, imin(w, h) / 2);
  u8g2.drawRBox(cx - w / 2, cy - h / 2, w, h, r);
}

// Ojo cerrado feliz: semicírculo superior grueso (^)
void ojoArco(int cx, int cy, int r, int grosor) {
  for (int i = 0; i < grosor; i++) {
    u8g2.drawCircle(cx, cy, r - i, U8G2_DRAW_UPPER_LEFT | U8G2_DRAW_UPPER_RIGHT);
  }
}

int alturaParpadeo(uint32_t ahora) {
  if (ahora >= ojos.tFinParp) return OJO_H;
  int transcurrido = 140 - (int)(ojos.tFinParp - ahora);
  int h = OJO_H * iabs(70 - transcurrido) / 70;
  return imax(4, h);
}

void dibujarZ(uint32_t ahora) {
  u8g2.setFont(u8g2_font_6x10_tf);
  for (int k = 0; k < 3; k++) {
    int fase = (int)((ahora + k * 900) % 2700);
    int zx = 98 + fase / 110;
    int zy = 36 - fase / 90;
    u8g2.drawStr(zx, zy, k == 0 ? "z" : "Z");
  }
}

// ---------- Dibujo de la expresión actual ----------
void ojosDibujar(uint32_t ahora) {
  uint32_t tt = ahora - ojos.t0;
  u8g2.clearBuffer();

  switch (ojos.expr) {
    case EX_NEUTRAL: {
      int h = alturaParpadeo(ahora);
      ojoRect(CX_IZQ, CY, OJO_W, h);
      ojoRect(CX_DER, CY, OJO_W, h);
      break;
    }
    case EX_MIRAR: {
      int dx = keyframe(KF_MIRAR, N_KF(KF_MIRAR), tt);
      int h = alturaParpadeo(ahora);
      ojoRect(CX_IZQ + dx, CY, OJO_W, h);
      ojoRect(CX_DER + dx, CY, OJO_W, h);
      break;
    }
    case EX_RISITA: {
      int s = ((tt / 70) % 2) ? -2 : 2;   // temblor
      ojoArco(CX_IZQ, CY + 10 + s, 17, 6);
      ojoArco(CX_DER, CY + 10 + s, 17, 6);
      break;
    }
    case EX_GUINIO: {
      ojoRect(CX_IZQ, CY, OJO_W, OJO_H);
      if (tt > 300 && tt < 900) ojoArco(CX_DER, CY + 8, 17, 6);
      else ojoRect(CX_DER, CY, OJO_W, OJO_H);
      break;
    }
    case EX_SORPRESA: {
      int r = keyframe(KF_SORPRESA, N_KF(KF_SORPRESA), tt);
      int temblor = (tt > 200 && tt < 1200) ? (int)((tt / 60) % 2) : 0;
      u8g2.drawDisc(34, CY, r);
      u8g2.drawDisc(94, CY, r);
      u8g2.setDrawColor(0);
      u8g2.drawDisc(34 + temblor, CY, 6);   // pupilas
      u8g2.drawDisc(94 + temblor, CY, 6);
      u8g2.setDrawColor(1);
      break;
    }
    case EX_SUENO: {
      int h = keyframe(KF_SUENO, N_KF(KF_SUENO), tt);
      int cy = CY + (OJO_H - h) / 2;      // el párpado baja, la base queda fija
      ojoRect(CX_IZQ, cy, OJO_W, h);
      ojoRect(CX_DER, cy, OJO_W, h);
      if (tt > 1100 && tt < 3000) dibujarZ(ahora);
      break;
    }
    case EX_VERGUENZA: {
      int dx = keyframe(KF_VERG_X, N_KF(KF_VERG_X), tt);
      int dy = keyframe(KF_VERG_Y, N_KF(KF_VERG_Y), tt);
      ojoRect(CX_IZQ + dx, CY + dy, OJO_W, 26);
      ojoRect(CX_DER + dx, CY + dy, OJO_W, 26);
      if (tt > 300) {                      // mejillas con rayitas
        for (int i = 0; i < 3; i++) {
          u8g2.drawLine(14 + i * 6, 60, 18 + i * 6, 53);
          u8g2.drawLine(96 + i * 6, 60, 100 + i * 6, 53);
        }
      }
      break;
    }
    case EX_ENAMORADO: {
      int r = 10 + (int)lroundf(2.5f * sinf(tt / 110.0f));   // late
      corazon(CX_IZQ, CY, r);
      corazon(CX_DER, CY, r);
      for (int k = 0; k < 3; k++) {                           // corazoncitos que suben
        int fase = (int)((tt + k * 500) % 1500);
        int yy = 46 - fase / 40;
        if (yy > 8) corazon(64 + (k - 1) * 6, yy, 2);
      }
      break;
    }
    case EX_ENOJADO: {
      int tem = ((tt / 60) % 2) ? 1 : -1;
      int h = 38;
      int xL = CX_IZQ + tem, xR = CX_DER - tem;
      ojoRect(xL, CY + 4, OJO_W, h);
      ojoRect(xR, CY + 4, OJO_W, h);
      int y0 = CY + 4 - h / 2;
      u8g2.setDrawColor(0);                // párpados inclinados hacia adentro
      u8g2.drawTriangle(xL - OJO_W / 2 - 1, y0 - 1, xL + OJO_W / 2 + 1, y0 - 1, xL + OJO_W / 2 + 1, y0 + 17);
      u8g2.drawTriangle(xR + OJO_W / 2 + 1, y0 - 1, xR - OJO_W / 2 - 1, y0 - 1, xR - OJO_W / 2 - 1, y0 + 17);
      u8g2.setDrawColor(1);
      break;
    }
    case EX_TRISTE: {
      int h = 34;
      ojoRect(CX_IZQ, CY + 4, OJO_W, h);
      ojoRect(CX_DER, CY + 4, OJO_W, h);
      int y0 = CY + 4 - h / 2;
      u8g2.setDrawColor(0);                // párpados caídos hacia afuera
      u8g2.drawTriangle(CX_IZQ - OJO_W / 2 - 1, y0 - 1, CX_IZQ + OJO_W / 2 + 1, y0 - 1, CX_IZQ - OJO_W / 2 - 1, y0 + 17);
      u8g2.drawTriangle(CX_DER + OJO_W / 2 + 1, y0 - 1, CX_DER - OJO_W / 2 - 1, y0 - 1, CX_DER + OJO_W / 2 + 1, y0 + 17);
      u8g2.setDrawColor(1);
      if (tt > 500) {                       // lágrima
        int ty = 55 + (int)((tt / 30) % 8);
        u8g2.drawDisc(CX_DER + 10, ty, 2);
        u8g2.drawTriangle(CX_DER + 8, ty, CX_DER + 12, ty, CX_DER + 10, ty - 5);
      }
      break;
    }
    case EX_SOSPECHA: {
      int dx = keyframe(KF_SOSP_X, N_KF(KF_SOSP_X), tt);
      ojoRect(CX_IZQ + dx, CY + 8, OJO_W, 12);          // ojo entrecerrado
      ojoRect(CX_DER + dx, CY - 2, OJO_W, 34);          // ojo normal
      u8g2.drawBox(CX_IZQ + dx - OJO_W / 2, CY - 6, OJO_W, 3);    // ceja baja
      u8g2.drawBox(CX_DER + dx - OJO_W / 2, CY - 24, OJO_W, 3);   // ceja alta
      break;
    }
    case EX_MAREO: {
      float fase = tt / 110.0f;
      for (int e = 0; e < 2; e++) {
        int cx = e ? CX_DER : CX_IZQ;
        int px = cx, py = CY;
        for (float a = 0.2f; a < 13.0f; a += 0.35f) {
          float rad = a * 1.45f;
          int x = cx + (int)lroundf(rad * cosf(a + fase));
          int y = CY + (int)lroundf(rad * sinf(a + fase));
          u8g2.drawLine(px, py, x, y);
          u8g2.drawLine(px + 1, py, x + 1, y);
          px = x; py = y;
        }
      }
      break;
    }
    case EX_ESTRELLAS: {
      int R = 18 + (int)lroundf(3 * sinf(tt / 90.0f));
      float rot = tt / 700.0f;
      estrella(CX_IZQ, CY, R, R * 45 / 100, rot);
      estrella(CX_DER, CY, R, R * 45 / 100, -rot);
      if ((tt / 150) % 3 == 0) destello(6, 8, 3);
      if ((tt / 150) % 3 == 1) destello(122, 10, 3);
      if ((tt / 150) % 3 == 2) destello(64, 58, 3);
      break;
    }
  }
}

// Avanza la lógica (cambio de expresión, parpadeos) y dibuja un cuadro
void ojosActualizar(uint32_t ahora) {
  if (!frameListo(ahora)) return;

  // ¿terminó la expresión actual?
  if (ojos.expr != EX_NEUTRAL && ahora - ojos.t0 >= ojos.dur) {
    ojos.expr = EX_NEUTRAL;
    ojos.tProx = ahora + random(2000, 5000);
  }
  // ¿toca una nueva? (sin repetir la anterior)
  if (ojos.expr == EX_NEUTRAL && ahora >= ojos.tProx) {
    Expr e;
    do { e = ELEGIBLES[random(N_ELEGIBLES)]; } while (e == ojos.ultima);
    ojos.ultima = e;
    ojosForzar(e, ahora);
  }
  // parpadeo
  if ((ojos.expr == EX_NEUTRAL || ojos.expr == EX_MIRAR) && ahora >= ojos.tParp) {
    ojos.tFinParp = ahora + 140;
    ojos.tParp = ahora + random(2500, 6000);
  }

  ojosDibujar(ahora);
  u8g2.sendBuffer();
}

// ---------- Despertar: devuelve true cuando terminó ----------
bool ojosDespertar(uint32_t t) {
  int h = keyframe(KF_DESPERTAR, N_KF(KF_DESPERTAR), t);
  int cy = CY + (OJO_H - h) / 2;
  u8g2.clearBuffer();
  ojoRect(CX_IZQ, cy, OJO_W, h);
  ojoRect(CX_DER, cy, OJO_W, h);
  u8g2.sendBuffer();
  return t >= 2100;
}

// ---------- Dormido: ojos cerrados y "zzz" ----------
void ojosDormido(uint32_t ahora) {
  u8g2.clearBuffer();
  ojoRect(CX_IZQ, 53, OJO_W, 4);
  ojoRect(CX_DER, 53, OJO_W, 4);
  dibujarZ(ahora);
  u8g2.sendBuffer();
}
