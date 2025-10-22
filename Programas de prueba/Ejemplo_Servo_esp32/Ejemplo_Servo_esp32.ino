#include <ESP32Servo.h>

Servo SERVOM;

void setup() {
  SERVOM.attach(17);
}

void loop() {
  // put your main code here, to run repeatedly:
  SERVOM.write(0);
  delay(1000);
  SERVOM.write(180);
  delay(1000);
}
