// ======================================================================
//  ROBOTITO  -  versión de UN SOLO BOTÓN
//  Placa: ESP32C3 Dev Module   |   USB CDC On Boot: Enabled
//  Librería necesaria: U8g2 (de oliver). El resto viene con el soporte de ESP32.
//
//  >>> Antes de subirlo, escribí tu WiFi en config.h (arriba de todo) <<<
//
//  Gestos del botón:
//    toque ........... acción de la pantalla (en el menú: pasa a la opción siguiente)
//    mantener 1 s .... elegir / salir (en los ojos: abre el menú)
//    mantener 5 s .... apagar ("Chauchas")
// ======================================================================

#include "config.h"
#include "display.h"
#include "buttons.h"
#include "storage.h"
#include "net.h"
#include "sprites.h"
#include "eyes.h"
#include "gifs.h"
#include "menu.h"
#include "jokes.h"
#include "tools.h"
#include "games.h"
#include "secrets.h"

// ---------- Estados ----------
enum Estado : uint8_t {
  S_OFF,           // pantalla apagada; un toque lo prende
  S_DESPERTANDO,   // animación de abrir los ojos
  S_IDLE,          // ojos con expresiones al azar
  S_DURMIENDO,     // ojos cerrados y "zzz" (cualquier toque lo despierta)
  S_MENU,
  S_APP,           // hora, clima, cronómetro, juegos...
  S_ESPECIAL,      // festejos, secretos, hora en punto y alarma
  S_APAGANDO       // "Chauchas" y apagado
};

Estado   estado = S_OFF;
uint32_t tEstado = 0;       // cuándo entró al estado actual
uint32_t tActividad = 0;    // última vez que se tocó el botón

Menu*      menuActual = &menuPrincipal;
bool       menuSucio = false;
const App* appActual = nullptr;

uint8_t  espActual = SEC_NINGUNO;
uint32_t espT0 = 0;
int      espA = 0, espB = 0, espAnio = 0;
bool     pruebaVisto[2] = { false, false };
long     ultimaMarcaHora = -1;

// ---------- "Ya lo vio este año" (cumpleaños y aniversario) ----------
bool yaVisto(uint8_t cual, int anio) {
  if (MODO_PRUEBA_FECHA) return pruebaVisto[cual];   // en prueba no se toca la memoria
  return leerU16(cual == 0 ? "yCumple" : "yAniv") == (uint16_t)anio;
}
void marcarVisto(uint8_t cual, int anio) {
  if (MODO_PRUEBA_FECHA) { pruebaVisto[cual] = true; return; }
  guardarU16(cual == 0 ? "yCumple" : "yAniv", (uint16_t)anio);
}

// ---------- Transiciones ----------
void irAIdle() {
  estado = S_IDLE;
  tEstado = millis();
  ojosIniciar(tEstado);
  aplicarBrillo();
  golpesReiniciar();
}

void encender() {
  randomSeed(micros());     // al tocar el botón, así cada encendido es distinto
  u8g2.setPowerSave(0);
  aplicarBrillo();
  estado = S_DESPERTANDO;
  tEstado = millis();
  tActividad = tEstado;
}

void iniciarApagado() {
  estado = S_APAGANDO;
  tEstado = millis();
  aplicarBrillo();
}

void apagarPantalla() {
  u8g2.clearBuffer();
  u8g2.sendBuffer();
  u8g2.setPowerSave(1);
  estado = S_OFF;
}

void despertarDeSueno() {
  consumirTodos();
  aplicarBrillo();
  estado = S_DESPERTANDO;
  tEstado = millis();
}

// Un parpadeo rápido como respuesta a cada toque en los ojos
void ojosParpadear(uint32_t ahora) {
  if (ojos.expr == EX_NEUTRAL || ojos.expr == EX_MIRAR) ojos.tFinParp = ahora + 140;
}

