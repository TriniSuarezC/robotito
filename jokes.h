#pragma once
#include <Arduino.h>

struct Chiste { const char* pregunta; const char* remate; };

// 40 chistes: programación (10), matemática (10), física (10), química (10)
const Chiste CHISTES[] = {
  // --- Programación ---
  { "¿Por qué los programadores confunden Halloween con Navidad?", "Porque OCT 31 = DEC 25." },
  { "¿Cuántos programadores hacen falta para cambiar una lamparita?", "Ninguno: es un problema de hardware." },
  { "Un SQL entra a un bar, se acerca a dos mesas y pregunta:", "¿Puedo unirme?" },
  { "¿Por qué los programadores de Java necesitan anteojos?", "Porque no pueden ver C#." },
  { "Cuando mi código funciona a la primera...", "...sospecho que algo anda muy mal." },
  { "¿Qué le dice un bit a otro bit?", "Nos vemos en el bus." },
  { "Hay 10 tipos de personas en el mundo:", "las que entienden binario y las que no." },
  { "Una programadora le pide a su novio: 'Comprá una leche, y si hay huevos, traé 6'.", "Volvió con 6 leches: había huevos." },
  { "¿Por qué el programador se quedó para siempre en la ducha?", "Porque el champú decía: enjuagar, repetir." },
  { "¿Qué hace un programador cuando tiene frío?", "Cierra Windows." },
  // --- Matemática ---
  { "¿Qué le dice un 3 a un 30?", "Para ser como yo tenés que ser sincero." },
  { "¿Por qué el libro de matemática estaba triste?", "Porque tenía muchos problemas." },
  { "¿Qué le dijo un cero a un ocho?", "Lindo cinturón." },
  { "¿Qué es un oso polar?", "Un oso rectangular después de un cambio de coordenadas." },
  { "¿Qué le dijo una recta a otra recta?", "Nos vemos en el infinito." },
  { "¿Por qué los matemáticos se pierden en el bosque?", "Porque siempre buscan las raíces." },
  { "¿Cuál es la planta favorita de un matemático?", "La raíz cuadrada." },
  { "¿Por qué pi nunca termina una conversación?", "Porque es irracional." },
  { "¿Qué hace un matemático cuando tiene frío?", "Se para en una esquina: ahí hay 90 grados." },
  { "¿Qué hace un matemático en el gimnasio?", "Trabaja sus potencias." },
  // --- Física ---
  { "Un fotón entra a un hotel y el recepcionista le pregunta si tiene equipaje.", "No, viajo ligero." },
  { "¿Qué le dice un electrón a otro?", "Qué negativo sos." },
  { "¿Por qué algunos parecen brillantes hasta que hablan?", "Porque la luz viaja más rápido que el sonido." },
  { "Dos átomos se cruzan. Uno dice: 'Perdí un electrón'. ¿Seguro?", "Sí, estoy positivo." },
  { "Un policía frena a Heisenberg: '¿Sabe a qué velocidad iba?'", "No, pero sé exactamente dónde estaba." },
  { "¿Cuál es el colmo de la gravedad?", "Que te caiga mal." },
  { "Nunca confíes en un átomo:", "lo componen todo." },
  { "¿Qué le dijo el imán a la heladera?", "Me atraés." },
  { "¿Cuál es la fruta favorita de Newton?", "La manzana: siempre le cae bien." },
  { "¿Cuál es el colmo de Schrödinger?", "Que su gato esté vivo y muerto de risa." },
  // --- Química ---
  { "Un químico pide H2O. Otro dice: 'Yo también: H2O2'.", "Al segundo lo enterraron." },
  { "¿Qué le dijo el oxígeno al potasio?", "OK." },
  { "¿Qué le dijo el cloro al sodio?", "Juntos somos la sal de la vida." },
  { "¿Qué le dijo un ion a otro?", "Me tenés cargado." },
  { "Lema de los químicos:", "Si no sos parte de la solución, sos parte del precipitado." },
  { "¿Por qué los químicos no se separan?", "Porque tienen buena química." },
  { "¿Cuál es el colmo de un químico?", "Que su novia se le ponga ácida." },
  { "¿Por qué los gases nobles no tienen amigos?", "Porque no reaccionan a nada." },
  { "¿Qué le dijo un electrón a un protón?", "Qué positivo sos." },
  { "¿Qué hace un elemento cuando se enoja?", "Se pone radiactivo." }
};
const uint8_t N_CHISTES = sizeof(CHISTES) / sizeof(CHISTES[0]);

struct Respuesta8 { const char* texto; uint8_t tipo; };   // tipo: 0 sí, 1 tal vez, 2 no

const Respuesta8 BOLA8[] = {
  { "Sí, definitivamente", 0 },
  { "Sin ninguna duda", 0 },
  { "Todo indica que sí", 0 },
  { "Mis circuitos dicen que sí", 0 },
  { "Podés contar con ello", 0 },
  { "Claro que sí", 0 },
  { "Las señales son buenas", 0 },
  { "Compilado con éxito: sí", 0 },
  { "Obvio que sí", 0 },
  { "¡Sí, sí, sí!", 0 },
  { "Preguntá de nuevo más tarde", 1 },
  { "Concentrate y volvé a preguntar", 1 },
  { "Mejor no te lo digo ahora", 1 },
  { "Respuesta confusa, reintentá", 1 },
  { "Ni yo lo sé... todavía", 1 },
  { "Mis fuentes dicen que no", 2 },
  { "Muy dudoso", 2 },
  { "No cuentes con ello", 2 },
  { "Mi respuesta es no", 2 },
  { "Ni en sueños", 2 }
};
const uint8_t N_BOLA8 = sizeof(BOLA8) / sizeof(BOLA8[0]);
