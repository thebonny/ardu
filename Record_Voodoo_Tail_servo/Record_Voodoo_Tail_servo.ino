#include <SD.h>
#include <SPI.h>

 
File myFile;

unsigned long duration = 0;
unsigned long start = 0;

//**
//* MODE 0 = stop record
//* MODE 1 = start record
//*/

unsigned int mode = 0;

unsigned int MODE_PIN = 7;
unsigned int RECORD_CHANNEL_PIN = 6;


char * TimeToString(unsigned long t) {
  static char str[12];
  long h = t / 3600;
  t = t % 3600;
  int m = t / 60;
  int s = t % 60;
  sprintf(str, "%04ld:%02d:%02d", h,m,s);
  return str;
  
  }


void setup() {
   Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  
   Serial.print("Initializing SD card...");
  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin 
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output 
  // or the SD library functions will not work. 
   pinMode(10, OUTPUT);
   
  if (!SD.begin(10)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
  // re-open the file for reading:
   
  start = millis();
}

void loop() {
    
    // check every second for mode shift an channel 6
    if ( (millis() - start) > 1000) {
      if ( pulseIn(MODE_PIN, HIGH) > 1700 ) {
        // idle up 1 --> no record
        mode = 1;
      } else {
        // idle up 2 or higher --> record
        mode = 1;
      }
      start = millis();
    }
    
    if (mode == 1) {
      duration = pulseIn(RECORD_CHANNEL_PIN, HIGH);
      File dataFile = SD.open("pwm.txt", FILE_WRITE);
      // if the file is available, write to it:
      if (dataFile) {
        dataFile.println(duration);
     
        dataFile.close();
      }
    }
     
}