// ---------- Pantallas especiales ----------
void lanzarEspecial(uint8_t e, int a, int b) {
  u8g2.setPowerSave(0);        // si estaba apagado o dormido, se prende solo
  aplicarBrillo();
  espActual = e;
  espT0 = millis();
  espA = a;
  espB = b;
  especialIniciar(e, espT0);
  estado = S_ESPECIAL;
}

void terminarEspecial() {
  if (espActual == SEC_CUMPLE) marcarVisto(0, espAnio);
  if (espActual == SEC_ANIV)   marcarVisto(1, espAnio);
  espActual = SEC_NINGUNO;
  irAIdle();
}

void loopEspecial(uint32_t ahora) {
  uint32_t dur = especialDuracion(espActual);
  bool termina = (dur > 0 && ahora - espT0 >= dur);
  if (algunaPresion()) {       // un toque la cierra
    consumirTodos();
    termina = true;
  }
  if (termina) { terminarEspecial(); return; }
  if (frameListo(ahora)) especialDibujar(espActual, ahora, ahora - espT0, espA, espB);
}

// Una vez por segundo: fechas especiales y hora en punto
void chequearEventos(uint32_t ahora) {
  static uint32_t tUlt = 0;
  if (ahora - tUlt < 1000) return;
  tUlt = ahora;
  if (estado == S_APAGANDO || estado == S_DESPERTANDO || estado == S_ESPECIAL) return;

  Ahora a;
  if (!leerAhora(a)) return;     // todavía sin hora de internet

  if (a.mes == CUMPLE_MES && a.dia == CUMPLE_DIA && !yaVisto(0, a.anio)) {
    espAnio = a.anio;
    lanzarEspecial(SEC_CUMPLE, a.anio, 0);
    return;
  }
  if (a.mes == ANIV_MES && a.dia == ANIV_DIA && a.anio > JUNTOS_ANIO && !yaVisto(1, a.anio)) {
    espAnio = a.anio;
    lanzarEspecial(SEC_ANIV, a.anio - JUNTOS_ANIO, 0);
    return;
  }
  // La hora grande aparece con el robot despierto en modo ojos. Si está dormido, también lo
  // despierta, pero solo de día (así de noche no ilumina el cuarto).
  bool puede = (estado == S_IDLE) ||
               (estado == S_DURMIENDO && a.hora >= ANUNCIO_DESDE_H && a.hora <= ANUNCIO_HASTA_H);
  if (puede) {
    bool toca = MODO_PRUEBA_ANUNCIO_CADA_MINUTO ? (a.seg < 3) : (a.min == 0 && a.seg < 3);
    long marca = diasDesdeCivil(a.anio, a.mes, a.dia) * 1440L + a.hora * 60L + (MODO_PRUEBA_ANUNCIO_CADA_MINUTO ? a.min : 0);
    if (toca && marca != ultimaMarcaHora) {
      ultimaMarcaHora = marca;
      // si estaba dormido, que se quede despierto un minuto después de mostrar la hora
      if (estado == S_DURMIENDO) tActividad = ahora - (SUENO_MS - 60000UL);
      lanzarEspecial(SEC_HORA_PUNTO, a.hora, a.min);
    }
  }
}

// ---------- Menú ----------
void abrirMenu(Menu* m) {
  menuActual = m;
  m->cursor = 0;
  m->primero = 0;
  estado = S_MENU;
  menuSucio = true;
  tEstado = millis();
}

void menuVolver() {
  if (menuActual->padre) {
    menuActual = menuActual->padre;
    menuSucio = true;
  } else {
    irAIdle();
  }
}

void abrirApp(const App* app) {
  appActual = app;
  app->iniciar(millis());
  estado = S_APP;
  tEstado = millis();
}

void salirApp() {
  estado = S_MENU;
  menuSucio = true;
}

