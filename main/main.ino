#include <LiquidCrystal.h>

const int redPin = 11;
const int yellowPin = 10;

//  Declaracion de la LCD
LiquidCrystal lcd (4, 5, 6, 7, 8, 9);

void colorOn(int pin){
  digitalWrite (pin, HIGH);
}

void colorOff(int pin){
  digitalWrite (pin, LOW);
}

//Inicio o reinicio del sistema
void systemOn()
{
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

void setup() {
  //pinMode(redPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);

  Serial.begin(9600);

  //  Inicializacion del LCD
  lcd.begin(16, 2);

  //hace que solo se ejecute cuando se enciende o cuando se reinicia
  systemOn();
}

void loop() {
  //codigo aqui

  delay(1000);

}
