/* made by BinaryWorlds
Not for commercial use,
in other case by free to use it. Just copy this text and link to oryginal repository: https://github.com/BinaryWorlds/ThermalPrinter
I am not responsible for errors in the library. I deliver it "as it is".
I will be grateful for all suggestions.
*/
#include <Arduino.h>
#include "TPrinter.h"
#include <HardwareSerial.h>

HardwareSerial mySerial(1);
/*  u need to use max3485 or similar if u have printer with rs232
  if u use esp32 or other MCU with 3v3 logic level u need to use logic level converter
pin 17 Tx
pin 16 Rx
*/
Tprinter myPrinter(&mySerial, 9600);

void setup() {
  micros();
  Serial.begin(9600); // monitor
  mySerial.begin(9600, SERIAL_8N1, 16, 17);
  myPrinter.begin();
  myPrinter.enableDtr(4);

  myPrinter.identifyChars("ą ę"); // UTF-8
  myPrinter.printCharset();
  myPrinter.printCodepage();
  myPrinter.justify('C');
  myPrinter.println("center");
  myPrinter.justify('R');
  myPrinter.println("right");
  myPrinter.justify('L');
  myPrinter.println("left");

  uint8_t list[]={5,10,15,20,25};
  myPrinter.setTabs(list, 5);
  myPrinter.print("1");
  myPrinter.tab();
  myPrinter.print("2");
  myPrinter.tab();
  myPrinter.print("3");
  myPrinter.tab();
  myPrinter.print("4");
  myPrinter.println();

  myPrinter.setMode(FONT_B, DOUBLE_WIDTH, DOUBLE_HEIGHT);
  myPrinter.println("FONT_B, bigger");
  myPrinter.unsetMode(FONT_B);
  myPrinter.println("FONT_A, bigger");
}

void loop() {
  myPrinter.printFromSerial(); //open monitor and print something
}
