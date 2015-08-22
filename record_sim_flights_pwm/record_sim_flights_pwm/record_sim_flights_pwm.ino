#include <SPI.h>
#include <SD.h>


const int chipSelect = 4;

volatile uint8_t prev; // remembers state of input bits from previous interrupt
volatile uint32_t risingEdge[6]; // time of last rising edge for each channel
volatile uint32_t uSec[6]; // the latest measured pulse width for each channel
unsigned int long time;
unsigned int long start;
unsigned long counter;
unsigned short IR_ACTIVE = 0;


char * TimeToString(unsigned long t) {
  static char str[12];
  long h = t / 3600;
  t = t % 3600;
  int m = t / 60;
  int s = t % 60;
  sprintf(str, "%04ld:%02d:%02d", h,m,s);
  return str;
  
  }

ISR(PCINT2_vect) { // one or more of pins 2~7 have changed state
  uint32_t now = micros();
  uint8_t curr = PIND; // current state of the 6 input bits
  uint8_t changed = curr ^ prev;
  int channel = 0;
  for (uint8_t mask = 0x04; mask; mask <<= 1) {
    if (changed & mask) { // this pin has changed state
      if (curr & mask) { // +ve edge so remember time
        risingEdge[channel] = now;
      }
      else { // -ve edge so store pulse width
        uSec[channel] = now - risingEdge[channel];
      }
    }
    channel++;
  }
  prev = curr;
  IR_ACTIVE = 1;
}

void setup() {
  counter = 1;
 
  Serial.begin(9600);
    pinMode(2, INPUT);
    pinMode(3, INPUT);
   
    pinMode(5, INPUT);
    pinMode(6, INPUT);
    pinMode(7, INPUT);
  

  PCMSK2 |= 0xFC; // set the mask to allow those 6 pins to generate interrupts
  PCICR |= 0x04;  // enable interupt for port D
  
  Serial.print("\nInitializing SD card...");
  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output
  // or the SD library functions will not work.
  pinMode(10, OUTPUT);     // change this to 53 on a mega
  digitalWrite(10, HIGH);
  
 if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  start = millis();
}

uint32_t mapTo100(uint32_t raw) {
 // return map(raw, 1000, 2000, 0, 100);
return raw;  
}

void loop() {
  
  
  
  time = millis() - start;
  if (IR_ACTIVE) {
    if ( ((time % 40) == 0) | (time == 0) ) {
      String stickmoves = "";
      stickmoves += counter;
      stickmoves += "\t";
      stickmoves += mapTo100(uSec[0]);
      stickmoves += "\t";
      stickmoves += mapTo100(uSec[1]);       
      stickmoves += "\t";
      stickmoves += mapTo100(uSec[3]);    
      stickmoves += "\t";
      stickmoves += mapTo100(uSec[4]);    
      stickmoves += "\t";
      stickmoves += mapTo100(uSec[5]);    
        
      File dataFile = SD.open("datalog.txt", FILE_WRITE);
  
      // if the file is available, write to it:
      if (dataFile) {
        dataFile.println(stickmoves);
        dataFile.close();
      } else {
        Serial.println("error opening datalog.txt");
      }
      Serial.flush();  
      Serial.println(stickmoves);
      counter++;
    }
  }
 
  
 
}
