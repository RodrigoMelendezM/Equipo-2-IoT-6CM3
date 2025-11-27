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
@date 23/11/2025
*/

// información para la conexión con Blynk
#define BLYNK_TEMPLATE_ID "TMPL27qOzqAiv"
#define BLYNK_TEMPLATE_NAME "Prueba de KY 024"
#define BLYNK_AUTH_TOKEN "uIDV6IpETIRlHTezdR6IIqrZpoJSGW1X"
#define BLYNK_FIRMWARE_VERSION "0.1.0"
#define BLYNK_PRINT Serial

#include <WiFi.h> // para el WiFi de ESP32
#include <BlynkSimpleEsp32.h> // uso la librería Blynk para ESP32
#include <ESP32Servo.h> // para usar el Servo con ESP32

const int entrada_tcrt5000 = 22;
const int salida_tcrt5000 = 21;

bool limite_alcanzado = false; // true: Límite alcanzado, false: Límite no alcanzado
int carros = 0;

const int sensores_pines[6] = {32, 33, 23, 19, 18, 16};
const int virtual_pines[6] = {V0, V1, V2, V3, V4, V5};

char ssid[] = "EB_WiFiB";
char pass[] = "FamEB2023?";

// objetos
BlynkTimer timer;
Servo SERVOM;

/*
 * Imprime el número de carros actualmente en el estacionamiento.
 */
void num_carros() {
  Serial.printf("Carros dentro del estacionamiento: %d\n", carros);
}

/*
 * Controla el acceso y salida de los carros del estacionamiento.
 */
void control_acceso() {
  int entrada = digitalRead(entrada_tcrt5000); // 1: no detecta carro, 0: detecta carro
  int salida = digitalRead(salida_tcrt5000); // 1: no detecta carro, 0: detecta carro

  if (!limite_alcanzado) { // si todavía hay lugar
    if (!entrada) { // si un carro entró
      Serial.print("Carro entrando...\n");
      SERVOM.write(110);
      delay(4000);
      SERVOM.write(20);
      carros++;
    }
  } else { // ya no hay lugar
    Serial.print("Límite de carros alcanzado (acceso negado)\n");
  }

  if (!salida && carros != 0) { // si un carro salió y queda al menos 1
    Serial.print("Carro saliendo...\n");
    SERVOM.write(110);
    delay(4000);
    SERVOM.write(20);
    carros--;
  }

  if (carros == 6) { // límite de carros alcanzado
    limite_alcanzado == true; // se restringe el acceso
  }

  if (carros < 6 && carros >= 0) { // se concede el acceso
    limite_alcanzado = false;
  }

  num_carros();
  delay(1000);
}

void setup() {
  Serial.begin(9600);

  // sensorres de entrada y salida
  pinMode(entrada_tcrt5000, INPUT);
  pinMode(salida_tcrt5000, INPUT);

  // micro servo
  SERVOM.attach(17);
  SERVOM.write(20); // simula el inicio en 0°

}

void loop() {
  int bloqueo = 0; // 1: estacionamiento bloqueado, 0: caso contrario

}