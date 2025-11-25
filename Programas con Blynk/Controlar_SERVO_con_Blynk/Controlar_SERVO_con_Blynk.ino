#define BLYNK_TEMPLATE_ID "TMPL2TlqCXvWe"
#define BLYNK_TEMPLATE_NAME "Control de un servo"
#define BLYNK_AUTH_TOKEN "KNJkf0ooNbMCwmVpnyaWAvzmBbBfCOMz"

// Librerías de Blynk y WiFi
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>

Servo SERVOM;
char ssid[] = "EB_WiFiB";
char password[] = "FamEB2023?";

#define LED_PIN 2 // LED del ESP32

BLYNK_WRITE(V0) {
  int value = param.asInt(); // 1 = ON, 0 = OFF

  if (value == 0) {
    SERVOM.write(0);
  } else {
    SERVOM.write(180);
  }

  digitalWrite(LED_PIN, value);

  Serial.print("SERVO = ");
  Serial.println(value);
}

void setup() {
  SERVOM.attach(17);
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  // Conexión a Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
}

void loop() {
  Blynk.run();
}