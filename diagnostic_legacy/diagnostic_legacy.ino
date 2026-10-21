// ------------------- Pin Setup -------------------
// 74HC595 shift register connections (for 8 rows)
const int dataPin  = 12;   // DS (pin 14 on 74HC595)
const int latchPin = 10;   // ST_CP (pin 12 on 74HC595)
const int clockPin = 11;   // SH_CP (pin 11 on 74HC595)

// Number of rows and columns
const int numRows = 8;   // bottom membrane → via 74HC595
const int numCols = 18;  // top membrane → direct to Arduino

// List of Arduino pins used for the 18 columns
// Replace these with the exact pins you connected
// Fill in YOUR actual wiring order here
const int colPins[numCols] = {
  0,1,2,3,4,5,6,7,8,9,13,A0,A1,A2,A3,A4,A5,
};

// ------------------- Setup -------------------
void setup() {
  // Shift register pins
  pinMode(dataPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);

  // Set all column pins as INPUT_PULLUP
  // (HIGH by default, will go LOW when connected to a row)
  for (int i = 0; i < numCols; i++) {
    pinMode(colPins[i], INPUT_PULLUP);
  }

  // Start Serial Monitor
  Serial.begin(9600);
  Serial.println("=== Keyboard Diagnostic Mode ===");
  Serial.println("Press a key and watch row/col values...");
}

// ------------------- Functions -------------------
// Activate one row at a time (only that row = LOW)
void activateRow(int row) {
  // If your keyboard needs rows LOW: use this
  byte rowData = ~(1 << row);

  // If your keyboard needs rows HIGH: use instead
  // byte rowData = (1 << row);

  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, rowData);
  digitalWrite(latchPin, HIGH);
}

// ------------------- Loop -------------------
void loop() {
  // Scan each row
  for (int row = 0; row < numRows; row++) {
    activateRow(row); // enable current row

    // Scan each column
    for (int col = 0; col < numCols; col++) {
      if (digitalRead(colPins[col]) == LOW) {
        // Key press detected!
        Serial.print("Key pressed at: Row ");
        Serial.print(row);
        Serial.print("  Col ");
        Serial.println(col);

        delay(150); // debounce / slow down
      }
    }
  }
}
