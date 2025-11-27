//#define BLYNK_TEMPLATE_ID "TMPL2TlqCXvWe"
//#define BLYNK_TEMPLATE_NAME "Control de un servo"
//#define BLYNK_AUTH_TOKEN "KNJkf0ooNbMCwmVpnyaWAvzmBbBfCOMz"

#define BLYNK_TEMPLATE_ID "TMPL27qOzqAiv"
#define BLYNK_TEMPLATE_NAME "Estacionamiento-IoT"
#define BLYNK_AUTH_TOKEN "uIDV6IpETIRlHTezdR6IIqrZpoJSGW1X"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>

// objetos
BlynkTimer timer;
Servo SERVOM;

// variables globales
char ssid[] = "POCO C65";
char password[] = "jasonmomoa";

bool bloqueado = false;

// para el filtro del TCRT5000
int ultimo_estado = HIGH;
int contador = 0;

// para el servo
int posicion_actual = 20; // posición real del servo
int posicion_objetivo = 20; // a donde se debe ir el servo

#define led_pin 2
#define sensor_pin 21

void mover_servo() {
  if (posicion_actual < posicion_objetivo) {
    posicion_actual += 5; // velocidad de subida
    if (posicion_actual > posicion_objetivo) posicion_actual = posicion_objetivo; // llego al objetivo
  }

  if (posicion_actual > posicion_objetivo) {
    posicion_actual -= 5; // velocidad de bajada
    if (posicion_actual < posicion_objetivo) posicion_actual = posicion_objetivo; // llego al objetivo
  }

  SERVOM.write(posicion_actual);
}

void accion() {
  if (bloqueado) {
    posicion_objetivo = 20; // siempre que se bloquea va a 0
    mover_servo();
    
    return;
  }

  int lectura = digitalRead(sensor_pin);

  // filtro por repeticiones para el servo
  if (lectura == ultimo_estado) {
    contador++;
  } else {
    contador = 0;
  }

  if (contador >= 3) { // 3 lecturas seguidas
    if (lectura == LOW) {
      posicion_objetivo = 110;
      Blynk.virtualWrite(V7, "Objeto detectado");
    } else {
      posicion_objetivo = 20;
      Blynk.virtualWrite(V7, "No detectado");
    }
  }

  ultimo_estado = lectura;

  mover_servo();
}

BLYNK_WRITE(V6) { // 0
  int estado = param.asInt(); // 1: bloquear, 0: desbloquear

  if (estado == 1) {
    bloqueado = true;
    posicion_objetivo = 20; // vuelve a posición segura
    digitalWrite(led_pin, HIGH);
    Blynk.virtualWrite(V8, "Sistema bloqueado");
  } else {
    bloqueado = false;
    digitalWrite(led_pin, LOW);
    Blynk.virtualWrite(V8, "Sistema desbloqueado");
  }
}

void setup() {
  // inicialización de los pines
  SERVOM.attach(17);
  pinMode(led_pin, OUTPUT);
  pinMode(sensor_pin, INPUT);
  Serial.begin(115200);

  // blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  timer.setInterval(50L, accion);
}

void loop() {
  Blynk.run();
  timer.run();
}