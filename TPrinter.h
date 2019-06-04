/* made by BinaryWorlds
Not for commercial use,
in other case by free to use it.

I am not responsible for errors in the library. I deliver it "as it is".
I will be grateful for all suggestions.
*/

#ifndef TPrinter_h
#define TPrinter_h
#include "Arduino.h"

/* Tested on firmware 2.69 and JP-QR701
Some features may not work on the older firmware.*/

#define HT    9     // HorizontalTab
#define LF    10    // 0x0A Line feed
#define CR    13    // 0x0D Carriage return
#define SPACE 32    // 0x20 Space
#define ESC   27    // 0x1B Escape
#define FS    28    // 0x1C Field separator

#define FONT_B        1         //Font B: 9x17, Standard font A: 12x24
#define DARK_MODE     (1 << 1)  // anti-white mode, didnt work?
#define UPSIDE_DOWN   (1 << 2)  //didnt work?, use invert(bool);
#define BOLD          (1 << 3)
#define DOUBLE_HEIGHT (1 << 4)
#define DOUBLE_WIDTH  (1 << 5)
#define STRIKEOUT     (1 << 6)  // didnt work?

                                /*from translator */
#define CODEPAGE_CP437        0 // USA, European Standard
#define CODEPAGE_KATAKANA     1
#define CODEPAGE_CP850        2 // Multilingual
#define CODEPAGE_CP860        3 // Portugal
#define CODEPAGE_CP863        4 // Canada-French
#define CODEPAGE_CP865        5 // Nordic
#define CODEPAGE_WCP1251      6 // Slavic
#define CODEPAGE_CP866        7 // Slavic 2
#define CODEPAGE_MIK          8 // Slavic/Bulgarian
#define CODEPAGE_CP755        9 // East Europe, Latvia 2
#define CODEPAGE_IRAN         10 //Persia
#define CODEPAGE_CP862        15 // Hebrew
#define CODEPAGE_WCP1252      16 // Latin 1
#define CODEPAGE_WCP1253      17 // Greece
#define CODEPAGE_CP852        18 // Latin 2
#define CODEPAGE_CP858        19 // Multilingual Latin 1 + ?
#define CODEPAGE_IRAN2        20 //Perisan
#define CODEPAGE_LATVIA       21
#define CODEPAGE_CP864        22 // Arabic
#define CODEPAGE_ISO_8859_1   23 // Western Europe
#define CODEPAGE_CP737        24 // Greece
#define CODEPAGE_WCP1257      25 // Baltic
#define CODEPAGE_THAI         26
#define CODEPAGE_CP720        27 // Arabic
#define CODEPAGE_CP855        28
#define CODEPAGE_CP857        29 // Turkish
#define CODEPAGE_WCP1250      30 // Central Europe
#define CODEPAGE_CP775        31
#define CODEPAGE_WCP1254      32 // Turkish
#define CODEPAGE_WCP1255      33 // Hebrew
#define CODEPAGE_WCP1256      34 // Arabic
#define CODEPAGE_WCP1258      35 // Vietnamese
#define CODEPAGE_ISO_8859_2   36 // Latin 2
#define CODEPAGE_ISO_8859_3   37 // Latin 3
#define CODEPAGE_ISO_8859_4   38 // Baltic
#define CODEPAGE_ISO_8859_5   39 // Slavic
#define CODEPAGE_ISO_8859_6   40 // Arabic
#define CODEPAGE_ISO_8859_7   41 // Greek
#define CODEPAGE_ISO_8859_8   42 // Hebrew
#define CODEPAGE_ISO_8859_9   43 // Turkish
#define CODEPAGE_ISO_8859_15  44 // Latin 9
#define CODEPAGE_THAI2        45
#define CODEPAGE_CP856        46
#define CODEPAGE_CP874        47

