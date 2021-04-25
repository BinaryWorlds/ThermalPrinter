// made by BinaryWorlds
// Not for commercial use, in other case by free to use it.
// Just copy this text and link to oryginal repository: https://github.com/BinaryWorlds/ThermalPrinter

// I am not responsible for errors in the library. I deliver it "as it is".
// I will be grateful for all suggestions.

// Tested on firmware 2.69 and JP-QR701
// Some features may not work on the older firmware.

#include "TPrinter.h"

Tprinter::Tprinter(Stream *s, int baud) : stream(s), baudrate(baud) {
  char_send_time = (11 * 1000000 / baud);  // for 8N1
}

// private
void Tprinter::update() {
  charHeight = (printMode & FONT_B) ? 17 : 24;  // B : A
  charWidth = (printMode & FONT_B) ? 9 : 12;
  widthMax = (printMode & FONT_B) ? 42 : 32;
  if (printMode & DOUBLE_WIDTH) {
    charWidth *= 2;
    widthMax /= 2;
  }
  if (printMode & DOUBLE_HEIGHT) charHeight *= 2;
  if (calculateMode) {
    calculatePrintTime();
  } else {
    print_time = charHeight * oneDotHeight_printTime;
    feed_time = (charHeight + interlineHeight) * oneDotHeight_feedTime;
  }
}

size_t Tprinter::write(uint8_t sign) {
  if (sign != A_CR) {
    wait();
    unsigned long val = char_send_time;
    stream->write(sign);
    cursor += charWidth + charSpacing;
    if (printMode & DOUBLE_WIDTH) cursor += charSpacing;
    if ((widthInDots - cursor) < charWidth && sign != '\n') stream->write(sign = '\n');
    // force the printout of a new line;
    // printer print a line after took widthMax + 1 char

    if (sign == '\n') {
      if (calculateMode) {
        val += (cursor * print_time) / widthInDots + feed_time;
      } else {
        val += print_time + feed_time;  // max time
      }
      cursor = 0;
    }
    setDelay(val);
    // printPosition();
  }
  return 1;
}

void Tprinter::feed(uint8_t n) {
  wait();
  stream->write(A_ESC);
  stream->write('d');
  stream->write(n);
  setDelay(3 * char_send_time + feed_time * n);
  cursor = 0;
}

void Tprinter::enableDtr(uint8_t dtr, bool busy) {
  if (dtrEnabled && dtrPin != dtr) disableDtr();  // useless but possible
  dtrPin = dtr;
  busyState = busy;
  pinMode(dtrPin, INPUT_PULLUP);
  dtrEnabled = true;
  wait();
  stream->write(A_GS);
  stream->write('a');
  stream->write(uint8_t(1 << 5));
  setDelay(3 * char_send_time);
}

void Tprinter::disableDtr(bool mode) {
  // if pin unused - pull them for decrease(vs INPUT mode)
  // EMI, noisy and power concumption
#ifdef INPUT_PULLDOWN
  if (mode) {
    pinMode(dtrPin, INPUT_PULLUP);
  } else {
    pinMode(dtrPin, INPUT_PULLDOWN);
  }
#else
  pinMode(dtrPin, INPUT_PULLUP);
#endif
  dtrEnabled = false;
}

void Tprinter::wait() {
  if (dtrEnabled) {
    while (digitalRead(dtrPin) == busyState) {
    };  // 0 - my printer busy, check your printer
  } else {
    while (long(micros() - endPoint) < 0) {
    };
  }
}

void Tprinter::setDelay(unsigned long time) {
  if (!dtrEnabled) endPoint = (micros() + time);
}

void Tprinter::setCodePage(uint8_t page) {
  // Set the same encoding in text editor as in the printer!
  if (page > 47) page = 47;

  wait();
  stream->write(A_FS);
  stream->write('.');  // kanji mode off

  stream->write(A_ESC);
  stream->write('t');
  stream->write(page);
  setDelay(5 * char_send_time);
}

