/*
    Sistemas Empotrados y Tiempo Real
    Creador: Cristina Homobono Fernández
    Fecha: 23/11/26
    
    El archivo main para el funcionamiento de la maquina
*/
#include "library.h"

void setup() {
  initSystem();
}

void loop() {
  wdt_reset();
  controller.run();
  unsigned long now = millis();

  if (handleButton(now)) return;

  Joy joy = pendingJoy;     pendingJoy = J_NONE;
  bool joyBtn = pendingBtn; pendingBtn = false;

  if (state >= ST_ADMIN_MENU) adminLoop(now, joy, joyBtn);
  else                        serviceLoop(now, joy, joyBtn);
}