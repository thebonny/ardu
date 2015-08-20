#include <SD.h>
#include <SPI.h>
 
File myFile;

unsigned long duration = 0;
unsigned long start = 0;

unsigned int RECORD_PITCH_PIN = 6;
unsigned int RECORD_RUDDER_PIN = 7;
unsigned int RECORD_NICK_PIN = 8;
unsigned int RECORD_ROLL_PIN = 9;


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
   
  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
  // re-open the file for reading:
   
  start = millis();
}

void loop() {
    
      writeToFile( RECORD_PITCH_PIN, "pitch.txt");
      writeToFile( RECORD_RUDDER_PIN, "rudder.txt");
      writeToFile( RECORD_NICK_PIN, "nick.txt");
      writeToFile( RECORD_ROLL_PIN, "roll.txt");
   
    
}

void writeToFile(unsigned int pin, const char *filename) {
       duration = pulseIn(pin, HIGH);
        File dataFile = SD.open(filename, FILE_WRITE);

        // if the file is available, write to it:
        if (dataFile) {
           dataFile.println(duration);
           dataFile.close();
        } else {
           Serial.println("error opening datalog.txt");
        }
}




