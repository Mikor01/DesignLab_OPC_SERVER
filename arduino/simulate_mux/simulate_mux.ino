// Compatible with ESP32 OPC-UA bridge

#include <AltSoftSerial.h>

#define I2C_LCD
#ifdef I2C_LCD
#include <Wire.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

#else
#include <LiquidCrystal.h>
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);
#endif

AltSoftSerial espSerial; // RX=8, TX=9


const byte outputPins[16] = {A0, A1, A2, A3, A4, A5, A6, A7, 10, 11, 12, 13, 22, 23, 24, 25};

int channel_out0 = -1;
int channel_out1 = -1;
int current_input_channel = 0;

char cmdBuffer[64];
int cmdIndex = 0;

void setup() {
  Serial.begin(9600);
  espSerial.begin(9600);

#ifdef I2C_LCD
  Wire.begin();
  lcd.init();
  lcd.backlight();
#else
  lcd.begin(16, 2);
#endif

  lcd.clear();
  lcd.print("IN0: OFF");
  lcd.setCursor(0, 1);
  lcd.print("IN1: OFF");

  // Ustaw wszystkie wyjścia
  for (int i = 0; i < 16; i++) {
    pinMode(outputPins[i], OUTPUT);
    digitalWrite(outputPins[i], LOW);
  }

  Serial.println("Arduino Multiplexer Simulator Ready");
  Serial.println("Commands: in <1-2> out <0-15>, in <1-2> off, release, clear, draw, screen");
}

void loop() {
  if (espSerial.available()) {
    char c = espSerial.read();

    if (c == '\n' || c == '\r') {
      if (cmdIndex > 0) {
        cmdBuffer[cmdIndex] = '\0';
        processCommand(cmdBuffer, true);
        cmdIndex = 0;
      }
    } else if (cmdIndex < 63) {
      cmdBuffer[cmdIndex++] = c;
    }
  }

  if (Serial.available()) {
    char c = Serial.read();

    if (c == '\n' || c == '\r') {
      if (cmdIndex > 0) {
        cmdBuffer[cmdIndex] = '\0';
        processCommand(cmdBuffer, false);
        cmdIndex = 0;
      }
    } else if (cmdIndex < 63) {
      cmdBuffer[cmdIndex++] = c;
    }
  }
}

void processCommand(char* input, bool fromESP) {
  if (fromESP) {
    Serial.print("ESP32 cmd: ");
  } else {
    Serial.print("Serial cmd: ");
  }
  Serial.println(input);

  char* token = strtok(input, " ");
  bool updateNeeded = false;
  bool commandOK = true;

  while (token != NULL && commandOK) {

    if (strcmp(token, "in") == 0) {
      token = strtok(NULL, " ");
      if (token == NULL) {
        sendResponse("ERROR: Missing input channel number", fromESP);
        commandOK = false;
        break;
      }

      int value = atoi(token);


      if (value < 0 || value > 1) {
        sendResponse("ERROR: Invalid input channel (use 0 or 1)", fromESP);
        commandOK = false;
        break;
      }

      current_input_channel = value;  // 1 → 0, 2 → 1
      updateNeeded = true;
    }

    else if (strcmp(token, "out") == 0) {
      token = strtok(NULL, " ");
      if (token == NULL) {
        sendResponse("ERROR: Missing output number", fromESP);
        commandOK = false;
        break;
      }

      int value = atoi(token);

      if (value < 0 || value > 15) {
        sendResponse("ERROR: Invalid output (use 0-15)", fromESP);
        commandOK = false;
        break;
      }

      if (current_input_channel == 0) {
        channel_out0 = value;
      } else if (current_input_channel == 1) {
        channel_out1 = value;
      }

      updateNeeded = true;
    }

    else if (strcmp(token, "off") == 0) {
      if (current_input_channel == 0) {
        channel_out0 = -1;
      } else if (current_input_channel == 1) {
        channel_out1 = -1;
      }
      updateNeeded = true;
    }

    else if (strcmp(token, "release") == 0) {
      sendResponse("OK: Released", fromESP);
      updateNeeded = true;
    }

    else if (strcmp(token, "clear") == 0) {
      lcd.clear();
      sendResponse("OK: Display cleared", fromESP);
    }

    else if (strcmp(token, "draw") == 0) {
      updateLCD();
      sendResponse("OK: Display redrawn", fromESP);
    }

    else if (strcmp(token, "screen") == 0) {
      lcd.clear();
      lcd.print("Screensaver ON");
      sendResponse("OK: Screensaver enabled", fromESP);
    }

    else if (strcmp(token, "STATUS") == 0) {
      sendStatus(fromESP);
      commandOK = true;
      token = NULL;
      continue;
    }

    else {
      String errorMsg = "ERROR: Unknown command '";
      errorMsg += token;
      errorMsg += "'";
      sendResponse(errorMsg.c_str(), fromESP);
      commandOK = false;
      break;
    }

    token = strtok(NULL, " ");
  }

  if (commandOK) {
    if (updateNeeded) {
      updateOutputs();
      updateLCD();
    }
    sendResponse("OK", fromESP);
    sendStatus(fromESP);
  }
}

void sendResponse(const char* msg, bool toESP) {
  if (toESP) {
    espSerial.println(msg);
  }
  Serial.println(msg);
}

void sendStatus(bool toESP) {
  String statusMsg = "IN0=";
  statusMsg += (channel_out0 >= 0) ? String(channel_out0) : "OFF";
  statusMsg += " IN1=";
  statusMsg += (channel_out1 >= 0) ? String(channel_out1) : "OFF";

  if (toESP) {
    espSerial.println(statusMsg);
  }
  Serial.println(statusMsg);
}

void updateLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("IN0: ");
  if (channel_out0 >= 0) {
    lcd.print(channel_out0);
  } else {
    lcd.print("OFF");
  }

  lcd.setCursor(0, 1);
  lcd.print("IN1: ");
  if (channel_out1 >= 0) {
    lcd.print(channel_out1);
  } else {
    lcd.print("OFF");
  }
}

void updateOutputs() {
  for (int i = 0; i < 16; i++) {
    if (i != channel_out0 && i != channel_out1) {
      digitalWrite(outputPins[i], LOW);
    }
  }

  if (channel_out0 >= 0 && channel_out0 <= 15) {
    digitalWrite(outputPins[channel_out0], HIGH);
  }
  if (channel_out1 >= 0 && channel_out1 <= 15) {
    digitalWrite(outputPins[channel_out1], HIGH);
  }
}
