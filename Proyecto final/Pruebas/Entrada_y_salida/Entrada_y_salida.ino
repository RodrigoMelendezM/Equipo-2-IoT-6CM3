/*
Programa donde se incluye el monitoreo de la disponibilidad y
el control de entradas y salidas del estacionamiento.

Pines de la ESP32

- Entrada TCRT5000: GPIO21 -> D21
- Salida TCRT5000: GPIO22 -> D22
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
#define POS_ABIERTO 110 // posición del servo cuando está abierto (°)

#define TIEMPO_ABIERTA 5000 // 5 segundos
#define LECTURAS_FILTRO 20 // 20 Lecturas * 100ms = 2 segundos

// ======> VARIABLES <======
char ssid[] = "EB_WiFiB";
char password[] = "FamEB2023?";

// ======> BLOQUEOS <======
bool bloqueo_manual = false; // activado desde Blynk
bool bloqueo_auto = false; // activado al llenars el estacionamiento
bool bloqueado = false; // bloqueo total (manual || auto)

// ======> filtros
int ultimo_estado_entrada = HIGH; // por defecto está en alto
int contador_entrada = 0;

int ultimo_estado_salida = HIGH; // por defecto está en alto
int contador_salida = 0;

// ======> servo
int posicion_actual = POS_CERRADO; // por defecto esta cerrado
int posicion_objetivo = POS_CERRADO; // por defecto esta cerrado

// ======> barrera
bool barrera_abierta = false; // por defecto permanece cerrada
unsigned long tiempo_apertura = 0;

// ======> carros
int carros = 0;

// =========================================
//           FUNCIONES PRINCIPALES          
// =========================================

void actualizar_bloqueo_general() {
  bloqueado = (bloqueo_manual || bloqueo_auto);
  digitalWrite(LED_INDICADOR, bloqueado ? HIGH : LOW);
}

void mover_servo() {
  if (posicion_actual < posicion_objetivo) { // se abre la barra
    posicion_actual += 4;
    if (posicion_actual > posicion_objetivo) posicion_actual = posicion_objetivo;
  }

  if (posicion_actual > posicion_objetivo) { // se cierra la barra
    posicion_actual -= 4;
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
    Blynk.virtualWrite(V7, "Sin cambios");
  }
}

// =====================================
//           CONTROL PRINCIPAL          
// =====================================
void controlar_estacionamiento() {
  // lee el valor de ambos sensores
  int lectura_entrada = digitalRead(SENSOR_ENTRADA);
  int lectura_salida = digitalRead(SENSOR_SALIDA);

  /*------------------.
  | SENSOR DE ENTRADA |
  `------------------*/
  if (!bloqueado) { // si el sistema esta bloqueado (general)
    if (lectura_entrada == ultimo_estado_entrada) contador_entrada++; // esta en alto
    else contador_entrada = 0; // esta en bajo

    if (contador_entrada >= LECTURAS_FILTRO) { // si las lecturas son las suficientes
      if (lectura_entrada == LOW && !barrera_abierta) { // barrera está cerrada
        abrir_barrera(); // se indica que la barrera se abrirá
        Blynk.virtualWrite(V7, "Carro ENTRANDO...");

        if (carros < MAX_CARROS) carros++; // si no se ha llenado el estacionamiento, se cuenta el carro

        if (carros >= MAX_CARROS) { // si se se llenó el estaciomiento
          bloqueo_auto = true; // se bloquea el estacionamiento
          Blynk.virtualWrite(V8, "Estacionamiento LLENO");
        }

        actualizar_bloqueo_general();
      }

      contador_entrada = 0; // se reinician las lecturas del filtro
    }
  }

  ultimo_estado_entrada = lectura_entrada; // se ectualiza el estado de la lectura

  /*-----------------.
  | SENSOR DE SALIDA |
  `-----------------*/
  if (lectura_salida == ultimo_estado_salida) contador_salida++; // esta en alto
  else contador_salida = 0; // esta en bajo

  if (contador_salida >= LECTURAS_FILTRO) { // si las lecturas son las suficientes
    if (lectura_salida == LOW && !barrera_abierta) { // barrera está cerrada
      abrir_barrera(); // se indica que la barrera se abrirá
      Blynk.virtualWrite(V7, "Carro SALIENDO...");

      if (carros > 0) carros--; // si no se ha vaciado el estacionamiento

      if (bloqueo_auto && carros < MAX_CARROS) { // si el estacionamiento está bloqueado y no se ha llenado
        bloqueo_auto = false; // se desbloquea el estacionamiento (auto)
        Blynk.virtualWrite(V8, "Espacio disponible");
      }

      actualizar_bloqueo_general();
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

// =====================================
//           BLYNK MANUAL LOCK          
// =====================================
BLYNK_WRITE(V6) {
  int estado = param.asInt(); // 1: bloquear, 0: desbloquear

  if (estado == 1) {
    bloqueo_manual = true;
    Blynk.virtualWrite(V8, "Sistema BLOQUEADO (Manual)");
  } else {
    bloqueo_manual = false;
    
    if (bloqueo_auto) {
      Blynk.virtualWrite(V8, "Estacionamiento LLENO");
    } else {
      Blynk.virtualWrite(V8, "Sistema DESBLOQUEADO (Manual)");
    }
  }

  actualizar_bloqueo_general();
}

void setup() {
  Serial.begin(115200);

  SERVOM.attach(17);
  pinMode(LED_INDICADOR, OUTPUT);

  pinMode(SENSOR_ENTRADA, INPUT);
  pinMode(SENSOR_SALIDA, INPUT);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  timer.setInterval(100L, controlar_estacionamiento);
  actualizar_bloqueo_general(); 
}

void loop() {
  Blynk.run();
  timer.run();
}