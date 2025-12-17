/*
 * Programa del proyecto de 'Estacionamiento IoT'. Incluye el control de entrada/salida del
 * estacionamiento así como el monitoreo de los lugares del estacionamiento.
 *
 * Pines de la ESP32
 *
 * - Entrada TCRT5000: GPIO21 -> D21
 * - Salida TCRT5000: GPIO22 -> D22
 * - Sensor HY-024 1: GPIO32 -> D32
 * - Sensor HY-024 2: GPIO33 -> D33
 * - Sensor HY-024 3: GPIO23 -> D23
 * - Sensor HY-024 4: GPIO19 -> D19
 * - Sensor HY-024 5: GPIO18 -> D18
 * - Sensor HY-024 6: GPIO16 -> RX2
 * - Micro servo: GPIO17 -> TX2
 *
 * @author García Escamilla Bryan Alexis
 * @date 30/11/2025
 */
// ======> INFORMACIÓN PARA LA CONEXIÓN CON BLYNK <======
#define BLYNK_TEMPLATE_ID "TMPL27qOzqAiv"
#define BLYNK_TEMPLATE_NAME "Estacionamiento IoT"
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

#define TIEMPO_ABIERTA 5000 // 5 segundos apertura
#define LECTURAS_FILTRO 20 // 20 lecturas * 100ms = 2 segundos
#define TIEMPO_LECTURA_PRINC 100L // 100 ms para la lógica principal

// ======> SENSORES DE PLAZAS <======
const int sensores_pines[6] = {16, 33, 32, 19, 18, 23};
const int virtual_pines[6] = {V0, V1, V2, V3, V4, V5};
#define TIEMPO_LECTURA_PLAZAS 1000L // 1 segundo para actualizar LEDs en Blynk

// ======> VARIABLES <======
char ssid[] = "Bryan Alexis's Galaxy A33 5G";
char password[] = "vtwv7692";

// ======> BLOQUEOS <======
bool bloqueo_manual = false; // activado desde Blynk
bool bloqueo_auto = false; // activado al llenarse el estacionamiento
bool bloqueado = false; // bloqueo total (manual || auto)

// ======> filtros <======
int ultimo_estado_entrada = HIGH;
int contador_entrada = 0;

int ultimo_estado_salida = HIGH;
int contador_salida = 0;

// ======> servo <======
int posicion_actual = POS_CERRADO;
int posicion_objetivo = POS_CERRADO;

// ======> barrera <======
bool barrera_abierta = false;
unsigned long tiempo_apertura = 0;

// ======> carros <======
int carros = 0;

// ==========================================
// ========== FUNCIONES AUXILIARES ==========
// ==========================================

/*
 * Actualiza el bloqueo general dependiendo del bloqueo automático y del manual.s
 */
void actualizar_bloqueo_general() {
  bloqueado = (bloqueo_manual || bloqueo_auto);
  digitalWrite(LED_INDICADOR, bloqueado ? HIGH : LOW);
}

/*
 * Mueve el servo de manera suave, depediendo de si la barra debe cerrar o abrir. 
 */
void mover_servo() {
  if (posicion_actual < posicion_objetivo) {
    posicion_actual += 4;

    if (posicion_actual > posicion_objetivo) posicion_actual = posicion_objetivo;
  }

  if (posicion_actual > posicion_objetivo) {
    posicion_actual -= 4;

    if (posicion_actual < posicion_objetivo) posicion_actual = posicion_objetivo;
  }

  SERVOM.write(posicion_actual);
}

/*
 * Indica que la barrera debe abrir, así como el tiempo que permanecerá abierta.
 */
void abrir_barrera() {
  posicion_objetivo = POS_ABIERTO;
  barrera_abierta = true;
  tiempo_apertura = millis();
}

/*
 * Cierra automáticamente de la barra después de que un vehículo sale o entra.
*/
void cierre_automatico() {
  if (barrera_abierta && (millis() - tiempo_apertura >= TIEMPO_ABIERTA)) {
    posicion_objetivo = POS_CERRADO;
    barrera_abierta = false;
    Blynk.virtualWrite(V7, "Sin cambios");
  }
}

