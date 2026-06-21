#include <Keypad.h>

const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'A','3','2','1'},
  {'B','6','5','4'},
  {'C','9','8','7'},
  {'D','#','0','*'}
};

byte rowPins[ROWS] = {23, 22, 21, 19};
byte colPins[COLS] = {15, 4, 5, 18};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void setup() {
  Serial.begin(9600);

  // RX = 16, TX = 17
  Serial2.begin(9600, SERIAL_8N1, 16, 17);

  Serial.println("Teclado pronto");
}

void loop() {
  char key = keypad.getKey();

  if (key) {
    Serial.print("Enviado: ");
    Serial.println(key);

    Serial2.print(key);
  }
}