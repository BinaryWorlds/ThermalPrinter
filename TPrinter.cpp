/* made by BinaryWorlds
Not for commercial use,
in other case by free to use it. Just copy this text and link to oryginal repository: https://github.com/BinaryWorlds/ThermalPrinter

I am not responsible for errors in the library. I deliver it "as it is".
I will be grateful for all suggestions.
*/
#include "TPrinter.h"

Tprinter::Tprinter(Stream *s, int baud):
stream(s), baudrate(baud){
  char_send_time = (11*1000000/baud); //for 8N1
}

void Tprinter::update(){ /*private*/
  charHeight = (printMode & FONT_B) ? 17 : 24; // B : A
  charWidth  = (printMode & FONT_B) ? 9 : 12;
  widthMax   = (printMode & FONT_B) ? 42 : 32;
  if(printMode & DOUBLE_WIDTH){
    charWidth *= 2;
    widthMax /= 2;
  }
  if(printMode & DOUBLE_HEIGHT) charHeight *= 2;
  print_time = charHeight * oneDotHeight_printTime;
  feed_time = (charHeight + interlineHeight) * oneDotHeight_feedTime;
}

size_t Tprinter::write(uint8_t sign){

  if(sign != CR){
    wait();
    unsigned long val = char_send_time;
    stream->write(sign);
    cursor += charWidth + charSpacing;
    if(printMode & DOUBLE_WIDTH) cursor += charSpacing;
    if(cursor >= (widthInDots - 6) && sign != '\n') stream->write(sign ='\n');// if font_B 384 - 42*9 = 6 no matters in other case; or (widthInDots - cursor)<charWidth,
    /* force the printout of a new line;
     printer print a line after took widthMax + 1 char*/
    if(sign == '\n') {
      val += print_time + feed_time;/* if only feed, still use print time. why not?*/
      cursor = 0;
    }
    setDelay(val);
    //printPosition();
  }
  return 1;
}

void Tprinter::feed(uint8_t n){
  wait();
  stream->write(ESC);
  stream->write('d');
  stream->write(n);
  setDelay(3*char_send_time + feed_time * n);
  cursor = 0;
}

void Tprinter::enableDtr(uint8_t dtr){
  if(dtrEnabled && dtrPin != dtr) disableDtr(); //useless but possible
  dtrPin = dtr;
  pinMode(dtrPin, INPUT);
  dtrEnabled = true;
}

void Tprinter::disableDtr(bool mode){ //if pin unused - pull them for decrease(vs INPUT mode) EMI, noisy and power concumption
  if(mode)pinMode(dtrPin, INPUT_PULLUP);
  else pinMode(dtrPin, INPUT_PULLDOWN);
  dtrEnabled = false;
}

void inline Tprinter::wait(){
  if(dtrEnabled) while(digitalRead(dtrPin));
  else while(long(micros() - endPoint) < 0);
}

void inline Tprinter::setDelay(unsigned long time){
  if(!dtrEnabled) endPoint = (micros() +  time);
}

/* Set the same encoding in text editor as in the printer! */
void Tprinter::setCodePage(uint8_t page){
  if(page > 47) page = 47;

  wait();
  stream->write(FS);
  stream->write('.'); //kanji mode off

  stream->write(ESC);
  stream->write('t');
  stream->write(page);
  setDelay(5*char_send_time);
}

void Tprinter::setCharset(uint8_t val){
  if(val > 15) val = 15;
  wait();

  stream->write(ESC);
  stream->write('R');
  stream->write(val);
  setDelay(3*char_send_time);
}

void Tprinter::setTimes(unsigned long p, unsigned long f){
  oneDotHeight_printTime = p;
  oneDotHeight_feedTime = f;

  print_time = p * charHeight;
  feed_time = f * (interlineHeight + charHeight);
}

void Tprinter::setHeat(uint8_t dots, uint8_t time, uint8_t interval){
  wait();
  stream->write(ESC);
  stream->write('7');
  stream->write(dots);
  stream->write(time);
  stream->write(interval);
  setDelay(5*char_send_time);
}

void Tprinter::setMode(uint8_t m1, uint8_t m2, uint8_t m3,
   uint8_t m4, uint8_t m5, uint8_t m6, uint8_t m7){

   wait();
   printMode |= (m1 + m2 + m3 + m4 + m5 + m6 + m7);
   stream->write(ESC);
   stream->write('!');
   stream->write(printMode);
   setDelay(3*char_send_time);
   update();
}

void Tprinter::unsetMode(uint8_t m1, uint8_t m2, uint8_t m3,
   uint8_t m4, uint8_t m5, uint8_t m6, uint8_t m7){

   wait();
   printMode &= ~(m1 + m2 + m3 + m4 + m5 + m6 + m7);
   stream->write(ESC);
   stream->write('!');
   stream->write(printMode);
   setDelay(3*char_send_time);

   update();
}

void Tprinter::invert(bool n){
  wait();
  stream->write(ESC);
  stream->write('{');
  stream->write(n);
  setDelay(3*char_send_time);
}

void Tprinter::justify(char val){
  uint8_t set{};
  wait();

  switch (val) {
    case 'L':
      set = 0; break;
    case 'C':
      set = 1; break;
    case 'R':
      set = 2; break;
  }
  stream->write(ESC);
  stream->write('a');
  stream->write(set);
  setDelay(3*char_send_time);
}