void Tprinter::setCharset(uint8_t val) {
  if (val > 15) val = 15;
  wait();

  stream->write(A_ESC);
  stream->write('R');
  stream->write(val);
  setDelay(3 * char_send_time);
}

void Tprinter::autoCalculate(bool val) {
  calculateMode = val;
  update();
}

void Tprinter::calculatePrintTime() {
  print_time = ((widthInDots * charHeight) / ((heating_dots + 1) * 8)) * 10 * (heating_time + heating_interval) / 2;
  // rarely all dots are printed
}

void Tprinter::setTimes(unsigned long p, unsigned long f) {
  oneDotHeight_printTime = p;
  oneDotHeight_feedTime = f;

  if (!calculateMode) {
    print_time = p * charHeight;
    feed_time = f * (interlineHeight + charHeight);
  }
}

void Tprinter::setHeat(uint8_t n1, uint8_t n2, uint8_t n3) {
  wait();
  stream->write(A_ESC);
  stream->write('7');
  stream->write(heating_dots = n1);
  stream->write(heating_time = n2);
  stream->write(heating_interval = n3);
  setDelay(5 * char_send_time);
  if (calculateMode) calculatePrintTime();
}

void Tprinter::setMode(uint8_t m1, uint8_t m2, uint8_t m3, uint8_t m4, uint8_t m5, uint8_t m6, uint8_t m7) {
  wait();
  printMode |= (m1 + m2 + m3 + m4 + m5 + m6 + m7);
  stream->write(A_ESC);
  stream->write('!');
  stream->write(printMode);
  setDelay(3 * char_send_time);
  update();
}

void Tprinter::unsetMode(uint8_t m1, uint8_t m2, uint8_t m3, uint8_t m4, uint8_t m5, uint8_t m6, uint8_t m7) {
  wait();
  printMode &= ~(m1 + m2 + m3 + m4 + m5 + m6 + m7);
  stream->write(A_ESC);
  stream->write('!');
  stream->write(printMode);
  setDelay(3 * char_send_time);

  update();
}

void Tprinter::invert(bool n) {
  wait();
  stream->write(A_ESC);
  stream->write('{');
  stream->write(n);
  setDelay(3 * char_send_time);
}

void Tprinter::justify(char val) {
  uint8_t set{};
  wait();

  switch (val) {
    case 'L':
      set = 0;
      break;
    case 'C':
      set = 1;
      break;
    case 'R':
      set = 2;
      break;
  }
  stream->write(A_ESC);
  stream->write('a');
  stream->write(set);
  setDelay(3 * char_send_time);
}

void Tprinter::underline(uint8_t n) {
  if (n > 2) n = 2;
  wait();
  stream->write(A_ESC);
  stream->write('-');
  stream->write(n);
  setDelay(3 * char_send_time);
}

void Tprinter::setInterline(uint8_t n) {
  // ESC '2' - back to default
  wait();
  stream->write(A_ESC);
  stream->write('3');
  if (n + charHeight >= 255) {
    interlineHeight = 255;
  } else {
    interlineHeight = n + charHeight;
  }
  stream->write(interlineHeight);
  interlineHeight -= charHeight;

  update();
  setDelay(3 * char_send_time);
}

void Tprinter::setCharSpacing(uint8_t n) {
  // printer default: 0
  wait();
  stream->write(A_ESC);
  stream->write(A_SPACE);
  stream->write(charSpacing = n);
  setDelay(3 * char_send_time);
}

void Tprinter::setTabs(uint8_t *tab, uint8_t size) {
  // Enter values ​​in ascending order.
  // The next value can't be equal to or less than the previous one
  // - the printer will consider it as a data
  // (will finish setting the tabs)
  // sets as absolute position;
  tabs[tabsAmount = 0] = 0;
  wait();
  stream->write(A_ESC);
  stream->write('D');

  for (uint8_t i = 0; i < size; i++) {
    if (tab[i] < widthMax && tab[i] > (tabs[tabsAmount] / charWidth) && tabsAmount < 32) {
      stream->write(tab[i]);
      tabs[tabsAmount++] = tab[i] * charWidth;
    }
  }
  stream->write((uint8_t)0);  // from datasheet - end of list
  cursor = 0;
  setDelay((tabsAmount + 3) * char_send_time);
}