// =======================================
// ========== LECTURA DE PLAZAS ==========
// =======================================

/*
 * Lee el valor de cada uno de los lugares del estacionamiento y escribe su estado en Blynk.
 */
void leer_sensores_plazas() {
  for (int i = 0; i < 6; i++) {
    int lectura = digitalRead(sensores_pines[i]);
    Blynk.virtualWrite(virtual_pines[i], lectura);
  }
}

// =======================================
// ========== CONTROL PRINCIPAL ==========
// =======================================

/*
 * Controla la entrada/salida del estacionamiento, así como el bloqueo de este (bloqueo manual y automático).
 */
void controlar_estacionamiento() {
  int lectura_entrada = digitalRead(SENSOR_ENTRADA);
  int lectura_salida = digitalRead(SENSOR_SALIDA);

  // -------------------- ENTRADA --------------------
  if (lectura_entrada == LOW) {
    contador_entrada ++;
  } else {
    contador_entrada = 0;
  }

  if (!bloqueado && contador_entrada >= LECTURAS_FILTRO) {
    if (!barrera_abierta) {
      abrir_barrera();
      Blynk.virtualWrite(V7, "Vehículo ENTRANDO...");
    }

    if (carros < MAX_CARROS) carros++;

    if (carros >= MAX_CARROS) {
      bloqueo_auto = true;
      Blynk.virtualWrite(V8, "Estacionamiento LLENO"); 
    }

    actualizar_bloqueo_general();

    contador_entrada = 0;
  }

  ultimo_estado_entrada = lectura_entrada;

  // -------------------- SALIDA --------------------
  if (lectura_salida == LOW) {
    contador_salida++;
  } else {
    contador_salida = 0;
  }

  if (contador_salida >= LECTURAS_FILTRO) {
    if (!barrera_abierta) {
      abrir_barrera();
      Blynk.virtualWrite(V7, "Vehículo SALIENDO...");
    }

    if (carros > 0) carros--;

    if (bloqueo_auto && carros < MAX_CARROS) {
      bloqueo_auto = false;
      Blynk.virtualWrite(V8, "Espacio disponible");
    }

    actualizar_bloqueo_general();

    contador_salida = 0;
  }

  ultimo_estado_salida = lectura_salida;

  // -------------------- CIERRE, BLYNK Y SERVO --------------------
  cierre_automatico();
  Blynk.virtualWrite(V9, carros);
  mover_servo();
}

// -------------------- BLYNK MANUAL LOCK --------------------
/*
 * Recibe datos desde Blynk, para el bloqueo manual. 
 */
BLYNK_WRITE(V6) {
  int estado = param.asInt(); // 1: bloquear, 0: desbloquear

  if (estado == 1) {
    bloqueo_manual = true;
    Blynk.virtualWrite(V8, "Entrada BLOQUEADA (Manual)");
  } else {
    bloqueo_manual = false;
    if (bloqueo_auto) Blynk.virtualWrite(V8, "Estacionamiento LLENO");
    else Blynk.virtualWrite(V8, "Entrada DESBLOQUEADA (Manual)");
  }

  actualizar_bloqueo_general();
}

void setup() {
  Serial.begin(115200);

  SERVOM.attach(17);
  pinMode(LED_INDICADOR, OUTPUT);
  pinMode(SENSOR_ENTRADA, INPUT);
  pinMode(SENSOR_SALIDA, INPUT);

  for (int i = 0; i < 6; i++) {
    pinMode(sensores_pines[i], INPUT);
  }

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);

  timer.setInterval(TIEMPO_LECTURA_PRINC, controlar_estacionamiento);
  timer.setInterval(TIEMPO_LECTURA_PLAZAS, leer_sensores_plazas);

  actualizar_bloqueo_general();
}

void loop() {
  Blynk.run();
  timer.run();
}