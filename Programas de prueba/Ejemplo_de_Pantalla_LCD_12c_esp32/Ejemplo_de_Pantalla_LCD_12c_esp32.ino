#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>

const uint8_t LCD_ADDR = 0x27; // ajusta si tu dirección es otra
LiquidCrystal_PCF8574 lcd(LCD_ADDR);

void scanI2C() {
  Serial.println("Scan I2C...");
  bool found = false;
  for (uint8_t address = 1; address < 127; ++address) {
    Wire.beginTransmission(address);
    uint8_t err = Wire.endTransmission();
    if (err == 0) {
      Serial.print("  Found device at 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.println();
      found = true;
    }
  }
  if (!found) Serial.println("  No devices found");
}

bool tryInitLCD() {
  // intentamos init y escritura básica
  lcd.begin(16, 2);
  lcd.setBacklight(255);
  delay(50);
  lcd.clear();
  lcd.home();
  lcd.print("Inicializando...");
  delay(200);
  // comprobamos lectura rápida del bus para ver si hay respuesta
  Wire.beginTransmission(LCD_ADDR);
  uint8_t err = Wire.endTransmission();
  if (err == 0) {
    Serial.println("LCD inicializada OK");
    return true;
  } else {
    Serial.print("LCD init fallo, err=");
    Serial.println(err);
    return false;
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22); // SDA, SCL
  // Fuerza I2C a 100kHz (más estable para muchos módulos)
  Wire.setClock(100000);
  delay(100);
  Serial.println("Arrancando diagnóstico LCD I2C");
  scanI2C();

  // Intentos de inicialización con reintentos
  const int MAX_RETRIES = 5;
  int tries = 0;
  bool ok = false;
  while (tries < MAX_RETRIES && !ok) {
    Serial.print("Intento init ");
    Serial.println(tries + 1);
    ok = tryInitLCD();
    if (!ok) {
      delay(200);
      tries++;
    }
  }
  if (!ok) {
    Serial.println("No se pudo inicializar la LCD tras varios intentos.");
    Serial.println("Revisa conexiones, soldaduras y VCC/VO.");
  }
}

void loop() {
  // Muestra estado y reintenta si se pierde
  scanI2C();
  // Intenta escribir actualizar segunda línea cada 2s para verificar estabilidad
  static int counter = 0;
  if (Wire.beginTransmission(LCD_ADDR) == 0) {
    lcd.setCursor(0,1);
    lcd.print("Cnt: ");
    lcd.print(counter++);
    lcd.print("    ");
  } else {
    Serial.println("LCD no responde durante loop; reintentando init...");
    if (tryInitLCD()) {
      Serial.println("Reinit LCD OK");
    } else {
      Serial.println("Reinit LCD fallo");
    }
  }
  delay(2000);
}