void Tprinter::tab() {
  // If you don't set the next horizontal tab position
  // (bigger than actual cursor position),
  // the command is ignored.
  for (uint8_t i = 0; i < tabsAmount; i++) {
    if (tabs[i] > cursor) {
      cursor = tabs[i];
      break;
    }
  }
  stream->write(A_HT);

  if ((widthInDots - cursor) < charWidth) {
    setDelay(char_send_time + print_time + feed_time);
    cursor = 0;  // printer go newline
  } else {
    setDelay(char_send_time);
  }
}

void Tprinter::clearTabs() {
  stream->write(A_ESC);
  stream->write('D');
  stream->write('0');
  setDelay(3 * char_send_time);

  tabs[tabsAmount = 0] = 0;
}

void Tprinter::reset() {
  stream->write(A_ESC);
  stream->write('@');
  setDelay(2 * char_send_time);

  cursor = 0;
  tabs[tabsAmount = 0] = 0;
  interlineHeight = 6;
  printMode = 0;
  charSpacing = 0;
  heating_dots = 9;
  heating_time = 80;
  heating_interval = 2;
  update();
}

void Tprinter::begin() {
  setDelay(2000000);  // 2s
  wait();
  reset();
  online();
  setHeat();
  setCodePage();
  setCharset();
  setInterline(0);  // save paper during testing
  uint8_t list[] = {4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48};
  setTabs(list, 12);  // if widthMax == 32, last tab is 28
}

void Tprinter::offline() {
  wait();
  stream->write(A_ESC);
  stream->write('=');
  stream->write((uint8_t)0);
  setDelay(3 * char_send_time);
}

void Tprinter::online() {
  wait();
  stream->write(A_ESC);
  stream->write('=');
  stream->write((uint8_t)1);
  setDelay(3 * char_send_time);
}

// for test
void Tprinter::identifyChars(char *tab) {
  // dont use it in the same time with printFromSerial()
  int i{};
  Serial.println();
  Serial.println(F("Separate letters with a space, e.g \"ą ć d\"!"));
  do {
    if (tab[i] != A_SPACE) {
      int val = i;
      while (tab[i] != A_SPACE && tab[i] != 0) {
        Serial.print(tab[i]);
        i++;
      }
      Serial.print(F(" HEX: "));
      while (val != i) {
        Serial.print(tab[val], HEX);
        val++;
      }
      Serial.println();
    } else {
      i++;
    }
  } while (tab[i] != 0);  // end of string
}

void Tprinter::printFromSerial() {
  // you can use it e.g for test;
  // require Serial.begin(baudrate) in void setup()
  wait();
  while (Serial.available()) {
    char sign{};
    sign = (char)Serial.read();
    print(sign);
  }
}

void Tprinter::printCharset() {
  wait();
  println();
  print(F("   01234567 89ABCDEF"));
  for (uint8_t i = 32; i < 128; i++) {
    if ((i % 16) == 0) {
      println();
      print(i / 16, DEC);
      print(F("- "));
    }
    write(i);
    if ((i % 16) == 7) print(F(" "));
  }
  println();
}

void Tprinter::printCodepage() {
  wait();
  println();
  print(F("   01234567 89ABCDEF"));
  for (uint8_t i = 128; i < 255; i++) {
    if ((i % 16) == 0) {
      println();
      char val = 'A';
      val += i / 16 - 10;
      if (i / 16 < 10) {
        print(i / 16, DEC);
      } else {
        print(val);
      }
      print(F("- "));
    }
    write(i);
    if ((i % 16) == 7) print(F(" "));
  }
  println();
}

