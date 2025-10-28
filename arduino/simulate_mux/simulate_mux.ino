//--------------------------------------------------------------------------------------
//this code was created in order to simulate wideband multiplexer present in laboratory
//--------------------------------------------------------------------------------------

#include <AltSoftSerial.h>

#define I2C_LCD //choose if you have I2C
#ifdef I2C_LCD
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ===== LCD I2C =====
// Format: LiquidCrystal_I2C(address, columns, rows)
LiquidCrystal_I2C lcd(0x27, 16, 2);

#else

#include <LiquidCrystal.h>
// ===== LCD Pins =====
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

#endif

// ===== Software Serial for ESP32 =====
AltSoftSerial espSerial; // RX=8, TX=9

// ===== Output Pins =====
const byte outputPins[12] = {A0, A1, A2, A3, A4, A5, A6, A7, 10, 11, 12, 13};

// ===== State Variables =====
int in1 = 0;
int in2 = 0;

void setup() {
  Serial.begin(9600);
  espSerial.begin(9600);

#ifdef I2C_LCD
  // Initialize I2C LCD
  Wire.begin();
  lcd.init();
  lcd.backlight();

#else
  lcd.begin(16, 2);

#endif
  lcd.clear();
  lcd.print("IN1: 0");
  lcd.setCursor(0, 1);
  lcd.print("IN2: 0");
  
  // Initialize output pins
  for (int i = 0; i < 12; i++) {
    pinMode(outputPins[i], OUTPUT);
    digitalWrite(outputPins[i], LOW);
  }
  
  Serial.println("Arduino ready. Waiting for ESP32...");
}

void loop() {
  // Check for commands from ESP32
  if (espSerial.available()) {
    String cmd = espSerial.readStringUntil('\n');
    cmd.trim();
    Serial.println("Received: " + cmd);
    
    if (cmd.startsWith("SET IN1")) {
      in1 = cmd.substring(7).toInt();
      updateLCD();
      updateOutputs();
      espSerial.print("OK: IN1 set to ");
      espSerial.print(in1);

    } else if (cmd.startsWith("SET IN2")) {
      in2 = cmd.substring(7).toInt();
      updateLCD();
      updateOutputs();
      espSerial.print("OK: IN2 set to ");
      espSerial.print(in2);

    } else if (cmd == "STATUS") {
      espSerial.print("IN1=" + String(in1) + " IN2=" + String(in2));
    } else if (cmd == "PING") {
      espSerial.print("PONG");
    } else {
      espSerial.print("UNKNOWN: " + cmd);
    }
  }
}

// ===== Update LCD =====
void updateLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("IN1: ");
  lcd.print(in1);
  lcd.setCursor(0, 1);
  lcd.print("IN2: ");
  lcd.print(in2);
}

// ===== Update Outputs =====
void updateOutputs() {
  // Turn all off first
  for (int i = 0; i < 12; i++) {
    digitalWrite(outputPins[i], LOW);
  }
  
  // Activate selected ones (1–12)
  if (in1 >= 1 && in1 <= 12)
    digitalWrite(outputPins[in1 - 1], HIGH);
  if (in2 >= 1 && in2 <= 12)
    digitalWrite(outputPins[in2 - 1], HIGH);
}