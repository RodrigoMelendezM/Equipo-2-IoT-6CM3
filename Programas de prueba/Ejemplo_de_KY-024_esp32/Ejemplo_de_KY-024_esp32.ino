// Variables globales

/* 
Cuando esta de frente el sensor, se calida hacia la derecha (cuado ya casi se esta apagando el led)

Para salida digital = HIGH (carro estacionado), LOW (lugar disponible)
Para salida analogica < 520 (carro estacionado), > 520 (lugar disponible)

*/

//const int digitalPin = 32;
const int analogPin = 33;
int analogValue = 0;
//int digitalValue = 0;

void setup() {
  Serial.begin(9600);
  //pinMode(digitalPin, INPUT);
}

void loop() {
  analogValue = analogRead(33);
  //digitalValue = digitalRead(digitalPin);
  
  if (analogValue < 520) {
    Serial.print("Carro estacionado (ANALOG): "); 
  } else {
    Serial.print("Lugar disponible (ANALOG): ");
  }
  
  Serial.println(analogValue);

  //if (digitalValue == HIGH) {
  //  Serial.print("Carro estacionado (DIGITAL): ");
  //} else {
  //  Serial.print("Lugar disponible (DIGITAL): ");
  //}

  //Serial.println(digitalValue);

  delay(1000);
}
