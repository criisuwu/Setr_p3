/*
    Sistemas Empotrados y Tiempo Real
    Creador: Cristina Homobono Fernández
    Fecha: 23/11/26
    
    Archivo para la base del programa, incluye inicio y reinicio, gestión de hilos, gestión de sensores...
*/

#include "library.h"

// ---------------- Objetos ----------------
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal lcd(4, 5, 6, 7, 8, 9);

// ---------------- Productos y Precios ----------------
const char* const NAMES[N_PROD] = {"Solo", "Cortado", "Doble", "Premium", "Chocolate"};
const int DEFAULT_PRICES[N_PROD] = {100, 110, 125, 150, 200};
int prices[N_PROD] = {100, 110, 125, 150, 200};

// ---------------- Estado global ----------------
volatile State state = ST_BOOT;
bool adminMode = false;

// Hilos
ThreadController controller;
Thread tBlink, tJoy, tDist, tDHT, tUI;

// Contador de segundos y brillo de LED
volatile unsigned long uptimeSec = 0;
volatile uint8_t pwmLevel = 0;
volatile uint8_t pwmCount = 0;
volatile uint16_t tickCount = 0;

// Boton
volatile bool btnPressed = false;
volatile bool btnLongFired = false;
volatile bool btnReleaseEvent = false;
volatile unsigned long btnPressStart = 0;
volatile unsigned long btnLastEdge = 0;
volatile unsigned long btnReleaseDur = 0;

// Joystick
bool joyLatched = false;
Joy pendingJoy = J_NONE;
bool pendingBtn = false;
bool joySwPrev = false;
unsigned long joySwTime = 0;

// Display en pantalla y movimientos de UI
unsigned long stateStart = 0;
uint8_t sel = 0;
int editPrice = 0;
uint8_t chosen = 0;
unsigned long prepDur = 0;
float temperature = 0, humidity = 0;
float distanceCm = 999;
uint8_t blinkCount = 0;
bool led1State = false;

// contador de tiempo para las interrupciones
void isrTick() {
  if (++pwmCount >= 64) pwmCount = 0;
  digitalWrite(LED2, pwmCount < pwmLevel ? HIGH : LOW);

  if (++tickCount >= 8000) {
    tickCount = 0;
    uptimeSec++;
  }
}

// Potencia del LED2
void setLed2Level(uint8_t level) {
  if (level > 64) level = 64;
  pwmLevel = level;
}

// ---------------- Botón ----------------
// Detecta si se ha presionado el botón
ISR(PCINT0_vect) {
  unsigned long now = millis();
  if (now - btnLastEdge < 20) return;
  bool low = !(PINB & (1 << PB2));
  if (low == btnPressed) return;
  btnLastEdge = now;
  if (low) {
    btnPressed = true;
    btnPressStart = now;
    btnLongFired = false;
  } else {
    btnPressed = false;
    btnReleaseDur = now - btnPressStart;
    btnReleaseEvent = true;
  }
}

// ---------------- Joystick ----------------
Joy readJoy() {
  analogRead(JOY_X);
  int x = analogRead(JOY_X);
  analogRead(JOY_Y);
  int y = analogRead(JOY_Y);
  Joy d = J_NONE;
  if (y < 300) d = J_UP;
  else if (y > 700) d = J_DOWN;
  else if (x < 300) d = J_LEFT;
  else if (x > 700) d = J_RIGHT;
  if (d == J_NONE) { joyLatched = false; return J_NONE; }
  if (joyLatched) return J_NONE;
  joyLatched = true;
  return d;
}

bool readJoyButton() {
  bool p = (digitalRead(JOY_SW) == LOW);
  bool ev = false;
  if (p && !joySwPrev && millis() - joySwTime > 30) {
    ev = true;
    joySwTime = millis();
  }
  joySwPrev = p;
  return ev;
}

// ---------------- Utilidades ----------------
// ! Para no estar llamado constantemente al posicionamiento del cursor y la escritura he creado una función.
void lcdLine(uint8_t row, const char* txt) {
  lcd.setCursor(0, row);
  uint8_t i = 0;
  while (txt[i] && i < 16) lcd.write(txt[i++]);
  while (i < 16) { lcd.write(' '); i++; }
}

float measureDistance() {
  digitalWrite(PIN_TRIGGER, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIGGER, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIGGER, LOW);
  unsigned long d = pulseIn(PIN_ECHO, HIGH, ECHO_TIMEOUT_US);
  if (d == 0) return 999.0f;
  return d * V_SOUND / 2.0f;
}

// Inicia el tiempo
void setState(State s) {
  state = s;
  stateStart = millis();
}

// Display del producto y si precio
void fmtProduct(uint8_t i, char* b) {
  snprintf(b, 16, "%-9s %d.%02dE", NAMES[i], prices[i] / 100, prices[i] % 100);
}

void drawMenu(uint8_t n, uint8_t s, void (*fmt)(uint8_t, char*)) {
  uint8_t top = (s / 2) * 2;
  for (uint8_t r = 0; r < 2; r++) {
    uint8_t idx = top + r;
    char buf[18];
    if (idx < n) {
      fmt(idx, buf + 1);
      buf[0] = (idx == s) ? '>' : ' ';
    } else {
      buf[0] = '\0';
    }
    lcdLine(r, buf);
  }
}