#define CHARSET_USA           0
#define CHARSET_FRANCE        1
#define CHARSET_GERMANY       2
#define CHARSET_UK            3
#define CHARSET_DENMARK1      4
#define CHARSET_SWEDEN        5
#define CHARSET_ITALY         6
#define CHARSET_SPAIN_1       7
#define CHARSET_JAPAN         8
#define CHARSET_NORWAY        9
#define CHARSET_DENMARK_2     10
#define CHARSET_SPAIN2        11
#define CHARSET_LATIN_AMERICA 12
#define CHARSET_SOUTH_KOREA   13
#define CHARSET_SLOVENIA      14
#define CHARSET_CHINA         15

class Tprinter : public Print {

private:

  bool dtrEnabled{false};
  Stream  *stream;
  int baudrate{9600}; //19200

  const uint16_t widthInDots = 384; /* to calculate aboslute position,
   when using some functions like setCharSpacing*/

  uint16_t
  cursor{}, //actual position, n of 384
  tabs[32] = {}; // default in printer: 8,16,24,32 (*12dots) - doesn't work in my case

  uint8_t
    dtrPin{},
    tabsAmount = {0},
    widthMax{32},//max char per line
    charHeight{24}, //dots
    charWidth{12}, //dots
    interlineHeight{6}, //dots
    charSpacing{}, //dots
    printMode{};
  unsigned long
    endPoint{},
    char_send_time{},
    oneDotHeight_printTime{30000},
    oneDotHeight_feedTime{2100},
    feed_time{63000}, //for 6 dot interline
    print_time{720000}; //for 24 high dot

  void update();


  public:
    Tprinter(Stream *s, int baud = 9600);

    size_t write(uint8_t sign);//from inherited Print class, u can use println() ...
    void feed(uint8_t n = 1); // feed n lines

    void enableDtr(uint8_t dtr); // pin nr
    void disableDtr(bool mode = 1); // 1 = pullup, 0 = pulldown

    void wait();
    void setDelay(unsigned long time);// microseconds; ignored if Dtr pin is disable

    void setCodePage(uint8_t page = 36);
    void setCharset(uint8_t val = 14);

    void setTimes(unsigned long p = 30000, unsigned long f = 2100);//ignored if Dtr pin is disabled
    void setHeat(uint8_t dots = 11,
       uint8_t time = 255,
       uint8_t interval = 40);
/*
dots: default 9 (80dots) becouse (9+1)* 8
units = 8 dots;
time: default 80 (800 us); units: 10 us
interval: default 2 (20us); units :10 us
*/
    void setMode(uint8_t = 0, uint8_t = 0, uint8_t = 0,
      uint8_t = 0, uint8_t = 0, uint8_t = 0, uint8_t = 0);
    void unsetMode(uint8_t = 0, uint8_t = 0, uint8_t = 0,
      uint8_t = 0, uint8_t = 0, uint8_t = 0, uint8_t = 0);

    void invert(bool n = 0); // 1 - invert ON
    void justify(char val); // 'L' - Left 'C' - center 'R' - right
    void underline(uint8_t  n); // off 0 - 2 max
    void setInterline(uint8_t n); //default: 6
    void setCharSpacing(uint8_t n = 0); //size x 0.125 millimeters(1dot); x2 if double width
    void setTabs(uint8_t* tab = 0, uint8_t size = 0);
    void clearTabs();
    void tab();

    void reset();
    void begin(); // simply start; edit default value for ur preference; use reset() first if u need
    void offline();
    void online();

    void identifyChars(char* tab); //require Serial.print(baudrate); in void setup()
    void printFromSerial(); //require Serial.print(baudrate); in void setup()
    void printCharset();  //print chars, 0x23-0x7E range
    void printCodepage(); //print chars, 0x80-0xFF range
    uint16_t printPosition(); //require Serial.print(baudrate); in void setup(); print actual position (n of 384)

};

#endif

/*
to do:
strikeout
sleep and wake

new type of timeout !!


no:
user-definied character
barcode
bitmap
*/
