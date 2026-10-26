#pragma once
#include "display.h"
#include "sprites.h"
#include "gifs.h"

// ============================ Pantallas especiales ============================
// Algunas duran un tiempo fijo; las de festejo se quedan hasta apretar un botón.
enum Especial : uint8_t {
  SEC_NINGUNO, SEC_CUMPLE, SEC_ANIV, SEC_BARCO, SEC_GIF, SEC_CUTI, SEC_DEV, SEC_HORA_PUNTO, SEC_ALARMA
};

// 0 = hasta que se aprete un botón
uint32_t especialDuracion(uint8_t e) {
  switch (e) {
    case SEC_BARCO: return 9000;
    case SEC_GIF: return 8000;
    case SEC_CUTI: return 7000;
    case SEC_DEV: return 8000;
    case SEC_HORA_PUNTO: return 4000;
    default: return 0;   // cumpleaños, aniversario y alarma
  }
}

// ---------- Golpecitos secretos (desde los ojos, con el único botón) ----------
// Se tocan grupos de golpecitos separados por una pausa corta (~1 segundo):
//   Barquito:        2 golpecitos, pausa, 1
//   GIF al azar:     1 golpecito,  pausa, 2
//   Te quiero cuti:  1, pausa, 4, pausa, 3   (1-4-3, el código de "I love you")
//   Dev ginius Dev:  3, pausa, 3
const uint8_t G_BARCO[] = { 2, 1 };
const uint8_t G_GIF[]   = { 1, 2 };
const uint8_t G_CUTI[]  = { 1, 4, 3 };
const uint8_t G_DEV[]   = { 3, 3 };
struct Patron { const uint8_t* g; uint8_t n; uint8_t esp; };
const Patron PATRONES[] = {
  { G_BARCO, 2, SEC_BARCO }, { G_GIF, 2, SEC_GIF }, { G_CUTI, 3, SEC_CUTI }, { G_DEV, 2, SEC_DEV }
};
const uint8_t N_PATRONES = 4;

uint8_t golGrupos[8];
uint8_t golN = 0, golCuenta = 0;
uint32_t golTUlt = 0;

void golpesReiniciar() { golN = 0; golCuenta = 0; }

// ¿los grupos g[0..n-1] son el comienzo de algún patrón (y todavía faltan grupos)?
bool golEsComienzo(const uint8_t* g, uint8_t n) {
  for (uint8_t i = 0; i < N_PATRONES; i++)
    if (PATRONES[i].n > n && memcmp(PATRONES[i].g, g, n) == 0) return true;
  return false;
}

// Llamar en cada toque corto
void golpesToque(uint32_t ahora) {
  if ((golN > 0 || golCuenta > 0) && ahora - golTUlt > GOLPES_REINICIO_MS) golpesReiniciar();
  if (golCuenta > 0 && ahora - golTUlt > GOLPES_PAUSA_MS) {   // la pausa cerró el grupo anterior
    if (golN < 8) golGrupos[golN++] = golCuenta;
    golCuenta = 0;
  }
  golCuenta++;
  golTUlt = ahora;
}

// Llamar seguido: devuelve la pantalla secreta cuando se completa un patrón
uint8_t golpesRevisar(uint32_t ahora) {
  if (golN > 0 && golCuenta == 0 && ahora - golTUlt > GOLPES_REINICIO_MS) golpesReiniciar();
  if (golCuenta == 0) return SEC_NINGUNO;
  if (ahora - golTUlt < GOLPES_PAUSA_MS) return SEC_NINGUNO;      // todavía puede seguir el grupo
  // pasó la pausa: se cierra el grupo y se compara
  uint8_t g[9];
  uint8_t n = golN;
  for (uint8_t i = 0; i < n; i++) g[i] = golGrupos[i];
  g[n++] = golCuenta;
  for (uint8_t i = 0; i < N_PATRONES; i++) {
    if (PATRONES[i].n == n && memcmp(PATRONES[i].g, g, n) == 0) {
      golpesReiniciar();
      return PATRONES[i].esp;
    }
  }
  if (golEsComienzo(g, n) && golN < 8) { golGrupos[golN++] = golCuenta; golCuenta = 0; }   // espera más grupos
  else golpesReiniciar();                                                                   // no lleva a ningún secreto
  return SEC_NINGUNO;
}

