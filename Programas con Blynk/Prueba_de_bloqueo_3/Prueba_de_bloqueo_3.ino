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
bool disponibilidad = true;

// para el filtro del TCRT5000
int ultimo_estado_entrada = HIGH;
int contador_entrada = 0;
int ultimo_estado_salida = HIGH;
int contador_salida = 0;

// para el servo
int posicion_actual = 20; // posición real del servo
int posicion_objetivo = 20; // a donde se debe ir el servo

int carros = 0;

#define LED_INDICADOR 2
#define SENSOR_ENTRADA 21
#define SENSOR_SALIDA 22
#define MAX_CARROS 6

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

void entrada(int lectura_entrada) {
  // filtro por repeticiones para el servo
  if (lectura_entrada == ultimo_estado_entrada) {
    contador_entrada++;
  } else {
    contador_entrada = 0;
  }

  if (contador_entrada >= 3) { // 3 lecturas seguidas
    if (lectura_entrada == LOW) {
      posicion_objetivo = 110;
      Blynk.virtualWrite(V7, "Carro entrando...");

      if (carros < MAX_CARROS) {
        carros++;
      }
    } else {
      posicion_objetivo = 20;
      Blynk.virtualWrite(V7, "Sin cambios");
    }
  }

  ultimo_estado_entrada = lectura_entrada;
}

void salida(int lectura_salida) {
  if (!carros <= MAX_CARROS) {
    return;
  }
  // filtro por repeticiones para el servo
  if (lectura_salida == ultimo_estado_salida) {
    contador_salida++;
  } else {
    contador_salida = 0;
  }

  if (contador_salida >= 3) { // 3 lecturas seguidas
    if (lectura_salida == LOW) {
      posicion_objetivo = 110;
      Blynk.virtualWrite(V7, "Carro saliendo...");
    } else {
      posicion_objetivo = 20;
      Blynk.virtualWrite(V7, "Sin cambios");
    }
  }

  ultimo_estado_salida = lectura_salida;
}

void control_estacionamiento() {
  if (bloqueado) { // si esta bloqueado no hace nada
    posicion_objetivo = 20; // siempre que se bloquea va a 0
    mover_servo();
    
    return;
  }

  int lectura_entrada = digitalRead(SENSOR_ENTRADA);
  int lectura_salida = digitalRead(SENSOR_SALIDA);
  
  entrada(lectura_entrada);
  salida(lectura_salida);

  mover_servo();
}

BLYNK_WRITE(V6) { // 0
  int estado = param.asInt(); // 1: bloquear, 0: desbloquear

  if (estado == 1) {
    bloqueado = true;
    posicion_objetivo = 20; // vuelve a posición segura
    digitalWrite(LED_INDICADOR, HIGH);
    Blynk.virtualWrite(V8, "Sistema bloqueado");
  } else {
    bloqueado = false;
    digitalWrite(LED_INDICADOR, LOW);
    Blynk.virtualWrite(V8, "Sistema desbloqueado");
  }
}

void setup() {
  // inicialización de los pines
  SERVOM.attach(17);
  pinMode(LED_INDICADOR, OUTPUT);
  pinMode(SENSOR_ENTRADA, INPUT);
  pinMode(SENSOR_SALIDA, INPUT);
  Serial.begin(115200);

  // blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  timer.setInterval(50L, control_estacionamiento);
}

void loop() {
  Blynk.run();
  timer.run();
}