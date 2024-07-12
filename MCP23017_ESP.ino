#include <Wire.h>
#include <Adafruit_MCP23X17.h>

// Create instances for the two MCP23017 devices
Adafruit_MCP23X17 mcp1;
Adafruit_MCP23X17 mcp2;

bool cycling = false;
unsigned long lastCycleTime = 0;
int cycleRelayIndex = 0;
bool cycleForward = true;

void setup() {
  Serial.begin(115200);  // Use a higher baud rate for more reliable communication with ESP32
  delay(1000);  // Delay to ensure Serial Monitor is ready
  Serial.println("MCP23017 Serial Control Test!");

  // Initialize I2C with specific SDA and SCL pins for ESP32
  Wire.begin(21, 22); // SDA, SCL
  delay(1000); // Give some time for the I2C bus to stabilize

  // Initialize the first MCP23017 using I2C with address 0x20
  Serial.println("Initializing MCP23017 at address 0x20...");
  if (!mcp1.begin_I2C(0x20, &Wire)) {
    Serial.println("Error initializing MCP23017 at address 0x20.");
    while (1);
  } else {
    Serial.println("Successfully initialized MCP23017 at address 0x20.");
  }

  // Initialize the second MCP23017 using I2C with address 0x21
  Serial.println("Initializing MCP23017 at address 0x21...");
  if (!mcp2.begin_I2C(0x21, &Wire)) {
    Serial.println("Error initializing MCP23017 at address 0x21.");
    while (1);
  } else {
    Serial.println("Successfully initialized MCP23017 at address 0x21.");
  }

  // Configure all pins of both MCP23017s as outputs and turn off all relays initially
  for (uint8_t pin = 0; pin < 16; pin++) {
    mcp1.pinMode(pin, OUTPUT);
    mcp1.digitalWrite(pin, HIGH);  // Ensure relays are off initially (HIGH for low-level trigger relays)
    
    mcp2.pinMode(pin, OUTPUT);
    mcp2.digitalWrite(pin, HIGH);  // Ensure relays are off initially (HIGH for low-level trigger relays)
  }

  Serial.println("Enter commands in the format Nx or Fx where x is the relay number (0-31).");
  Serial.println("Use commands ALLON, ALLOFF, CYCLEF, CYCLER, CYCLE, or STOP for additional operations.");
}

void loop() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command.equals("ALLON")) {
      turnAllRelays(true);
      Serial.println("All relays turned ON one by one.");
    } else if (command.equals("ALLOFF")) {
      turnAllRelays(false);
      Serial.println("All relays turned OFF one by one.");
    } else if (command.equals("CYCLEF")) {
      cycleForwardOnce();
      Serial.println("Cycled forward through all relays once.");
    } else if (command.equals("CYCLER")) {
      cycleBackwardOnce();
      Serial.println("Cycled backward through all relays once.");
    } else if (command.equals("CYCLE")) {
      cycling = true;
      cycleRelayIndex = 0;
      cycleForward = true;
      Serial.println("Started continuous cycling.");
    } else if (command.equals("STOP")) {
      cycling = false;
      turnAllRelays(false);
      Serial.println("Stopped continuous cycling.");
    } else if (command.length() > 1) {
      char action = command.charAt(0);
      int relay = command.substring(1).toInt();

      if (relay >= 0 && relay < 32) {
        Adafruit_MCP23X17 &mcp = (relay < 16) ? mcp1 : mcp2;
        uint8_t pin = (relay < 16) ? relay : relay - 16;

        if (action == 'N') {
          mcp.digitalWrite(pin, LOW); // Turn on relay (LOW for low-level trigger)
          Serial.print("Relay ");
          Serial.print(relay);
          Serial.println(" turned ON.");
        } else if (action == 'F') {
          mcp.digitalWrite(pin, HIGH); // Turn off relay (HIGH for low-level trigger)
          Serial.print("Relay ");
          Serial.print(relay);
          Serial.println(" turned OFF.");
        } else {
          Serial.println("Invalid command. Use Nx or Fx where x is the relay number (0-31).");
        }
      } else {
        Serial.println("Invalid relay number. Use 0-31.");
      }
    } else {
      Serial.println("Invalid command format. Use Nx or Fx where x is the relay number (0-31).");
    }
  }

  if (cycling && (millis() - lastCycleTime) > 100) { // Adjust the delay as needed
    // Turn off all relays first
    turnAllRelays(false);
    
    // Turn on the current relay
    cycleRelay(cycleRelayIndex, true);

    if (cycleForward) {
      cycleRelayIndex++;
      if (cycleRelayIndex >= 32) {
        cycleRelayIndex = 31;
        cycleForward = false;
      }
    } else {
      cycleRelayIndex--;
      if (cycleRelayIndex < 0) {
        cycleRelayIndex = 0;
        cycleForward = true;
      }
    }

    lastCycleTime = millis();
  }
}

void turnAllRelays(bool state) {
  for (uint8_t pin = 0; pin < 16; pin++) {
    mcp1.digitalWrite(pin, state ? LOW : HIGH);
    mcp2.digitalWrite(pin, state ? LOW : HIGH);
  }
}

void cycleForwardOnce() {
  for (uint8_t relay = 0; relay < 32; relay++) {
    cycleRelay(relay, false);
  }
}

void cycleBackwardOnce() {
  for (int relay = 31; relay >= 0; relay--) {
    cycleRelay(relay, false);
  }
}

void cycleRelay(int relay, bool continuous) {
  Adafruit_MCP23X17 &mcp = (relay < 16) ? mcp1 : mcp2;
  uint8_t pin = (relay < 16) ? relay : relay - 16;

  // Turn on current relay
  mcp.digitalWrite(pin, LOW);
  delay(100); // Delay to keep relay on for a short time

  // Turn off current relay unless continuous mode
  if (!continuous) {
    mcp.digitalWrite(pin, HIGH);
  }
}

void cycleRelays() {
  for (uint8_t relay = 0; relay < 32; relay++) {
    cycleRelay(relay, false);
  }
  for (int relay = 31; relay >= 0; relay--) {
    cycleRelay(relay, false);
  }
  turnAllRelays(false);
}