// ============================ Fondos animados ============================
// ---------- Confeti ----------
struct Papelito { float x, y, vy, fase; };
Papelito papeles[28];
void confetiIniciar() {
  for (uint8_t i = 0; i < 28; i++) {
    papeles[i].x = (float)random(0, 128);
    papeles[i].y = (float)random(0, 64);
    papeles[i].vy = random(6, 16) / 10.0f;
    papeles[i].fase = random(0, 628) / 100.0f;
  }
}
void confetiDibujar(uint32_t ahora) {
  for (uint8_t i = 0; i < 28; i++) {
    papeles[i].y += papeles[i].vy;
    if (papeles[i].y > 66) { papeles[i].y = -2; papeles[i].x = (float)random(0, 128); }
    int x = (int)(papeles[i].x + 3.0f * sinf(ahora / 300.0f + papeles[i].fase));
    int y = (int)papeles[i].y;
    if (i % 3 == 0) bx(x, y, 2, 2);
    else if (i % 3 == 1) ln(x, y, x + 2, y + 1);
    else px(x, y);
  }
}

// ---------- Globos ----------
struct Globo { float x, y, vy; bool lleno; };
Globo globos[5];
void globosIniciar() {
  for (uint8_t i = 0; i < 5; i++) {
    globos[i].x = 14.0f + i * 25.0f;
    globos[i].y = (float)random(20, 90);
    globos[i].vy = random(4, 9) / 10.0f;
    globos[i].lleno = (i % 2 == 0);
  }
}
void globosDibujar() {
  for (uint8_t i = 0; i < 5; i++) {
    globos[i].y -= globos[i].vy;
    if (globos[i].y < -22) globos[i].y = 86;
    int x = (int)globos[i].x, y = (int)globos[i].y;
    if (globos[i].lleno) elip(x, y, 6, 8);
    else {
      elip(x, y, 6, 8);
      u8g2.setDrawColor(0);
      elip(x, y, 4, 6);
      u8g2.setDrawColor(1);
    }
    tri(x - 2, y + 9, x + 2, y + 9, x, y + 11);
    ln(x, y + 11, x + 2, y + 17);
  }
}

// ---------- Fuegos artificiales ----------
struct Fuego { int x, y; uint32_t t0; };
Fuego fuego = { 64, 24, 0 };
void fuegoDibujar(uint32_t ahora) {
  uint32_t t = ahora - fuego.t0;
  if (t > 1700) {   // nuevo estallido
    fuego.x = (int)random(26, 102);
    fuego.y = (int)random(16, 34);
    fuego.t0 = ahora;
    t = 0;
  }
  if (t > 900) return;
  int r = (int)(t / 55);
  if (r > 15) r = 15;
  for (int i = 0; i < 12; i++) {
    float a = i * PI_F / 6.0f;
    int x1 = fuego.x + (int)lroundf(r * cosf(a)), y1 = fuego.y + (int)lroundf(r * sinf(a));
    if (t < 600) {
      int r0 = imax(0, r - 5);
      ln(fuego.x + (int)lroundf(r0 * cosf(a)), fuego.y + (int)lroundf(r0 * sinf(a)), x1, y1);
    } else {
      px(x1, y1);
    }
  }
}

// ---------- Corazones que suben ----------
struct CorFlota { float x, y, vy, fase; uint8_t r; };
CorFlota flotas[9];
void flotasIniciar() {
  for (uint8_t i = 0; i < 9; i++) {
    flotas[i].x = (float)random(10, 118);
    flotas[i].y = (float)random(8, 80);
    flotas[i].vy = random(5, 13) / 10.0f;
    flotas[i].fase = random(0, 628) / 100.0f;
    flotas[i].r = (uint8_t)random(2, 5);
  }
}
void flotasDibujar(uint32_t ahora, uint8_t cantidad) {
  for (uint8_t i = 0; i < cantidad && i < 9; i++) {
    flotas[i].y -= flotas[i].vy;
    if (flotas[i].y < 9) { flotas[i].y = 72; flotas[i].x = (float)random(10, 118); }
    int x = (int)(flotas[i].x + 4.0f * sinf(ahora / 500.0f + flotas[i].fase));
    x = iclamp(x, 10, 118);
    int y = (int)flotas[i].y;
    if (y < 62) corazon(x, y, flotas[i].r);
  }
}