uint16_t Tprinter::printPosition() {
  Serial.print("Actual position: ");
  Serial.println(cursor, DEC);
  return cursor;
}

// private
void Tprinter::initBitmapData(uint8_t rowsInPackage, uint8_t bytesPerRow) {
  wait();
  stream->write(A_DC2);
  stream->write(A_STAR);
  stream->write(rowsInPackage);
  stream->write(bytesPerRow);
  setDelay(4 * char_send_time);
}

// private
void Tprinter::sendBitmapByte(uint8_t byteToSend) {
  wait();
  stream->write(byteToSend);
  setDelay(char_send_time);
}

// private
void Tprinter::setDelayBitmap(uint16_t width, uint16_t height, uint16_t blackPixels) {
  if (dtrEnabled) return;
  if (calculateMode) {
    uint16_t totalPixels = width * height;
    unsigned long time =
        (blackPixels / totalPixels) *
        (((totalPixels / ((heating_dots + 1) * 8)) * 10 * (heating_time + heating_interval)));
    setDelay(time);
    return;
  }
  setDelay((oneDotHeight_printTime + oneDotHeight_feedTime) * height);
}

void Tprinter::printBitmap(uint8_t *bitmap, uint16_t width, uint16_t height, uint8_t scale, bool center) {
  uint8_t maxScale = widthInDots / width;  // widthInDots = 384;
  if (scale == 0 || scale > maxScale) scale = maxScale;

  bool isMarginAdded = {};
  const uint8_t marginWidth = center ? (widthInDots - scale * width) / 2 : 0;
  uint8_t marginCounter = {};

  const uint8_t bytesPerRow = (scale * width + marginWidth + 7) / 8;

  const uint8_t bufferSize = (printerBufferLimit / bytesPerRow) * bytesPerRow;
  uint8_t bufferFillLevel = {};

  uint8_t rowsInPackage = bufferSize / bytesPerRow;
  uint16_t totalRowsToSend = height * scale;
  const uint16_t printedWidth = width * scale + marginWidth;
  uint16_t currentDotNr = {};

  uint8_t dot = {};
  uint8_t byteToSend = {};
  uint8_t offset = {7};

  uint16_t burned{};

  feed(1);

  for (uint16_t row = 0; row < height; row++) {
    for (uint8_t rowFat = 0; rowFat < scale; rowFat++) {
      isMarginAdded = false;
      marginCounter = 0;

      for (uint16_t column = 0; column < width; column++) {
        if (center && !isMarginAdded && marginCounter == marginWidth) {
          isMarginAdded = true;
          column = 0;
        }
        if (center && !isMarginAdded) {
          column = 0;
          dot = 0;
          marginCounter++;
        } else {
          dot = bitRead(bitmap[(row * width + column) / 8], 7 - (row * width + column) % 8);
        }

        for (uint8_t columnFat = 0; columnFat < scale; columnFat++) {
          if (center && !isMarginAdded) columnFat = scale;
          byteToSend |= dot << offset;
          currentDotNr++;
          if (!dtrEnabled && dot) burned++;
          if (currentDotNr == printedWidth) {  // end of line
            offset = 0;
            currentDotNr = 0;
          }
          if (offset == 0) {
            if (bufferFillLevel == 0) {
              if (rowsInPackage > totalRowsToSend) rowsInPackage = totalRowsToSend;
              initBitmapData(rowsInPackage, bytesPerRow);
            }

            sendBitmapByte(byteToSend);
            offset = 7;
            byteToSend = 0;
            bufferFillLevel++;

            if (bufferFillLevel == bufferSize) {
              setDelayBitmap(printedWidth, rowsInPackage, burned);
              totalRowsToSend -= rowsInPackage;
              bufferFillLevel = 0;
              burned = 0;
            }
          } else {
            offset--;
          }
        }
      }
    }
  }
}
