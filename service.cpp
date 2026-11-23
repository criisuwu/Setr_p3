/*
    Sistemas Empotrados y Tiempo Real
    Creador: Cristina Homobono Fernández
    Fecha: 23/11/26
    
    Archivo para las funciones del sistema de servicio al cliente
*/

#include "library.h"

void serviceLoop(unsigned long now, Joy joy, bool joyBtn) {
  switch (state) {

    case ST_BOOT:
      break;

    case ST_IDLE:
      if (distanceCm < PRESENCE_CM) {
        drawEnv();
        setState(ST_ENV);
      }
      break;

    case ST_ENV:
      if (now - stateStart >= ENV_SHOW_MS) {
        sel = 0;
        drawMenu(N_PROD, sel, fmtProduct);
        setState(ST_CATALOG);
      }
      break;

    case ST_CATALOG:
      if (joy == J_UP && sel > 0) { sel--; drawMenu(N_PROD, sel, fmtProduct); }
      else if (joy == J_DOWN && sel < N_PROD - 1) { sel++; drawMenu(N_PROD, sel, fmtProduct); }
      else if (joyBtn) {
        chosen = sel;
        prepDur = (unsigned long)(chosen + 1) * PREP_BASE_MS;
        lcdLine(0, "Preparando...");
        lcdLine(1, "");
        setState(ST_PREPARING);
      }
      break;

    case ST_PREPARING:
      break;                              

    case ST_RETIRE:
      if (now - stateStart >= RETIRE_MS) enterIdle();
      break;

    default:
      break;
  }
}

// Muestra el proceso de preparacion de la bebida y avisa de que se puede retirar el producto
void serviceUI() {
  char b[17];
  if (state == ST_PREPARING) {
    unsigned long el = millis() - stateStart;
    if (el >= prepDur) {
      setLed2Level(64);
      lcdLine(0, "RETIRE BEBIDA");
      lcdLine(1, "");
      setState(ST_RETIRE);
    } else {
      uint8_t pct = (el * 100UL) / prepDur;
      setLed2Level((pct * 64UL) / 100);
      snprintf(b, 17, "%-9s %3u%%", NAMES[chosen], pct);
      lcdLine(1, b);
    }
  }
}