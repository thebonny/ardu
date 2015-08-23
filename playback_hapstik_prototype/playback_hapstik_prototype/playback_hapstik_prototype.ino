#include <Servo.h>
#include <SD.h>
#include <SPI.h>
 
Servo pitch_servo; 
Servo rudder_servo; 
Servo nick_servo; 
Servo roll_servo; 

unsigned int up = 1;

unsigned long useconds = 1000;                                        

File myFile;


unsigned int SERVO_PITCH_PIN = 8;
unsigned int SERVO_RUDDER_PIN = 9;
unsigned int SERVO_NICK_PIN = 12;
unsigned int SERVO_ROLL_PIN = 13;


void setup() {
   Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
   // put your setup code here, to run once:
   pitch_servo.attach(SERVO_PITCH_PIN);
   rudder_servo.attach(SERVO_RUDDER_PIN);
   nick_servo.attach(SERVO_NICK_PIN);
   roll_servo.attach(SERVO_ROLL_PIN);
   Serial.print("Initializing SD card...");
  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin 
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output 
  // or the SD library functions will not work. 
   pinMode(10, OUTPUT);
   digitalWrite(10, HIGH);
   
  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
   

}

void loop() {
    
   
      pitch_servo.writeMicroseconds(useconds);
      delay (10);
      rudder_servo.writeMicroseconds(useconds);
      delay (10);
      nick_servo.writeMicroseconds(useconds);
            delay (10);
      roll_servo.writeMicroseconds(useconds);
      delay (10);
      
      if (up) {
      useconds += 5;
      } else {
      useconds -= 5;
      }      
      
      if (useconds == 2000) {
        up = 0;
      } else if (useconds == 1000) {
        up = 1;
      }
      Serial.println(useconds);
  
}




