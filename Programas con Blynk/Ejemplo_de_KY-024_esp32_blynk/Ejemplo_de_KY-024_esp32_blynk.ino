// Esta información se saca de la misma aplicación Blynk

#define BLYNK_TEMPLATE_ID "TMPL27qOzqAiv"
#define BLYNK_TEMPLATE_NAME "Prueba de KY 024"
#define BLYNK_AUTH_TOKEN "uIDV6IpETIRlHTezdR6IIqrZpoJSGW1X"
#define BLYNK_FIRMWARE_VERSION "0.1.0"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>   // Uso la librería Blynk para ESP32

#define DIGITAL_PIN 32

BlynkTimer timer;

//char ssid[] = "EB_WiFiB";
//char pass[] = "FamEB2023?";
char ssid[] = "Bryan Alexis's Galaxy A33 5G";
char pass[] = "vtwv7692";
//char ssid[] = "Wi-Fi IPN";
//char pass[] = "";

void read_sensor() {
  int digital_value = digitalRead(DIGITAL_PIN);   // lectura del pin digital
  Blynk.virtualWrite(V0, digital_value);          // envía el valor a V0

  if (digital_value == HIGH) {
    Serial.println("Carro estacionado");
  } else {
    Serial.println("Lugar disponible");
  }
  Serial.print("Valor raw: ");
  Serial.println(digital_value);
}

void setup() {
  Serial.begin(115200);

  pinMode(DIGITAL_PIN, INPUT);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(1000L, read_sensor);
}

void loop() {
  Blynk.run();
  timer.run();
}