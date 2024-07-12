#include <Wire.h>

void setup() {
  Serial.begin(115200);
  while (!Serial); // Wait for the serial connection
  Serial.println("I2C Scanner");

  Wire.begin(21, 22); // ESP32 default I2C pins
  delay(1000); // Allow time for initialization

  Serial.println("Scanning...");
  byte count = 0;

  for (byte i = 8; i < 120; i++) {
    Wire.beginTransmission(i);
    if (Wire.endTransmission() == 0) {
      Serial.print("Found I2C Device: ");
      Serial.print(" (0x");
      Serial.print(i, HEX);
      Serial.println(")");
      count++;
    }
  }

  Serial.print("Found ");
  Serial.print(count, DEC);
  Serial.println(" device(s).");
}

void loop() {
  // Do nothing
}
