/*
Programa donde se incluye el monitoreo de la disponibilidad y
el control de entradas y salidas del estacionamiento.

Pines de la ESP32

- Entrada TCRT5000: GPIO22 -> D22
- Salida TCRT5000: GPIO21 -> D21
- Sensor HY-024 1: GPIO32 -> D32
- Sensor HY-024 2: GPIO33 -> D33
- Sensor HY-024 3: GPIO23 -> D23
- Sensor HY-024 4: GPIO19 -> D19
- Sensor HY-024 5: GPIO18 -> D18
- Sensor HY-024 6: GPIO16 -> RX2
- Micro servo: GPIO17 -> TX2

@author García Escamilla Bryan Alexis
@date 27/11/2025
*/

// ======> INFORMACIÓN PARA LA CONEXIÓN CON BLYNK <======
#define BLYNK_TEMPLATE_ID "TMPL27qOzqAiv"
#define BLYNK_TEMPLATE_NAME "Estacionamiento-IoT"
#define BLYNK_AUTH_TOKEN "uIDV6IpETIRlHTezdR6IIqrZpoJSGW1X"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>

// ======> OBJETOS <======
BlynkTimer timer;
Servo SERVOM;

// ======> CONSTANTES <======
#define LED_INDICADOR 2 // led de la esp32 que indica bloqueo del sistema
#define SENSOR_ENTRADA 21
#define SENSOR_SALIDA 22
#define MAX_CARROS 6

#define POS_CERRADO 20 // posición del servo cuando está cerrado (°)
#define POS_ABIERTO 90 // posición del servo cuando está abierto (°)

#define TIEMPO_ABIERTA 3000 // 3 segundos
#define LECTURAS_FILTRO 30 // 30 Lecturas * 100ms = 3 segundos

// ======> VARIABLES <======
char ssid[] = "EB_WiFiB";
char password[] = "FamEB2023?";

bool bloqueado = false; // para indicar cuando el sistema se bloquea o no

// filtros
int ultimo_estado_entrada = HIGH;
int contador_entrada = 0;

int ultimo_estado_salida = HIGH;
int contador_salida = 0;

// servo
int posicion_actual = POS_CERRADO;
int posicion_objetivo = POS_CERRADO;

// barrera
bool barrera_abierta = false;
unsigned long tiempo_apertura = 0;

// carros
int carros = 0;

// ======> FUNCIONES <======
void mover_servo() {
  if (posicion_actual < posicion_objetivo) { // se abre la barra
    posicion_actual += 2;
    if (posicion_actual > posicion_objetivo) posicion_actual = posicion_objetivo;
  }

  if (posicion_actual > posicion_objetivo) { // se cierra la barra
    posicion_actual -= 2;
    if (posicion_actual < posicion_objetivo) posicion_actual = posicion_objetivo;
  }

  SERVOM.write(posicion_actual); // se mueve el servo suvamente
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

void controlar_estacionamiento() {
  int lectura_entrada = digitalRead(SENSOR_ENTRADA);
  int lectura_salida = digitalRead(SENSOR_SALIDA);

  /*------------------.
  | SENSOR DE ENTRADA |
  `------------------*/
  if (!bloqueado) { // si el sistema no está bloqueado
    if (lectura_entrada == ultimo_estado_entrada) {
      contador_entrada++;
    } else {
      contador_entrada = 0;
    }

    if (contador_entrada >= LECTURAS_FILTRO) {
      if (lectura_entrada == LOW && !barrera_abierta) {
        abrir_barrera();
        Blynk.virtualWrite(V7, "Carro ENTRANDO...");

        if (carros < MAX_CARROS) {
          carros++;
        }

        if (carros >= MAX_CARROS) {
          bloqueado = true;
          digitalWrite(LED_INDICADOR, HIGH);
          Blynk.virtualWrite(V8, "Estacionamiento LLENO");
        }
      }

      contador_entrada = 0;
    }
  }

  ultimo_estado_entrada = lectura_entrada;

  /*-----------------.
  | SENSOR DE SALIDA |
  `-----------------*/
  if (lectura_salida == ultimo_estado_salida) {
    contador_salida++;
  } else {
    contador_salida = 0;
  }

  if (contador_salida >= LECTURAS_FILTRO) {
    if (lectura_salida == LOW && !barrera_abierta) {
      abrir_barrera();
      Blynk.virtualWrite(V7, "Carro SALIENDO...");

      if (carros > 0) {
        carros--;
      }

      if (bloqueado && carros < MAX_CARROS) {
        bloqueado = false;
        digitalWrite(LED_INDICADOR, LOW);
        Blynk.virtualWrite(V8, "Espacio disponible");
      }
    }

    contador_salida = 0;
  }

  ultimo_estado_salida = lectura_salida;

  /*------------------.
  | CIERRE AUTOMÁTICO |
  `------------------*/
  cierre_automatico();

  /*------.
  | BLYNK |
  `------*/
  Blynk.virtualWrite(V9, carros);

  /*------------.
  | SERVO MOTOR |
  `------------*/
  mover_servo();
}

BLYNK_WRITE(V6) {
  int estado = param.asInt(); // 1: bloquear, 0: desbloquear

  if (estado == 1) {
    bloqueado = true;
    posicion_objetivo = POS_CERRADO;
    digitalWrite(LED_INDICADOR, HIGH);
    Blynk.virtualWrite(V8, "Sistema BLOQUEADO (Manual)");
  } else {
    bloqueado = false;
    digitalWrite(LED_INDICADOR, LOW);
    Blynk.virtualWrite(V8, "Sistema DESBLOQUEADO (Manual)");
  }
}

void setup() {
  Serial.begin(115200);

  SERVOM.attach(17);
  pinMode(LED_INDICADOR, OUTPUT);

  pinMode(SENSOR_ENTRADA, INPUT);
  pinMode(SENSOR_SALIDA, INPUT);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  timer.setInterval(100L, controlar_estacionamiento);
}

void loop() {
  Blynk.run();
  timer.run();
}