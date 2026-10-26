#pragma once
#include "tools.h"
#include "storage.h"

// Controles con un solo botón:
//   Dino y Flappy: cada toque salta / aletea.   Snake: cada toque gira a la derecha.
//   Mantener el botón 1 s vuelve al menú.

enum FaseJ : uint8_t { J_ESPERA, J_CORRE, J_PIERDE };

void juegoMensaje(const char* l1, const char* l2, int y) {
  u8g2.setFont(u8g2_font_6x10_tf);
  if (l1) textoCentradoBorde(l1, y);
  if (l2) textoCentradoBorde(l2, y + 12);
}

void juegoHUD(int puntaje, uint16_t record) {
  char b[24];
  u8g2.setFont(u8g2_font_5x7_tf);
  snprintf(b, sizeof(b), "%05d", puntaje);
  u8g2.drawStr(98, 7, b);
  if (record > 0) {
    snprintf(b, sizeof(b), "HI %05d", record);
    u8g2.drawStr(52, 7, b);
  }
}

// ============================ DINO ============================
const int D_SUELO = 56, D_X = 14, D_W = 10, D_H = 12;
const float D_GRAV = 0.8f, D_SALTO = 7.5f;
struct Cactus { float x; uint8_t tallos, w, h; bool activo; };
Cactus dCact[3];
float dAltura = 0, dVelY = 0, dVel = 3.0f, dDist = 0, dHasta = 60;
int dPuntaje = 0;
uint8_t dFase = J_ESPERA, dFrame = 0;
bool dSalto = false, dRecordNuevo = false;
uint16_t dRec = 0;
uint32_t dTPierde = 0;

void dinoReset() {
  dAltura = 0; dVelY = 0; dVel = 3.0f; dDist = 0; dPuntaje = 0; dHasta = 60;
  dSalto = false; dRecordNuevo = false;
  for (uint8_t i = 0; i < 3; i++) dCact[i].activo = false;
}
void dinoIniciar(uint32_t) { dFase = J_ESPERA; dRec = leerU16("rDino"); dinoReset(); }

bool dinoActualizar(uint32_t ahora) {
  if (algunaPresion()) {
    if (dFase == J_CORRE) dSalto = true;
    else if (dFase == J_ESPERA || (dFase == J_PIERDE && ahora - dTPierde > 500)) { dinoReset(); dFase = J_CORRE; }
  }
  if (!frameListo(ahora)) return false;
  dFrame++;

  if (dFase == J_CORRE) {
    if (dSalto && dAltura == 0) dVelY = D_SALTO;
    dSalto = false;
    if (dAltura > 0 || dVelY > 0) {
      dAltura += dVelY;
      dVelY -= D_GRAV;
      if (dAltura <= 0) { dAltura = 0; dVelY = 0; }
    }
    dDist += dVel;
    dPuntaje = (int)(dDist / 8);
    dVel = fminf(7.0f, 3.0f + dPuntaje / 150.0f);

    dHasta -= dVel;
    if (dHasta <= 0) {
      for (uint8_t i = 0; i < 3; i++) {
        if (!dCact[i].activo) {
          dCact[i].tallos = (uint8_t)random(1, 4);
          dCact[i].w = dCact[i].tallos * 5 - 2;
          dCact[i].h = (uint8_t)random(10, 19);
          dCact[i].x = 128;
          dCact[i].activo = true;
          break;
        }
      }
      dHasta = random(55, 120) + dVel * 12;
    }
    int top = D_SUELO - D_H - (int)dAltura;
    for (uint8_t i = 0; i < 3; i++) {
      if (!dCact[i].activo) continue;
      dCact[i].x -= dVel;
      if (dCact[i].x + dCact[i].w < 0) { dCact[i].activo = false; continue; }
      bool choqueX = D_X + D_W - 2 > dCact[i].x && D_X + 2 < dCact[i].x + dCact[i].w;
      bool choqueY = top + D_H - 2 > D_SUELO - dCact[i].h;
      if (choqueX && choqueY) {
        dFase = J_PIERDE;
        dTPierde = ahora;
        dRecordNuevo = registrarRecord("rDino", (uint16_t)dPuntaje);
        if (dRecordNuevo) dRec = (uint16_t)dPuntaje;
      }
    }
  }

  u8g2.clearBuffer();
  u8g2.drawHLine(0, D_SUELO, 128);
  int top = D_SUELO - D_H - (int)dAltura;
  u8g2.drawRBox(D_X, top, D_W, 10, 2);
  u8g2.setDrawColor(0);
  u8g2.drawBox(D_X + 6, top + 2, 2, 3);     // ojo
  u8g2.setDrawColor(1);
  bool quieto = dAltura > 0 || dFase != J_CORRE;
  bool paso = (dFrame / 4) % 2;
  u8g2.drawBox(D_X + 1, top + 10, 3, (quieto || !paso) ? 2 : 1);
  u8g2.drawBox(D_X + 6, top + 10, 3, (quieto || paso) ? 2 : 1);
  for (uint8_t i = 0; i < 3; i++) {
    if (!dCact[i].activo) continue;
    for (uint8_t k = 0; k < dCact[i].tallos; k++) {
      int h = dCact[i].h - (k % 2) * 4;
      bx((int)dCact[i].x + k * 5, D_SUELO - h, 3, h);
    }
  }
  juegoHUD(dPuntaje, dRec);
  if (dFase == J_ESPERA) juegoMensaje("Tocá para jugar", "mantené para salir", 28);
  else if (dFase == J_PIERDE) juegoMensaje(dRecordNuevo ? "¡NUEVO RÉCORD!" : "GAME OVER", "Tocá para reintentar", 26);
  u8g2.sendBuffer();
  return false;
}
const App APP_DINO = { dinoIniciar, dinoActualizar, nullptr, 0 };