void Tprinter::underline(uint8_t n){
  if(n > 2) n = 2;
  wait();
  stream->write(ESC);
  stream->write('-');
  stream->write(n);
  setDelay(3*char_send_time);
}

void Tprinter::setInterline(uint8_t n){ // ESC '2' - back to default
  wait();
  stream->write(ESC);
  stream->write('3');
  if(n + charHeight >= 255) interlineHeight = 255;
  else  interlineHeight = n + charHeight;
  stream->write(interlineHeight);
  interlineHeight -= charHeight;

  update();
  setDelay(3*char_send_time);
}

void Tprinter::setCharSpacing(uint8_t n){ // printer default: 0
  wait();
  stream->write(ESC);
  stream->write(SPACE);
  stream->write(charSpacing = n);
  setDelay(3*char_send_time);
}

/*Enter values ​​in ascending order.
The next value can't be equal to or less than the previous one
 - the printer will consider it as a data
  (will finish setting the tabs) */
void Tprinter::setTabs(uint8_t* tab, uint8_t size){ //sets as absolute position;
  tabs[tabsAmount = 0] = 0;
  wait();
  stream->write(ESC);
  stream->write('D');

  for(uint8_t i = 0; i<size; i++){
    if(tab[i] < widthMax && tab[i] > (tabs[tabsAmount] / charWidth) && tabsAmount < 32){
      stream->write(tab[i]);
      tabs[tabsAmount++] = tab[i] * charWidth;
    }
  }
  stream->write((uint8_t)0);//from datasheet - end of list
  cursor = 0;
  setDelay((tabsAmount +3)*char_send_time);
}

/* If you don't set the next horizontal tab position
(bigger than actual cursor position),
the command is ignored.*/
void Tprinter::tab(){
  for(uint8_t i = 0; i < tabsAmount; i++){
    if(tabs[i] > cursor){
      cursor = tabs[i];
      break;
    }
  }
  stream->write(HT);

  if(cursor >= (widthInDots-6)){
    setDelay(char_send_time + print_time + feed_time);
    cursor = 0; // printer go newline
  }
  else setDelay(char_send_time);
}

void Tprinter::clearTabs(){
  stream->write(ESC);
  stream->write('D');
  stream->write('0');
  setDelay(3*char_send_time);

  tabs[tabsAmount = 0] = 0;
}

void Tprinter::reset(){
  stream->write(ESC);
  stream->write('@');
  setDelay(2*char_send_time);

  cursor = 0;
  tabs[tabsAmount = 0] = 0;
  interlineHeight = 6;
  printMode = 0;
  charSpacing = 0;
  update();
}

void Tprinter::begin(){
  setDelay(3000000); // 3s
  wait();
  online();
  setHeat();
  setCodePage();
  setCharset();
  update();
  uint8_t list[]={4,8,12,16,20,24,28,32,36,40,44,48};
  setTabs(list, 12);// if widthMax == 32, last tab is 28
}

void Tprinter::offline(){
  wait();
  stream->write(ESC);
  stream->write('=');
  stream->write((uint8_t)0);
  setDelay(3*char_send_time);
}

void Tprinter::online(){
  wait();
  stream->write(ESC);
  stream->write('=');
  stream->write((uint8_t)1);
  setDelay(3*char_send_time);
}

/*for test*/
void Tprinter::identifyChars(char* tab){ // dont use it in the same time with printFromSerial)()
  int i{};
  Serial.println();
  Serial.println(F("Separate letters with a space, e.g \"ą ć d\"!"));
  do{
    if(tab[i] != SPACE){
      int val = i;
      while(tab[i] != SPACE && tab[i] != 0){
        Serial.print(tab[i]);
        i++;
      }
      Serial.print(F(" HEX: "));
      while(val != i){
        Serial.print(tab[val], HEX);
        val++;
      }
      Serial.println();
    }else i++;
  }while(tab[i] != 0); //end of string
}

void Tprinter::printFromSerial(){// u can use it e.g for test; require Serial.begin(baudrate) in void setup()
  wait();
  while(Serial.available()){
    char sign{};
    sign = (char) Serial.read();
    print(sign);
  }
}

void Tprinter::printCharset(){
  wait();
  println();
  print(F("   01234567 89ABCDEF"));
  for(uint8_t i=32; i<128; i++){
    if((i % 16) == 0){
      println();
      print(i/16, DEC);
      print(F("- "));
    }
    write(i);
    if((i % 16) == 7) print(F(" "));
  }
   println();
}

void Tprinter::printCodepage(){
  wait();
  println();
  print(F("   01234567 89ABCDEF"));
  for(uint8_t i=128; i<255; i++){
    if((i % 16) == 0){
      println();
      char val = 'A';
      val += i/16 - 10;
      if(i/16 < 10) print(i/16, DEC);
      else print(val);
      print(F("- "));
    }
    write(i);
    if((i % 16) == 7) print(F(" "));
  }
   println();
}

uint16_t Tprinter::printPosition(){
  Serial.print("Actual position: ");
  Serial.println(cursor, DEC);
  return cursor;
}
