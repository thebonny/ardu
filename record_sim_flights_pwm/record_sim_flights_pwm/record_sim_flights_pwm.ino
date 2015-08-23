#include <SPI.h>
#include <SD.h>


const int chipSelect = 4;

volatile uint8_t prev; // remembers state of input bits from previous interrupt
volatile uint32_t risingEdge[6]; // time of last rising edge for each channel
volatile double uSec[6]; // the latest measured pulse width for each channel
unsigned int long time;
unsigned int long start;
unsigned long counter;
unsigned int fileCounter = 1;
unsigned short IR_ACTIVE = 0;
String filename = "stik_1.txt";
const double HALF_SWING_ANGLE = 0.392699081698724;
const double MITTENIMPULS = 1500.0;
const double AUSSCHLAG = 500.0;



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

  while (SD.exists( (char*) filename.c_str() )){
    fileCounter++;
    filename = "2308_";
    filename += fileCounter;
    filename += ".txt";
  }
  
  // TODO write header in datei
  
     String header = "";
      header += "Bild";
      header += "\t";
      header += "Nick";
      header += "\t";
      header += "Roll";       
      header += "\t";
      header += "Gas";    
      header += "\t";
      header += "Ruder";    
      header += "\t";
      header += "Pitch";   
      header += "\t";
      header += "Nick R";
      header += "\t";
      header += "Roll R";       
      header += "\t";
      header += "Gas R";    
      header += "\t";
      header += "Ruder R";    
      header += "\t";
      header += "Pitch R";   
 
      File dataFile = SD.open( filename.c_str(), FILE_WRITE);
  
      // if the file is available, write to it:
      if (dataFile) {
        dataFile.println(header);
        dataFile.close();
      } else {
        Serial.println("error opening datalog.txt");
      }
        Serial.println(header);
        
      start = millis();
}

uint32_t mapTo100(uint32_t raw) {
  return map(raw, 1000, 2000, 0, 100);
//  return raw;
}

double mapToRadiens(uint32_t raw) {
 return ((raw-MITTENIMPULS)/AUSSCHLAG)*HALF_SWING_ANGLE;
}

String printFloat(double value, int places) {
  // this is used to cast digits 
  int digit;
  float tens = 0.1;
  int tenscount = 0;
  int i;
  float tempfloat = value;
  String printed = "";

    // make sure we round properly. this could use pow from <math.h>, but doesn't seem worth the import
  // if this rounding step isn't here, the value  54.321 prints as 54.3209

  // calculate rounding term d:   0.5/pow(10,places)  
  float d = 0.5;
  if (value < 0)
    d *= -1.0;
  // divide by ten for each decimal place
  for (i = 0; i < places; i++)
    d/= 10.0;    
  // this small addition, combined with truncation will round our values properly 
  tempfloat +=  d;

  // first get value tens to be the large power of ten less than value
  // tenscount isn't necessary but it would be useful if you wanted to know after this how many chars the number will take

  if (value < 0)
    tempfloat *= -1.0;
  while ((tens * 10.0) <= tempfloat) {
    tens *= 10.0;
    tenscount += 1;
  }


  // write out the negative if needed
  if (value < 0)
    printed += "-";

  if (tenscount == 0)
        printed += "0";

  for (i=0; i< tenscount; i++) {
    digit = (int) (tempfloat/tens);
    printed += digit;
    tempfloat = tempfloat - ((float)digit * tens);
    tens /= 10.0;
  }

  // if no places after decimal, stop now and return
  if (places <= 0)
    return printed;

  // otherwise, write the point and continue on
  printed += ".";

  // now write out each decimal place by shifting digits one by one into the ones place and writing the truncated value
  for (i = 0; i < places; i++) {
    tempfloat *= 10.0; 
    digit = (int) tempfloat;
      printed += digit;    
    // once written, subtract off that digit
    tempfloat = tempfloat - (float) digit; 
  }
  return printed;
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
      stickmoves += "\t";
      stickmoves +=    printFloat(mapToRadiens(uSec[0]), 8);   
      stickmoves += "\t";
      stickmoves +=    printFloat(mapToRadiens(uSec[1]), 8);   
      stickmoves += "\t";
      stickmoves +=    printFloat(mapToRadiens(uSec[3]), 8);   
      stickmoves += "\t";
      stickmoves +=    printFloat(mapToRadiens(uSec[4]), 8);   
      stickmoves += "\t";
      stickmoves +=    printFloat(mapToRadiens(uSec[5]), 8);   
          
     File dataFile = SD.open( filename.c_str(), FILE_WRITE);
  
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