// ============================ FLAPPY ============================
const float F_GRAV = 0.36f, F_FLAP = -3.1f, F_VEL = 1.7f;
const int F_GAP = 26, F_X = 28, F_ANCHO = 12;
struct Tubo { float x; uint8_t gapY; bool activo, contado; };
Tubo fTubos[3];
float fY = 32, fVy = 0, fHasta = 40;
int fPuntaje = 0;
uint8_t fFase = J_ESPERA;
bool fAleteo = false, fRecordNuevo = false;
uint16_t fRec = 0;
uint32_t fTPierde = 0;

void flappyReset() {
  fY = 30; fVy = 0; fHasta = 50; fPuntaje = 0; fAleteo = false; fRecordNuevo = false;
  for (uint8_t i = 0; i < 3; i++) fTubos[i].activo = false;
}
void flappyIniciar(uint32_t) { fFase = J_ESPERA; fRec = leerU16("rFlap"); flappyReset(); }

bool flappyActualizar(uint32_t ahora) {
  if (algunaPresion()) {
    if (fFase == J_CORRE) fAleteo = true;
    else if (fFase == J_ESPERA || (fFase == J_PIERDE && ahora - fTPierde > 500)) { flappyReset(); fFase = J_CORRE; fAleteo = true; }
  }
  if (!frameListo(ahora)) return false;

  if (fFase == J_CORRE) {
    if (fAleteo) fVy = F_FLAP;
    fAleteo = false;
    fVy = fminf(4.0f, fVy + F_GRAV);
    fY += fVy;

    fHasta -= F_VEL;
    if (fHasta <= 0) {
      for (uint8_t i = 0; i < 3; i++) {
        if (!fTubos[i].activo) {
          fTubos[i].x = 128;
          fTubos[i].gapY = (uint8_t)random(8, 64 - 8 - F_GAP + 1);
          fTubos[i].activo = true;
          fTubos[i].contado = false;
          break;
        }
      }
      fHasta = 58;
    }
    bool choque = (fY - 3 < 0) || (fY + 3 > 63);
    for (uint8_t i = 0; i < 3; i++) {
      if (!fTubos[i].activo) continue;
      fTubos[i].x -= F_VEL;
      if (fTubos[i].x + F_ANCHO < 0) { fTubos[i].activo = false; continue; }
      if (!fTubos[i].contado && fTubos[i].x + F_ANCHO < F_X - 3) { fTubos[i].contado = true; fPuntaje++; }
      bool enX = F_X + 3 > fTubos[i].x && F_X - 3 < fTubos[i].x + F_ANCHO;
      if (enX && (fY - 3 < fTubos[i].gapY || fY + 3 > fTubos[i].gapY + F_GAP)) choque = true;
    }
    if (choque) {
      fFase = J_PIERDE;
      fTPierde = ahora;
      fRecordNuevo = registrarRecord("rFlap", (uint16_t)fPuntaje);
      if (fRecordNuevo) fRec = (uint16_t)fPuntaje;
    }
  }

  u8g2.clearBuffer();
  for (uint8_t i = 0; i < 3; i++) {
    if (!fTubos[i].activo) continue;
    int tx = (int)fTubos[i].x, g = fTubos[i].gapY;
    bx(tx, 0, F_ANCHO, g);                      // tubo de arriba
    bx(tx, g + F_GAP, F_ANCHO, 64 - g - F_GAP); // tubo de abajo
    u8g2.setDrawColor(0);
    bx(tx + 3, 0, F_ANCHO - 6, g - 5);          // hueco interior para que se vea hueco
    bx(tx + 3, g + F_GAP + 5, F_ANCHO - 6, 64);
    u8g2.setDrawColor(1);
    bx(tx - 1, g - 4, F_ANCHO + 2, 4);          // bordes
    bx(tx - 1, g + F_GAP, F_ANCHO + 2, 4);
  }
  int by = (int)fY;
  disc(F_X, by, 4);
  u8g2.setDrawColor(0);
  px(F_X + 1, by - 1);
  px(F_X + 2, by - 1);
  u8g2.setDrawColor(1);
  tri(F_X + 4, by, F_X + 4, by + 2, F_X + 7, by + 1);   // pico
  juegoHUD(fPuntaje, fRec);
  if (fFase == J_ESPERA) juegoMensaje("Tocá para aletear", "mantené para salir", 28);
  else if (fFase == J_PIERDE) juegoMensaje(fRecordNuevo ? "¡NUEVO RÉCORD!" : "GAME OVER", "Tocá para reintentar", 26);
  u8g2.sendBuffer();
  return false;
}
const App APP_FLAPPY = { flappyIniciar, flappyActualizar, nullptr, 0 };