// ---------- Estrellitas que titilan ----------
void estrellitasDibujar(uint32_t ahora) {
  const int pos[8][2] = { {8, 10}, {118, 14}, {20, 52}, {108, 54}, {6, 32}, {122, 34}, {44, 8}, {84, 58} };
  for (uint8_t i = 0; i < 8; i++) {
    uint32_t f = (ahora / 180 + i * 3) % 6;
    if (f == 0) px(pos[i][0], pos[i][1]);
    else if (f < 4) destello(pos[i][0], pos[i][1], f < 3 ? 2 : 1);
  }
}

// ---------- Lluvia de código ----------
const char CODIGO[] = "01{}<>/;=[]()#&*";
struct Columna { int y; uint8_t vel; uint8_t sem; };
Columna cols[16];
void codigoIniciar() {
  for (uint8_t i = 0; i < 16; i++) {
    cols[i].y = (int)random(-40, 60);
    cols[i].vel = (uint8_t)random(2, 5);
    cols[i].sem = (uint8_t)random(0, 255);
  }
}
void codigoDibujar(uint32_t ahora) {
  u8g2.setFont(u8g2_font_4x6_tf);
  for (uint8_t i = 0; i < 16; i++) {
    cols[i].y += cols[i].vel;
    if (cols[i].y > 80) { cols[i].y = (int)random(-30, 0); cols[i].vel = (uint8_t)random(2, 5); cols[i].sem = (uint8_t)random(0, 255); }
    for (int k = 0; k < 5; k++) {
      int y = cols[i].y - k * 7;
      if (y < 6 || y > 63) continue;
      char c[2] = { CODIGO[(cols[i].sem + k * 5 + (ahora / (140 + k * 40))) % (sizeof(CODIGO) - 1)], 0 };
      if (k == 0 || (ahora / 90 + i + k) % 5 != 0) u8g2.drawStr(i * 8 + 2, y, c);
    }
  }
}

// ============================ Escenas ============================
uint8_t gifElegido = 0;

void especialIniciar(uint8_t e, uint32_t ahora) {
  (void)ahora;
  switch (e) {
    case SEC_CUMPLE: confetiIniciar(); globosIniciar(); fuego.t0 = ahora; break;
    case SEC_ANIV:   flotasIniciar(); break;
    case SEC_CUTI:   flotasIniciar(); break;
    case SEC_DEV:    codigoIniciar(); break;
    case SEC_GIF:    gifElegido = (uint8_t)random(N_GIFS); break;
    default: break;
  }
}

void escenaCumple(uint32_t ahora, uint32_t tt) {
  confetiDibujar(ahora);
  globosDibujar();
  fuegoDibujar(ahora);
  int s = ((tt / 450) % 2) ? 1 : 0;   // el texto "salta" un poquito
  u8g2.setFont(u8g2_font_helvB18_tf);
  textoCentradoBorde("Feliz", 28 - s);
  fuenteQueEntra("Cumpleaños", 124);
  textoCentradoBorde("Cumpleaños", 54 - s);
}

void escenaAniv(uint32_t ahora, uint32_t tt, int anios) {
  flotasDibujar(ahora, 9);
  // flores que crecen desde abajo y después se mecen
  const int xs[6] = { 8, 24, 40, 88, 104, 120 };
  for (int i = 0; i < 6; i++) {
    int crece = (int)imin(100, (int)(tt / 25) - i * 6);
    if (crece < 0) crece = 0;
    int h = 8 + (crece * (4 + (i % 3) * 2)) / 100;
    int sway = (int)lroundf(2.0f * sinf(ahora / 600.0f + i));
    flor(xs[i], 63, h, i % 2, sway);
  }
  if (anios >= 1) {
    char b[24];
    snprintf(b, sizeof(b), anios == 1 ? "¡1 año juntos!" : "¡%d años juntos!", anios);
    u8g2.setFont(u8g2_font_6x10_tf);
    textoCentradoBorde(b, 10);
  }
  fuenteQueEntra("Aniversario", 120);
  textoCentradoBorde("Feliz", anios >= 1 ? 27 : 24);
  textoCentradoBorde("Aniversario", anios >= 1 ? 44 : 44);
}

