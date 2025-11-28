//#define BLYNK_TEMPLATE_ID "TMPL2TlqCXvWe"
//#define BLYNK_TEMPLATE_NAME "Control de un servo"
//#define BLYNK_AUTH_TOKEN "KNJkf0ooNbMCwmVpnyaWAvzmBbBfCOMz"

#define BLYNK_TEMPLATE_ID "TMPL27qOzqAiv"
#define BLYNK_TEMPLATE_NAME "Estacionamiento-IoT"
#define BLYNK_AUTH_TOKEN "uIDV6IpETIRlHTezdR6IIqrZpoJSGW1X"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>

// ---------- OBJETOS ----------
BlynkTimer timer;
Servo SERVOM;

// ---------- CONSTANTES ----------
#define LED_INDICADOR 2
#define SENSOR_ENTRADA 21
#define MAX_CARROS 6

#define POS_CERRADO 20
#define POS_ABIERTO 110

#define TIEMPO_ABIERTA 3000 // 3 segundos
#define LECTURAS_FILTRO 30 // 30 lecturas * 100ms = 3 segundos

// ---------- VARIABLES ----------
char ssid[] = "EB_WiFiB";
char password[] = "FamEB2023?";

bool bloqueado = false;

// filtro sensor
int ultimo_estado = HIGH;
int contador = 0;

// servo
int posicion_actual = POS_CERRADO;
int posicion_objetivo = POS_CERRADO;

// barrera
bool barrera_abierta = false;
unsigned long tiempo_apertura = 0;

// carros
int carros = 0;

// ---------- FUNCIONES ----------

void mover_servo() { 
  if (posicion_actual < posicion_objetivo) {
    posicion_actual += 2;
    if (posicion_actual > posicion_objetivo) posicion_actual = posicion_objetivo;
  }

  if (posicion_actual > posicion_objetivo) {
    posicion_actual -= 2;
    if (posicion_actual < posicion_objetivo) posicion_actual = posicion_objetivo;
  }

  SERVOM.write(posicion_actual);
}

void abrir_barrera() {
  posicion_objetivo = POS_ABIERTO;
  barrera_abierta = true;
  tiempo_apertura = millis();
}

void cierre_automatico() {
  if (barrera_abierta && (millis() - tiempo_apertura >= TIEMPO_ABIERTA)) {
    posicion_objetivo = POS_CERRADO;
    barrera_abierta = false;
  }
}

void control_estacionamiento() {

  if (bloqueado) {
    posicion_objetivo = POS_CERRADO;
    mover_servo();
    return;
  }

  int lectura = digitalRead(SENSOR_ENTRADA);

  // --------- FILTRO ----------
  if (lectura == ultimo_estado) {
    contador++;
  } else {
    contador = 0;
  }

  if (contador >= LECTURAS_FILTRO) {

    if (lectura == LOW && !barrera_abierta) {
      abrir_barrera();
      Blynk.virtualWrite(V7, "Carro detectado");

      if (carros < MAX_CARROS) {
        carros++;
      }

      if (carros >= MAX_CARROS) {
        bloqueado = true;
        digitalWrite(LED_INDICADOR, HIGH);
        Blynk.virtualWrite(V8, "Estacionamiento lleno");
      }
    }

    contador = 0; // reiniciamos después de accionar
  }

  ultimo_estado = lectura;

  // ---------- CIERRE ----------
  cierre_automatico();

  // ---------- ENVÍO A BLYNK ----------
  Blynk.virtualWrite(V9, carros);

  // ---------- MOVER SERVO ----------
  mover_servo();
}

// --------- BLOQUEO MANUAL ---------
BLYNK_WRITE(V6) {
  int estado = param.asInt(); // 1: bloquear, 0: desbloquear

  if (estado == 1) {
    bloqueado = true;
    posicion_objetivo = POS_CERRADO;
    digitalWrite(LED_INDICADOR, HIGH);
    Blynk.virtualWrite(V8, "Sistema BLOQUEADO (Manual)");
  } 
  else {
    bloqueado = false;
    digitalWrite(LED_INDICADOR, LOW);
    Blynk.virtualWrite(V8, "Sistema DESBLOQUEADO (Manual)");
  }
}

// ---------- SETUP ----------
void setup() {
  Serial.begin(115200);

  SERVOM.attach(17);
  pinMode(LED_INDICADOR, OUTPUT);
  pinMode(SENSOR_ENTRADA, INPUT);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  timer.setInterval(100L, control_estacionamiento);
}

// ---------- LOOP ----------
void loop() {
  Blynk.run();
  timer.run();
}