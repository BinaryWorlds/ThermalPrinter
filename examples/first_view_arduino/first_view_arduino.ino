/* 
  made by BinaryWorlds
  Not for commercial use,
  in other case by free to use it. Just copy this text and link to oryginal repository: https://github.com/BinaryWorlds/ThermalPrinter

  I am not responsible for errors in the library. I deliver it "as it is".
  I will be grateful for all suggestions.
*/

#include <Arduino.h>
#include "TPrinter.h"
#include <SoftwareSerial.h> // if error u can also use HardwareSerial.h, after adding library

/* 
  You need to use max3485(for 3V3 logic lvl) or similar, if u have printer with rs232.
  If u use esp32 or other MCU with 3v3 logic level u can try with logic level converter.

  Check which pin pairs are responsible for communication on your board!
*/
const byte rxPin = 10; // Not all pins of arduino support interrupts, check it!
const byte txPin = 11;

const byte dtrPin = 4;
const int printerBaudrate = 9600; // or 19200 usually

SoftwareSerial mySerial(rxPin, txPin);
Tprinter myPrinter(&mySerial, printerBaudrate);
//u can assign here other stream like Serial1, Serial2, Serial3 etc

void setup()
{
  micros();
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  Serial.begin(9600);               // monitor
  mySerial.begin(printerBaudrate);  //must be 8N1 mode
  myPrinter.begin();                // u can edit what be happen in Tprinter.cpp, like delay time(2s)
  myPrinter.enableDtr(dtrPin, LOW); // pinNR, busyState;
  /* my printer is busy when DTR is LOW,
    there is a high probability that you should change it to HIGH;
    if wrong - print will no start  */

  // if u dont enable checking dtr, u should call this:
  //myPrinter.autoCalculate();

  //or u can set it manually using this:
  //myPrinter.setTimes(40000, 3000); //oneDotHeight_printTime, oneDotHeight_feedTime in micros
  //last is worst option

  myPrinter.identifyChars("ą ę"); // UTF-8

  myPrinter.println("charset:");
  myPrinter.printCharset();

  myPrinter.println("codepage:");
  myPrinter.printCodepage();

  myPrinter.justify('C');
  myPrinter.println("center");
  myPrinter.justify('R');
  myPrinter.println("right");
  myPrinter.justify('L');
  myPrinter.println("left");

  uint8_t list[] = {5, 10, 15, 20, 25};
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

  myPrinter.feed(6);
}

void loop()
{
  myPrinter.printFromSerial(); //open monitor and print something
}