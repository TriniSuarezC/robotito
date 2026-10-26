#pragma once
#include "config.h"

struct Boton {
  uint8_t  pin;
  bool     estable;                      // HIGH = suelto (INPUT_PULLUP)
  uint32_t tCambio, tPresion, tRepeat;
  bool     largoDisp, muyLargoDisp;
  // Eventos: valen solo durante una vuelta del loop
  bool press;      // se acaba de apretar
  bool corto;      // se soltó antes de LARGO_MS
  bool largo;      // se llegó a LARGO_MS apretado
  bool muyLargo;   // se llegó a MUY_LARGO_MS apretado
  bool repeat;     // repetición mientras se mantiene
};

Boton enter;   // el único botón

void botonIniciar(Boton& b, uint8_t pin) {
  b.pin = pin;
  pinMode(pin, INPUT_PULLUP);
  b.estable = HIGH;
  b.tCambio = b.tPresion = b.tRepeat = 0;
  b.largoDisp = b.muyLargoDisp = false;
  b.press = b.corto = b.largo = b.muyLargo = b.repeat = false;
}

void botonActualizar(Boton& b, uint32_t ahora) {
  b.press = b.corto = b.largo = b.muyLargo = b.repeat = false;

  bool actual = digitalRead(b.pin);
  if (actual != b.estable && ahora - b.tCambio > DEBOUNCE_MS) {
    b.tCambio = ahora;
    b.estable = actual;
    if (actual == LOW) {
      b.press = true;
      b.tPresion = ahora;
      b.tRepeat = ahora;
      b.largoDisp = false;
      b.muyLargoDisp = false;
    } else if (!b.largoDisp) {
      b.corto = true;
    }
  }

  if (b.estable == LOW) {
    uint32_t dur = ahora - b.tPresion;
    if (!b.largoDisp && dur >= LARGO_MS) {
      b.largoDisp = true;
      b.largo = true;
    }
    if (!b.muyLargoDisp && dur >= MUY_LARGO_MS) {
      b.muyLargoDisp = true;
      b.muyLargo = true;
    }
    if (dur >= REPEAT_INICIO_MS && ahora - b.tRepeat >= REPEAT_CADA_MS) {
      b.tRepeat = ahora;
      b.repeat = true;
    }
  }
}

void botonesIniciar()     { botonIniciar(enter, PIN_BOTON); }
void botonesActualizar()  { botonActualizar(enter, millis()); }

bool algunaPresion() { return enter.press; }
bool apretado(const Boton& b) { return b.estable == LOW; }   // se está manteniendo ahora

// Descarta lo que quede de una presión en curso (para que no dispare nada al soltar)
void consumir(Boton& b) {
  b.largoDisp = true;
  b.muyLargoDisp = true;
}
void consumirTodos() { consumir(enter); }
