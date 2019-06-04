#include <Arduino.h>
#include "TPrinter.h"
#include <HardwareSerial.h>

HardwareSerial mySerial(1);
/*  u need to use max3485 or similar if u have printer with rs232
  if u use esp32 or other MCU with 3v3 logic level u need to use logic level converter
pin 17 Tx
pin 16 Rx
*/
Tprinter druk(&mySerial, 9600);

void setup() {
  micros();
  Serial.begin(9600); // monitor
  mySerial.begin(9600, SERIAL_8N1, 16, 17);

  druk.begin();

  druk.identifyChars("ą ę"); // UTF-8
  druk.printCharset();
  druk.printCodepage();
  druk.justify('C');
  druk.println("center");
  druk.justify('R');
  druk.println("right");
  druk.justify('L');
  druk.println("left");

  uint8_t list[]={5,10,15,20,25};
  druk.setTabs(list, 5);
  druk.print("1");
  druk.tab();
  druk.print("2");
  druk.tab();
  druk.print("3");
  druk.tab();
  druk.print("4");
  druk.println();

  druk.setMode(FONT_B, DOUBLE_WIDTH, DOUBLE_HEIGHT);
  druk.println("FONT_B, bigger");
  druk.unsetMode(FONT_B);
  druk.println("FONT_A, bigger");

}

void loop() {
  druk.printFromSerial(); //open monitor and print something
}
