#include <Servo.h>
#include <SD.h>
#include <SPI.h>
 
Servo myservo; 

File myFile;

unsigned long duration = 0;
unsigned long start = 0;

unsigned int opened = 0;
//**
//* MODE 0 = start heli
//* MODE 1 = record start
//* MODE 2 = record stop
//*/

unsigned int mode = 0;

unsigned int SERVO_PIN = 7;
unsigned int RECORD_CHANNEL_PIN = 6;


void setup() {
   Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
   // put your setup code here, to run once:
   myservo.attach(SERVO_PIN);
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
    
    // check every second for mode shift an channel 6
    if ( (millis() - start) > 1000) {
      if ( pulseIn(MODE_PIN, HIGH) < 1010 ) {
        mode = 1;
      }
      start = millis();
    }
    if ( mode == 0 ) {
    duration = pulseIn(RECORD_CHANNEL_PIN, HIGH);
 
   
   File dataFile = SD.open("pwm.txt", FILE_WRITE);

    // if the file is available, write to it:
    if (dataFile) {
      myservo.writeMicroseconds(duration);
      dataFile.println(duration);
      dataFile.close();
    }
    // if the file isn't open, pop up an error:
    else {
      Serial.println("error opening datalog.txt");
    }
    } else {
      if (opened == 0) {
        myFile = SD.open("pwm.txt");
        opened = 1;
      }
     
      char result[4]; 
      uint8_t i = 0; 
      
      int  data = myFile.read();
      
      while (data != 13) {
         result[i] = data; // store character in result
         i++; // increase counter
         data = myFile.read();
      }
        
      unsigned long number = atol(result); // convert result string to numeric value
      Serial.println(number); // print the number     
      
      myservo.writeMicroseconds(number);
      
      delay(19);
  }
}




