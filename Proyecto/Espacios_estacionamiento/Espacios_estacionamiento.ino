// Esta información se saca de la misma aplicación Blynk

#define BLYNK_TEMPLATE_ID "TMPL27qOzqAiv"
#define BLYNK_TEMPLATE_NAME "Prueba de KY 024"
#define BLYNK_AUTH_TOKEN "uIDV6IpETIRlHTezdR6IIqrZpoJSGW1X"
#define BLYNK_FIRMWARE_VERSION "0.1.0"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>   // Uso la librería Blynk para ESP32

BlynkTimer timer;

//char ssid[] = "EB_WiFiB";
//char pass[] = "FamEB2023?";
char ssid[] = "Bryan Alexis's Galaxy A33 5G";
char pass[] = "vtwv7692";
//char ssid[] = "WiFi-IPN";
//char pass[] = ""; // Sin contraseña

const int sensores_pines[6] = {32, 33, 23, 19, 18, 16};
const int virtual_pines[6] = {V0, V1, V2, V3, V4, V5};

void read_sensores() {
  for (int i = 0; i < 6; i++) {
    int digital_value = digitalRead(sensores_pines[i]); // Lectura de los sensores
    Blynk.virtualWrite(virtual_pines[i], digital_value); // Envía la lectura a Blynk (pines virtuales)

    if (digital_value == HIGH) {
      Serial.print("\nSensor ");
      Serial.print(i);
      Serial.print(": Carro estacionado");
    } else {
      Serial.print("\nSensor ");
      Serial.print(i);
      Serial.print(": Lugar disponible");
    }
  }

  Serial.print("\n------------------------");
}

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < 6; i++) {
    pinMode(sensores_pines[i], INPUT); // Inicializa las pines de la ESP32 para los sensores
  }

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(1000L, read_sensores);
}

void loop() {
  Blynk.run();
  timer.run();
}