/*
    Sistemas Empotrados y Tiempo Real
    Creador: Cristina Homobono Fernández
    Fecha: 23/11/26
    
    Archivo libreria para acortar código
*/
#ifndef LIBRARY_H
#define LIBRARY_H

#include <Arduino.h>
#include <DHT.h>
#include <LiquidCrystal.h>
#include <Thread.h>
#include <ThreadController.h>
#include <TimerOne.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>

// Los pines 0 y 1 ya no se usan para el motor, así que se puede usar Serial con DEBUG 1.
#define DEBUG 0

// ---------------- Pines ----------------
#define DHTPIN A4        // DHT11 en A4
#define DHTTYPE DHT11
extern DHT dht;
extern LiquidCrystal lcd;

const byte LED1 = A2;
const byte LED2 = A3;
const byte BUTTON_PIN = 10;
const byte PIN_TRIGGER = 2;
const byte PIN_ECHO = 3;
const byte JOY_X = A0;
const byte JOY_Y = A1;
const byte JOY_SW = 11;

// ---------------- Preparación de bebida ----------------
// Sin motor, la preparación es una espera con barra de progreso.
// Duración = (producto + 1) * PREP_BASE_MS  (Solo=3 s, Cortado=6 s, Doble=9 s...)
const unsigned long PREP_BASE_MS = 3000;

// ---------------- Constantes ----------------
const float V_SOUND = 0.0343f;
const unsigned long ECHO_TIMEOUT_US = 12000UL;   // ~2 m
const float PRESENCE_CM = 100.0f;
const unsigned long ENV_SHOW_MS = 5000;
const unsigned long RETIRE_MS = 3000;
const unsigned long RESET_MIN_MS = 2000;   // reinicio: soltar entre RESET_MIN_MS y ADMIN_MS
const unsigned long ADMIN_MS = 5000;       // admin: mantener ADMIN_MS

// ---------------- Productos ----------------
const uint8_t N_PROD = 5;
const bool RESET_RESTORES_PRICES = true;
extern const char* const NAMES[N_PROD];
extern const int DEFAULT_PRICES[N_PROD];
extern int prices[N_PROD];

// ---------------- Tipos ----------------
enum State {
  ST_BOOT, ST_IDLE, ST_ENV, ST_CATALOG, ST_PREPARING, ST_RETIRE,
  ST_ADMIN_MENU, ST_ADMIN_TEMP, ST_ADMIN_DIST, ST_ADMIN_COUNT,
  ST_ADMIN_PRICES, ST_ADMIN_EDIT
};
enum Joy { J_NONE, J_UP, J_DOWN, J_LEFT, J_RIGHT };

// ---------------- Estado global ----------------
extern volatile State state;
extern bool adminMode;

// Threads
extern ThreadController controller;
extern Thread tBlink, tJoy, tDist, tDHT, tUI;

// TimerOne
extern volatile unsigned long uptimeSec;
extern volatile uint8_t pwmLevel;
extern volatile uint8_t pwmCount;
extern volatile uint16_t tickCount;

// Botón externo
extern volatile bool btnPressed;
extern volatile bool btnLongFired;
extern volatile bool btnReleaseEvent;
extern volatile unsigned long btnPressStart;
extern volatile unsigned long btnLastEdge;
extern volatile unsigned long btnReleaseDur;

// Joystick
extern bool joyLatched;
extern Joy pendingJoy;
extern bool pendingBtn;
extern bool joySwPrev;
extern unsigned long joySwTime;

// Variables de trabajo
extern unsigned long stateStart;
extern uint8_t sel;
extern int editPrice;
extern uint8_t chosen;
extern unsigned long prepDur;
extern float temperature, humidity;
extern float distanceCm;
extern uint8_t blinkCount;
extern bool led1State;

// ---------------- start.cpp ----------------
void isrTick();
void setLed2Level(uint8_t level);
Joy  readJoy();
bool readJoyButton();
void lcdLine(uint8_t row, const char* txt);
float measureDistance();
void setState(State s);
void fmtProduct(uint8_t i, char* b);
void drawMenu(uint8_t n, uint8_t s, void (*fmt)(uint8_t, char*));
void drawEnv();
void enterIdle();
void startBoot();
void resetSystem();
void blinkTask();
void joyTask();
void distTask();
void dhtTask();
void uiTask();
void initSystem();            // todo el contenido de setup()
bool handleButton(unsigned long now);   // true => salir del loop() esta vuelta

// ---------------- service.cpp ----------------
void serviceLoop(unsigned long now, Joy joy, bool joyBtn);
void serviceUI();

// ---------------- admin.cpp ----------------
void enterAdmin();
void exitAdmin();
void adminLoop(unsigned long now, Joy joy, bool joyBtn);
void adminUI();

#endif