#include <ESP32Servo.h>

const int Digital_Pin = 32;
const int Digital_Pin_2 = 25;
int Bandera = 0; // 1 = Límite alcanzado, 0 = Límite no alcanzado
int Carros = 0;
Servo SERVOM;

void CarrosNum(int Carros) {
  Serial.printf("Carros dentro del estacionamiento: %d\n", Carros);
}

void setup() {
  Serial.begin(9600);
  pinMode(Digital_Pin, INPUT);
  pinMode(Digital_Pin_2, INPUT);
  SERVOM.attach(17);
  SERVOM.write(20); // el servo inicia en 0°
}

void loop() {
  int Entrada = digitalRead(Digital_Pin);
  int Salida = digitalRead(Digital_Pin_2);

  if (!Bandera) { // Si todavía hay lugar
    if (!Entrada) { // Si un carro entró
      Serial.print("Carro entrando...\n");
      SERVOM.write(110);
      delay(4000); // esperamos a que el carro entre
      SERVOM.write(20);
      Carros++;
    }
  } else { // Ya no hay lugar
    Serial.print("Límite de carros alcanzado (no se permite entrar)\n");
  }

  if (!Salida && Carros != 0) { // Si un carro salió y es diferente de ceros
    Serial.print("Carro saliendo...\n");
    SERVOM.write(110);
    delay(4000); // esperamos a que el carro salga
    SERVOM.write(20);
    Carros--;
  }

  if (Carros == 6) { // Se restringe el acceso
    Bandera = 1;
  }

  if (Carros < 6 && Carros >= 0) { // Se concede el acceso
    Bandera = 0;
  }

  CarrosNum(Carros);
  delay(1000);
}