void drawEnv() {
  char b[17];
  snprintf(b, 17, "Temp: %d \xDF" "C", (int)temperature);
  lcdLine(0, b);
  snprintf(b, 17, "Hum: %d %%", (int)humidity);
  lcdLine(1, b);
}

// ---------------- Espera al cliente ----------------
void enterIdle() {
  digitalWrite(LED1, LOW);
  setLed2Level(0);
  distanceCm = 999;
  lcdLine(0, "ESPERANDO");
  lcdLine(1, "CLIENTE");
  sel = 0;
  setState(ST_IDLE);
}

// Dislpay inicio del sistema o reinicio
void startBoot() {
  adminMode = false;
  digitalWrite(LED1, LOW);
  setLed2Level(0);
  distanceCm = 999;
  led1State = false;
  blinkCount = 0;
  lcd.clear();
  lcdLine(0, "CARGANDO ...");
  lcdLine(1, "");
  tBlink.setInterval(500);
  setState(ST_BOOT);
}

// Reinicio del sistema sin presionar el boton de la placa
void resetSystem() {
  noInterrupts();
  btnReleaseEvent = false;
  btnLongFired = false;
  uptimeSec = 0;
  tickCount = 0;
  interrupts();

  pendingJoy = J_NONE;
  pendingBtn = false;
  joyLatched = false;
  joySwPrev = false;

  chosen = 0;
  editPrice = 0;
  prepDur = 0;
  sel = 0;
  if (RESET_RESTORES_PRICES) {
    for (uint8_t i = 0; i < N_PROD; i++) prices[i] = DEFAULT_PRICES[i];
  }
  startBoot();
}

// ---------------- Threads ----------------
// Parpadeo
void blinkTask() {
  if (state != ST_BOOT) return;
  led1State = !led1State;
  digitalWrite(LED1, led1State);
  if (++blinkCount >= 6) { 
    digitalWrite(LED1, LOW);
    enterIdle();
  }
}

void joyTask() {
  Joy d = readJoy();
  if (d != J_NONE) pendingJoy = d;
  if (readJoyButton()) pendingBtn = true;
}

void distTask() {
  if (state == ST_IDLE || state == ST_ADMIN_DIST) {
    distanceCm = measureDistance();
#if DEBUG
    Serial.print(F("Distancia: "));
    Serial.println(distanceCm);
#endif
  }
}

void dhtTask() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (!isnan(h)) humidity = h;
  if (!isnan(t)) temperature = t;
}

void uiTask() {
  serviceUI();
  adminUI();
}

// ---------------- Inicialización ----------------
void initSystem() {
  wdt_disable();
#if DEBUG
  Serial.begin(115200);
#endif
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(JOY_SW, INPUT_PULLUP);
  pinMode(PIN_TRIGGER, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  digitalWrite(PIN_TRIGGER, LOW);

  randomSeed(analogRead(A5) ^ micros());

  dht.begin();
  lcd.begin(16, 2);

  // Explicación en el blog
  PCICR |= (1 << PCIE0);
  PCMSK0 |= (1 << PCINT2);

  // TimerOne
  Timer1.initialize(125);
  Timer1.attachInterrupt(isrTick);

  // Threads
  tBlink.onRun(blinkTask);     tBlink.setInterval(500);
  tJoy.onRun(joyTask);         tJoy.setInterval(20);
  tDist.onRun(distTask);       tDist.setInterval(150);
  tDHT.onRun(dhtTask);         tDHT.setInterval(2000);
  tUI.onRun(uiTask);           tUI.setInterval(100);
  controller.add(&tBlink);
  controller.add(&tJoy);
  controller.add(&tDist);
  controller.add(&tDHT);
  controller.add(&tUI);

  startBoot();

  wdt_enable(WDTO_4S);
}

// ---------------- Botón ----------------
// Devuelve true si se ha producido un cambio de modo/reinicio y el loop
// debe terminar esta vuelta
  bool pressed, longFired, releaseEv;
  unsigned long pStart, rDur;
  noInterrupts();
  pressed = btnPressed;
  longFired = btnLongFired;
  releaseEv = btnReleaseEvent;
  pStart = btnPressStart;
  rDur = btnReleaseDur;
  btnReleaseEvent = false;
  interrupts();

#if DEBUG
  if (releaseEv) {
    Serial.print(F("Boton soltado tras "));
    Serial.print(rDur);
    Serial.println(F(" ms"));
  }
#endif

  if (state != ST_BOOT) {
    // Entrada/Salida del modo admin
    if (pressed && !longFired && (long)(now - pStart) >= (long)ADMIN_MS) {
      noInterrupts(); btnLongFired = true; interrupts();
      if (adminMode) exitAdmin(); else enterAdmin();
      return true;
    }
    // Reinicio
    if (releaseEv && !longFired &&
        rDur >= RESET_MIN_MS && rDur < ADMIN_MS) {
      resetSystem();
      return true;
    }
  }
  return false;
}