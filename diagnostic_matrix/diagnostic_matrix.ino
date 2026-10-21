// Matrix diagnostic matching the FINAL wiring used by samsung_keyboard_code.
// Columns (18) are driven LOW one at a time: 16 via two 74HC595 + 2 direct pins.
// Rows (8) are read with INPUT_PULLUP. Output: "Row r  Col c"
// (same numbering as hardware/Keyboard_Mapping.csv). Serial Monitor: 115200 baud.
// Pins 0/1 are columns; the Pro Micro's native USB Serial doesn't use them.

const int dataPin1 = 14, latchPin1 = 15;   // 74HC595 #1 (cols 0-7)
const int dataPin2 = 10, latchPin2 = 16;   // 74HC595 #2 (cols 8-15)
const int clockPin = A0;                   // shared SH_CP
const int extraColPins[2] = {0, 1};        // cols 16, 17
const int rowPins[8] = {2, 3, 4, 5, 6, 7, 8, 9};
const int NUM_COLS = 18;

void setColumn(int col) {
  uint8_t r1 = 0xFF, r2 = 0xFF;
  digitalWrite(extraColPins[0], HIGH);
  digitalWrite(extraColPins[1], HIGH);
  if (col >= 0 && col < 8) r1 &= ~(1 << col);
  else if (col >= 8 && col < 16) r2 &= ~(1 << (col - 8));
  else if (col == 16) digitalWrite(extraColPins[0], LOW);
  else if (col == 17) digitalWrite(extraColPins[1], LOW);
  digitalWrite(latchPin1, LOW); shiftOut(dataPin1, clockPin, LSBFIRST, r1); digitalWrite(latchPin1, HIGH);
  digitalWrite(latchPin2, LOW); shiftOut(dataPin2, clockPin, LSBFIRST, r2); digitalWrite(latchPin2, HIGH);
}

void setup() {
  pinMode(dataPin1, OUTPUT); pinMode(latchPin1, OUTPUT);
  pinMode(dataPin2, OUTPUT); pinMode(latchPin2, OUTPUT);
  pinMode(clockPin, OUTPUT);
  for (int i = 0; i < 8; i++) pinMode(rowPins[i], INPUT_PULLUP);
  for (int i = 0; i < 2; i++) { pinMode(extraColPins[i], OUTPUT); digitalWrite(extraColPins[i], HIGH); }
  setColumn(-1);
  Serial.begin(115200);
  while (!Serial && millis() < 3000) {}
  Serial.println(F("=== Matrix diagnostic: press a key ==="));
}

void loop() {
  for (int c = 0; c < NUM_COLS; c++) {
    setColumn(c);
    delayMicroseconds(50);
    for (int r = 0; r < 8; r++) {
      if (digitalRead(rowPins[r]) == LOW) {
        Serial.print(F("Row ")); Serial.print(r);
        Serial.print(F("  Col ")); Serial.println(c);
        delay(150);
      }
    }
  }
  setColumn(-1);
}
