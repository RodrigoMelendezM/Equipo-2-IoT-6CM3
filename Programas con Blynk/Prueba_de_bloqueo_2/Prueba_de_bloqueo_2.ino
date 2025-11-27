//#define BLYNK_TEMPLATE_ID "TMPL2TlqCXvWe"
//#define BLYNK_TEMPLATE_NAME "Control de un servo"
//#define BLYNK_AUTH_TOKEN "KNJkf0ooNbMCwmVpnyaWAvzmBbBfCOMz"

#define BLYNK_TEMPLATE_ID "TMPL27qOzqAiv"
#define BLYNK_TEMPLATE_NAME "Estacionamiento-IoT"
#define BLYNK_AUTH_TOKEN "uIDV6IpETIRlHTezdR6IIqrZpoJSGW1X"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>

// ------ objetos ------
BlynkTimer timer;
Servo SERVOM;

// ------ para el WiFi ------
char ssid[] = "EB_WiFiB";
char password[] = "FamEB2023?";

// ------ constantes ------
#define LED_INDICADOR 2
#define SENSOR_ENTRADA 21 // tcrt5000 de entrada
#define SENSOR_SALIDA 22 // trt5000 de salida
#define MAX_CARROS 6

// ------ variables ------
bool bloqueado = false;
int carros = 0;

// para el filtro del TCRT5000
int ultimo_estado_entrada = HIGH;
int contador_entrada = 0;
int ultimo_estado_salida = HIGH;
int contador_salida = 0;

// para el servo
int posicion_actual = 20; // posición real del servo
int posicion_objetivo = 20; // a donde se debe ir el servo

// para el tiempo de la barrera
bool barrera_abierta = false;
unsigned long tiempo_apertura = 0;
#define TIEMPO_ABIERTA 3000  // 3 segundos

void mover_servo() {
  if (posicion_actual < posicion_objetivo) {
    posicion_actual += 2; // velocidad de subida
    if (posicion_actual > posicion_objetivo) posicion_actual = posicion_objetivo; // llego al objetivo
  }

  if (posicion_actual > posicion_objetivo) {
    posicion_actual -= 2; // velocidad de bajada
    if (posicion_actual < posicion_objetivo) posicion_actual = posicion_objetivo; // llego al objetivo
  }

  SERVOM.write(posicion_actual);
}

void control_estacionamiento() {
  int lectura_entrada = digitalRead(SENSOR_ENTRADA);
  int lectura_salida = digitalRead(SENSOR_SALIDA);

  // ------ filtro entrada ------
  if (lectura_entrada == ultimo_estado_entrada) {
    contador_entrada++;
  } else {
    contador_entrada = 0;
  }

  // Si estuvo 3 segundos en LOW → entra un carro
  if (contador_entrada >= 30 && lectura_entrada == LOW && !bloqueado) {
    posicion_objetivo = 110;
    barrera_abierta = true;
    tiempo_apertura = millis();
    
    if (carros < MAX_CARROS) {
      carros++;
    }
    
    Blynk.virtualWrite(V7, "Carro entrando...");

    if (carros >= MAX_CARROS) {
      carros = MAX_CARROS;
      bloqueado = true;
      Blynk.virtualWrite(V8, "Sistema BLOQUEADO");
      digitalWrite(LED_INDICADOR, HIGH);
    }

    contador_entrada = 0; // evita multi conteo
  }

  ultimo_estado_entrada = lectura_entrada;

  // ------ filtro de salida ------
  if (lectura_salida == ultimo_estado_salida) {
    contador_salida++;
  } else {
    contador_salida = 0;
  }

  // Si estuvo 3 segundos en LOW → sale un carro
  if (contador_salida >= 30 &&  lectura_salida == LOW) {
    posicion_objetivo = 110;
    barrera_abierta = true;
    tiempo_apertura = millis();

    if (carros > 0) {
      carros--;
    }

    if (bloqueado && carros < MAX_CARROS) {
      bloqueado = false;
      Blynk.virtualWrite(V8, "Acceso PERMITIDO");
      digitalWrite(LED_INDICADOR, LOW);
    }

    Blynk.virtualWrite(V7, "Carro saliendo...");
    contador_salida = 0; // evita multi conteo
  }

  ultimo_estado_salida = lectura_salida;

  // ------ bloqueo total y mantiene cerrado ------
  if (bloqueado && !barrera_abierta) {
    posicion_objetivo = 20;
  }

  // ------ enviar contador ------
  Blynk.virtualWrite(V9, carros);

  mover_servo();
}

// bloquear manualmente
BLYNK_WRITE(V6) {
  int estado = param.asInt(); // 1: bloquear, 0: desbloquear

  if (estado == 1) {
    bloqueado = true;
    posicion_objetivo = 20; // vuelve a posición segura
    digitalWrite(LED_INDICADOR, HIGH);
    Blynk.virtualWrite(V8, "Sistema BLOQUEADO (Manual)");
  } else {
    bloqueado = false;
    digitalWrite(LED_INDICADOR, LOW);
    Blynk.virtualWrite(V8, "Sistema DESBLOQUEADO (Manual)");
  }
}

void setup() {
  // inicialización de los pines
  SERVOM.attach(17);
  pinMode(LED_INDICADOR, OUTPUT);
  pinMode(SENSOR_ENTRADA, INPUT);
  pinMode(SENSOR_SALIDA, INPUT);

  // blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  timer.setInterval(100L, control_estacionamiento);
}

void loop() {
  Blynk.run();
  timer.run();
}