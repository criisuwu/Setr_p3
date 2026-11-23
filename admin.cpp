/*
    Sistemas Empotrados y Tiempo Real
    Creador: Cristina Homobono Fernández
    Fecha: 23/11/26
    
    Archivo para las funciones del sistema de administrador
*/

#include "library.h"

static void fmtAdmin(uint8_t i, char* b) {
  const char* t[4] = {"a. Temperatura", "b. Distancia", "c. Contador", "d. Precios"};
  snprintf(b, 16, "%s", t[i]);
}

void enterAdmin() {
  adminMode = true;
  digitalWrite(LED1, HIGH);
  setLed2Level(64);
  sel = 0;
  drawMenu(4, sel, fmtAdmin);
  setState(ST_ADMIN_MENU);
}

void exitAdmin() {
  adminMode = false;
  enterIdle();
}

void adminLoop(unsigned long now, Joy joy, bool joyBtn) {
  (void)now;
  switch (state) {

    case ST_ADMIN_MENU:
      if (joy == J_UP && sel > 0) { sel--; drawMenu(4, sel, fmtAdmin); }
      else if (joy == J_DOWN && sel < 3) { sel++; drawMenu(4, sel, fmtAdmin); }
      else if (joyBtn) {
        lcd.clear();
        switch (sel) {
          case 0: setState(ST_ADMIN_TEMP); break;
          case 1: setState(ST_ADMIN_DIST); break;
          case 2: setState(ST_ADMIN_COUNT); break;
          case 3:
            sel = 0;
            drawMenu(N_PROD, sel, fmtProduct);
            setState(ST_ADMIN_PRICES);
            break;
        }
      }
      break;

    case ST_ADMIN_TEMP:
    case ST_ADMIN_DIST:
    case ST_ADMIN_COUNT:
      if (joy == J_LEFT) {
        sel = (state == ST_ADMIN_TEMP) ? 0 : (state == ST_ADMIN_DIST) ? 1 : 2;
        drawMenu(4, sel, fmtAdmin);
        setState(ST_ADMIN_MENU);
      }
      break;

    case ST_ADMIN_PRICES:
      if (joy == J_LEFT) { sel = 3; drawMenu(4, sel, fmtAdmin); setState(ST_ADMIN_MENU); }
      else if (joy == J_UP && sel > 0) { sel--; drawMenu(N_PROD, sel, fmtProduct); }
      else if (joy == J_DOWN && sel < N_PROD - 1) { sel++; drawMenu(N_PROD, sel, fmtProduct); }
      else if (joyBtn) {
        editPrice = prices[sel];
        char b[17];
        lcdLine(0, NAMES[sel]);
        snprintf(b, 17, "Precio: %d.%02dE", editPrice / 100, editPrice % 100);
        lcdLine(1, b);
        setState(ST_ADMIN_EDIT);
      }
      break;

    case ST_ADMIN_EDIT: {
      bool redraw = false;
      if (joy == J_UP && editPrice < 995) { editPrice += 10; redraw = true; }
      else if (joy == J_DOWN && editPrice > 0) { editPrice -= 10; redraw = true; }
      else if (joy == J_LEFT) {           // si se va a la izquierda se vuelve hacia atras
        drawMenu(N_PROD, sel, fmtProduct);
        setState(ST_ADMIN_PRICES);
        break;
      } else if (joyBtn) {                // presiona el boton para confirmar
        prices[sel] = editPrice;
        drawMenu(N_PROD, sel, fmtProduct);
        setState(ST_ADMIN_PRICES);
        break;
      }
      if (redraw) {
        char b[17];
        snprintf(b, 17, "Precio: %d.%02dE", editPrice / 100, editPrice % 100);
        lcdLine(1, b);
      }
      break;
    }

    default:
      break;
  }
}

// Sistema de interfaz del modo administrador
void adminUI() {
  char b[17];
  switch (state) {
    case ST_ADMIN_TEMP:
      drawEnv();
      break;
    case ST_ADMIN_DIST:
      if (distanceCm >= 999) snprintf(b, 17, "Distancia: --");
      else snprintf(b, 17, "Distancia: %d cm", (int)distanceCm);
      lcdLine(0, b);
      lcdLine(1, "");
      break;
    case ST_ADMIN_COUNT: {
      noInterrupts();
      unsigned long s = uptimeSec;
      interrupts();
      snprintf(b, 17, "Contador: %lu s", s);
      lcdLine(0, b);
      lcdLine(1, "");
      break;
    }
    default:
      break;
  }
}