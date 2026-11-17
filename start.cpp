/*
    Sistemas Empotrados y Tiempo Real
    Creador: Cristina Homobono Fernández
    Fecha: 17/11/26
    
    Archivo para las funciones para el arranque y el reinicio
*/

#include "library.h"

void systemOn(){
  lcd.setCursor(0, 0);
  lcd.print("Cargando...");

  for (int i = 0; i <= 2; i++)
  {
    colorOn(yellowPin);
    delay(100);
    colorOff(yellowPin);
    delay(100);
  }
}