void ejecutarItem(uint8_t id) {
  switch (id) {
    case ITEM_HORA:     abrirApp(&APP_HORA);     break;
    case ITEM_CLIMA:    abrirApp(&APP_CLIMA);    break;
    case ITEM_DIAS:     abrirApp(&APP_DIAS);     break;
    case ITEM_CRONO:    abrirApp(&APP_CRONO);    break;
    case ITEM_TIMER:    abrirApp(&APP_TIMER);    break;
    case ITEM_MONEDA:   abrirApp(&APP_MONEDA);   break;
    case ITEM_DADO:     abrirApp(&APP_DADO);     break;
    case ITEM_BOLA:     abrirApp(&APP_BOLA);     break;
    case ITEM_CHISTE:   abrirApp(&APP_CHISTE);   break;
    case ITEM_JUEGOS:   abrirMenu(&menuJuegos);  break;
    case ITEM_DINO:     abrirApp(&APP_DINO);     break;
    case ITEM_FLAPPY:   abrirApp(&APP_FLAPPY);   break;
    case ITEM_SNAKE:    abrirApp(&APP_SNAKE);    break;
    default: break;
  }
}

// En el menú: un toque pasa a la siguiente opción; mantener 1 s elige ("Volver" es la última)
void loopMenu(uint32_t ahora) {
  Menu& m = *menuActual;

  if (enter.corto) { menuSiguiente(m); menuSucio = true; }
  if (enter.largo) {
    if (m.cursor == m.n) menuVolver();
    else ejecutarItem(m.items[m.cursor].id);
    return;
  }

  if (ahora - tActividad > MENU_TIMEOUT_MS) { irAIdle(); return; }

  if (menuSucio) {
    menuDibujar(m);
    menuSucio = false;
  }
}

// ---------- Arduino ----------
void setup() {
  pantallaIniciar();
  botonesIniciar();
  almacenamientoIniciar();
  aplicarBrillo();
  redIniciar();             // se conecta al WiFi en segundo plano
  apagarPantalla();         // al enchufarlo arranca apagado
}

void loop() {
  uint32_t ahora = millis();
  botonesActualizar();
  redActualizar(ahora);
  if (algunaPresion()) tActividad = ahora;

  if (estado != S_APAGANDO && timerVencido(ahora)) lanzarEspecial(SEC_ALARMA, 0, 0);
  chequearEventos(ahora);

  // Mantener el botón 5 s apaga desde cualquier pantalla
  if (enter.muyLargo && estado != S_OFF && estado != S_APAGANDO) {
    iniciarApagado();
    return;
  }

  switch (estado) {
    case S_OFF:
      if (enter.press) {
        consumir(enter);
        encender();
      } else {
        delay(10);
      }
      break;

    case S_DESPERTANDO:
      if (frameListo(ahora) && ojosDespertar(ahora - tEstado)) irAIdle();
      break;

    case S_IDLE: {
      if (enter.largo) { abrirMenu(&menuPrincipal); break; }       // mantener 1 s: menú
      if (enter.corto) { golpesToque(ahora); ojosParpadear(ahora); }
      uint8_t e = golpesRevisar(ahora);                             // ¿se completó un secreto?
      if (e != SEC_NINGUNO) { lanzarEspecial(e, 0, 0); break; }
      if (ahora - tActividad > SUENO_MS) {
        estado = S_DURMIENDO;
        aplicarBrilloDormido();
        break;
      }
      ojosActualizar(ahora);
      break;
    }

    case S_DURMIENDO:
      if (algunaPresion()) { despertarDeSueno(); break; }
      if (frameListo(ahora)) ojosDormido(ahora);
      break;

    case S_MENU:
      loopMenu(ahora);
      break;

    case S_APP:
      if (enter.largo) {                                            // mantener 1 s
        bool salir = appActual->alMantener ? appActual->alMantener(ahora) : true;
        if (salir) { salirApp(); break; }
      }
      if (appActual->timeoutMs && ahora - tActividad > appActual->timeoutMs) { irAIdle(); break; }
      if (appActual->actualizar(ahora)) salirApp();
      break;

    case S_ESPECIAL:
      loopEspecial(ahora);
      break;

    case S_APAGANDO:
      if (frameListo(ahora)) dibujarChauchas(ahora - tEstado);
      if (ahora - tEstado >= 2000) apagarPantalla();
      break;
  }
}
