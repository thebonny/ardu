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
    pinMode(5, INPUT);
    pinMode(6, INPUT);
    pinMode(7, INPUT);
  

  PCMSK2 |= 0xE4; // set the mask to allow those 6 pins to generate interrupts
  PCICR |= 0x04;  // enable interupt for port D

  start = millis();
}

void loop() {
  
  
  
  time = millis() - start;
  if (IR_ACTIVE) {
    if ( ((time % 40) == 0) | (time == 0) ) {
      Serial.flush();  
      Serial.print(counter);
      Serial.print("\t");
      Serial.print(time);
      Serial.print("\t");
      Serial.print(uSec[0]);
      Serial.print("\t");
       Serial.print(uSec[3]);
      Serial.print("\t");
       Serial.print(uSec[4]);
      Serial.print("\t");
       Serial.print(uSec[5]);
  
      Serial.println();
      counter++;
    }
  }
 
  
 
}
