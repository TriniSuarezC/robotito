#pragma once
#include "display.h"
#include "config.h"

struct Item { const char* label; uint8_t id; };

struct Menu {
  const Item* items;
  uint8_t n;          // cantidad de items (sin contar "Volver")
  uint8_t cursor;     // 0..n  (n = "Volver")
  uint8_t primero;    // primera fila visible
  Menu*   padre;      // menú al que se vuelve (nullptr = salir a los ojos)
};

enum ItemId : uint8_t {
  ITEM_HORA, ITEM_CLIMA, ITEM_DIAS, ITEM_CRONO, ITEM_TIMER,
  ITEM_MONEDA, ITEM_DADO, ITEM_BOLA, ITEM_CHISTE, ITEM_JUEGOS,
  ITEM_DINO, ITEM_FLAPPY, ITEM_SNAKE
};

#define N_ITEMS(a) ((uint8_t)(sizeof(a) / sizeof((a)[0])))

const Item ITEMS_PRINCIPAL[] = {
  { "Hora",          ITEM_HORA },
  { "Clima",         ITEM_CLIMA },
  { "Días juntos",   ITEM_DIAS },
  { "Cronómetro",    ITEM_CRONO },
  { "Timer",         ITEM_TIMER },
  { "Cara o cruz",   ITEM_MONEDA },
  { "Dado",          ITEM_DADO },
  { "Bola mágica 8", ITEM_BOLA },
  { "Chiste nerd",   ITEM_CHISTE },
  { "Juegos",        ITEM_JUEGOS }
};

const Item ITEMS_JUEGOS[] = {
  { "Dino",    ITEM_DINO },
  { "Flappy",  ITEM_FLAPPY },
  { "Snake",   ITEM_SNAKE }
};

Menu menuPrincipal = { ITEMS_PRINCIPAL, N_ITEMS(ITEMS_PRINCIPAL), 0, 0, nullptr };
Menu menuJuegos    = { ITEMS_JUEGOS,    N_ITEMS(ITEMS_JUEGOS),    0, 0, &menuPrincipal };

// ---------- Navegación circular ----------
const uint8_t FILAS = 4;
const uint8_t ALTO_FILA = 16;

// Un toque pasa a la opción siguiente (y da la vuelta al llegar al final)
void menuSiguiente(Menu& m) {
  uint8_t total = m.n + 1;                       // + "Volver"
  m.cursor = (m.cursor + 1) % total;
  if (m.cursor < m.primero) m.primero = m.cursor;
  if (m.cursor >= m.primero + FILAS) m.primero = m.cursor - FILAS + 1;
}

// ---------- Dibujo ----------
void menuDibujar(const Menu& m) {
  uint8_t total = m.n + 1;
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x13_tf);

  for (uint8_t i = 0; i < FILAS; i++) {
    uint8_t idx = m.primero + i;
    if (idx >= total) break;
    const char* txt = (idx == m.n) ? "« Volver" : m.items[idx].label;
    int y = i * ALTO_FILA;
    if (idx == m.cursor) {
      u8g2.drawRBox(0, y, 121, ALTO_FILA - 1, 3);
      u8g2.setDrawColor(0);
      u8g2.drawUTF8(6, y + 12, txt);
      u8g2.setDrawColor(1);
    } else {
      u8g2.drawUTF8(6, y + 12, txt);
    }
  }

  // barra de scroll
  if (total > FILAS) {
    int th = imax(6, 64 * FILAS / total);
    int ty = (64 - th) * m.primero / (total - FILAS);
    u8g2.drawVLine(126, 0, 64);
    u8g2.drawBox(124, ty, 4, th);
  }
  u8g2.sendBuffer();
}
