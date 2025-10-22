#include <Wire.h>

void setup() {
  Wire.begin(21,22); // SDA, SCL
  Serial.begin(9600);
  delay(1000);
  Serial.println("\nI2C Scanner");

}

void loop() {
  byte error, address;
  int nDevices = 0;

  Serial.println("Bus scan...");

  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("Found I2C device at 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.println("  !");
      nDevices++;
    }
  }

  if (nDevices == 0) Serial.println("No I2C devices found\n");
  else Serial.println("Done\n");
  delay(2000);
}