// ============================ SNAKE ============================
const int S_CW = 4, S_COLS = 32, S_ROWS = 14, S_OY = 8, S_MAX = S_COLS * S_ROWS;
uint8_t sxs[S_MAX], sys[S_MAX];
int sLen = 3, sPuntaje = 0;
uint8_t sDir = 1, sComX = 0, sComY = 0, sFase = J_ESPERA, sFrames = 0, sPeriodo = 8;
int8_t sGiro = 0;
bool sRecordNuevo = false;
uint16_t sRec = 0;
uint32_t sTPierde = 0;

void snakeComida() {
  for (int intentos = 0; intentos < 200; intentos++) {
    uint8_t x = (uint8_t)random(S_COLS), y = (uint8_t)random(S_ROWS);
    bool libre = true;
    for (int i = 0; i < sLen; i++) if (sxs[i] == x && sys[i] == y) { libre = false; break; }
    if (libre) { sComX = x; sComY = y; return; }
  }
}
void snakeReset() {
  sLen = 3; sPuntaje = 0; sDir = 1; sGiro = 0; sFrames = 0; sPeriodo = 8; sRecordNuevo = false;
  for (int i = 0; i < sLen; i++) { sxs[i] = (uint8_t)(S_COLS / 2 - i); sys[i] = S_ROWS / 2; }
  snakeComida();
}
void snakeIniciar(uint32_t) { sFase = J_ESPERA; sRec = leerU16("rSnake"); snakeReset(); }

bool snakeActualizar(uint32_t ahora) {
  if (sFase == J_CORRE) {
    if (enter.press) sGiro = 1;      // cada toque gira a la derecha
  } else if (algunaPresion() && (sFase == J_ESPERA || ahora - sTPierde > 500)) {
    snakeReset();
    sFase = J_CORRE;
  }
  if (!frameListo(ahora)) return false;

  if (sFase == J_CORRE) {
    sFrames++;
    if (sFrames >= sPeriodo) {
      sFrames = 0;
      sDir = (uint8_t)((sDir + sGiro + 4) % 4);
      sGiro = 0;
      int nx = sxs[0], ny = sys[0];
      if (sDir == 0) ny--; else if (sDir == 1) nx++; else if (sDir == 2) ny++; else nx--;
      nx = (nx + S_COLS) % S_COLS;     // las paredes se traspasan
      ny = (ny + S_ROWS) % S_ROWS;
      bool come = (nx == sComX && ny == sComY);
      bool choca = false;
      int limite = come ? sLen : sLen - 1;   // la cola se mueve si no come
      for (int i = 0; i < limite; i++) if (sxs[i] == nx && sys[i] == ny) { choca = true; break; }
      if (choca) {
        sFase = J_PIERDE;
        sTPierde = ahora;
        sRecordNuevo = registrarRecord("rSnake", (uint16_t)sPuntaje);
        if (sRecordNuevo) sRec = (uint16_t)sPuntaje;
      } else {
        if (come && sLen < S_MAX) sLen++;
        for (int i = sLen - 1; i > 0; i--) { sxs[i] = sxs[i - 1]; sys[i] = sys[i - 1]; }
        sxs[0] = (uint8_t)nx; sys[0] = (uint8_t)ny;
        if (come) {
          sPuntaje++;
          sPeriodo = (uint8_t)imax(3, 8 - sPuntaje / 4);
          snakeComida();
        }
      }
    }
  }

  u8g2.clearBuffer();
  for (int i = sLen - 1; i >= 0; i--) {
    int x = sxs[i] * S_CW, y = S_OY + sys[i] * S_CW;
    if (i == 0) u8g2.drawBox(x, y, 4, 4); else u8g2.drawBox(x, y, 3, 3);
  }
  if ((ahora / 200) % 2 == 0 || sFase != J_CORRE) {
    int fx = sComX * S_CW, fy = S_OY + sComY * S_CW;
    u8g2.drawFrame(fx, fy, 4, 4);
  }
  juegoHUD(sPuntaje, sRec);
  if (sFase == J_ESPERA) juegoMensaje("Cada toque gira", "a la derecha", 26);
  else if (sFase == J_PIERDE) juegoMensaje(sRecordNuevo ? "¡NUEVO RÉCORD!" : "GAME OVER", "Tocá para reintentar", 26);
  u8g2.sendBuffer();
  return false;
}
const App APP_SNAKE = { snakeIniciar, snakeActualizar, nullptr, 0 };