void escenaBarco(uint32_t ahora, uint32_t tt) {
  // sol con rayos
  sol(108, 14, 6, ahora);
  // nubes
  int nx = 140 - (int)((ahora / 90) % 190);
  disc(nx, 14, 4); disc(nx + 6, 11, 5); disc(nx + 13, 14, 4); bx(nx - 3, 14, 20, 4);
  // olas
  for (int capa = 0; capa < 2; capa++) {
    int yb = 54 + capa * 5;
    int ant = yb;
    for (int x = 0; x < 128; x += 2) {
      int y = yb + (int)lroundf(2.5f * sinf((x + ahora / (capa ? 55.0f : 40.0f)) * 0.16f + capa));
      if (x > 0) ln(x - 2, ant, x, y);
      ant = y;
    }
  }
  // barquito cruzando (con vaivén)
  float u = (float)tt / 8200.0f;
  if (u > 1) u = 1;
  int bxp = -36 + (int)(u * 176.0f);
  int byp = 42 + (int)lroundf(2.0f * sinf(ahora / 300.0f));
  barquito(bxp, byp);
}

void escenaGif(uint32_t ahora, uint32_t tt) {
  estrellitasDibujar(ahora);
  uint8_t n = GIF_CUADROS[gifElegido];
  uint8_t cuadro = (uint8_t)((tt * 30UL / 1000UL) % n);
  u8g2.drawXBM(32, 0, 64, 64, GIFS[gifElegido] + (uint32_t)cuadro * 512UL);
}

void escenaCuti(uint32_t ahora, uint32_t tt) {
  estrellitasDibujar(ahora);
  flotasDibujar(ahora, 7);
  int latido = (int)lroundf(1.5f * sinf(tt / 160.0f));
  u8g2.setFont(u8g2_font_7x13_tf);                 // letra por defecto (la misma del menú)
  textoCentradoBorde("Te quiero cuti", 36 + latido);
}

void escenaDev(uint32_t ahora, uint32_t tt) {
  codigoDibujar(ahora);
  u8g2.setFont(u8g2_font_7x13_tf);                 // letra por defecto (la misma del menú)
  int x = (128 - u8g2.getUTF8Width("Dev ginius Dev_")) / 2;
  textoConBorde(x, 36, "Dev ginius Dev");
  if ((ahora / 450) % 2 == 0) textoConBorde(x + u8g2.getUTF8Width("Dev ginius Dev"), 36, "_");   // cursor
  (void)tt;
}

void escenaHoraPunto(uint32_t ahora, uint32_t tt, int hora, int minuto) {
  char b[8];
  snprintf(b, sizeof(b), "%02d:%02d", hora, minuto);
  uint8_t alto = fuenteHoraQueEntra(b, 124);
  int y = 32 + alto / 2;
  // entra deslizando desde abajo
  int off = (tt < 350) ? (int)(((350 - tt) * 40) / 350) : 0;
  textoCentrado(b, y + off);
  (void)ahora;
}

void escenaAlarma(uint32_t ahora) {
  bool inv = ((ahora / 350) % 2) == 0;
  if (inv) u8g2.drawBox(0, 0, 128, 64);
  u8g2.setDrawColor(inv ? 0 : 1);
  u8g2.setFont(u8g2_font_helvB18_tf);
  textoCentrado("¡Listo!", 28);
  int tem = ((ahora / 70) % 2) ? 2 : -2;
  u8g2.drawRBox(30 + tem, 38, 24, 22, 6);
  u8g2.drawRBox(74 + tem, 38, 24, 22, 6);
  u8g2.setDrawColor(1);
}

// Dibuja un cuadro de la escena. Devuelve true mientras sigue.
void especialDibujar(uint8_t e, uint32_t ahora, uint32_t tt, int a, int b) {
  u8g2.clearBuffer();
  switch (e) {
    case SEC_CUMPLE:     escenaCumple(ahora, tt); break;
    case SEC_ANIV:       escenaAniv(ahora, tt, a); break;
    case SEC_BARCO:      escenaBarco(ahora, tt); break;
    case SEC_GIF:        escenaGif(ahora, tt); break;
    case SEC_CUTI:       escenaCuti(ahora, tt); break;
    case SEC_DEV:        escenaDev(ahora, tt); break;
    case SEC_HORA_PUNTO: escenaHoraPunto(ahora, tt, a, b); break;
    case SEC_ALARMA:     escenaAlarma(ahora); break;
    default: break;
  }
  u8g2.sendBuffer();
}
