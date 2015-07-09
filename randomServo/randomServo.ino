/* Sweep
 by BARRAGAN <http://barraganstudio.com> 
 This example code is in the public domain.

 modified 8 Nov 2013
 by Scott Fitzgerald
 http://arduino.cc/en/Tutorial/Sweep
*/ 

#include <Servo.h>
#include <EEPROM.h>

 
Servo myservo;  // create servo object to control a servo 
                // twelve servo objects can be created on most boards
int potPin = 0;
int middlePin = 9;
int leftPin = 8;
int rightPin = 7;
int val = 0;
int target_pos = 0;    // variable to store the servo position
int last_pos = 0;  
int pos = 0;
int sweep_delay = 0;
int loop_delay = 0;

int middlePos = 0;
int leftSide = 0;
int rightSide = 0;

int starttime = 0; 
int address = 0;
 
void setup() 
{ 
  
  starttime = millis();
  pinMode(middlePin, INPUT);
  pinMode(leftPin, INPUT);
  pinMode(rightPin, INPUT);
  middlePos = EEPROM.read(address);
  leftSide = EEPROM.read(address+1);
  rightSide = EEPROM.read(address+2);
  myservo.attach(13);  // attaches the servo on pin 9 to the servo object 
  myservo.write(middlePos);
} 
 
void loop() 

{ 
    // check whether for calibration mode
    if (digitalRead(middlePin) == HIGH ){
      val = analogRead(potPin); 
      // if < 10sec, then calibrate middle pos
        middlePos = map(val, 0, 1023, 60, 120);
         myservo.write(middlePos);
         delay(100);     
         EEPROM.write(address, middlePos);        
       // next calibrate left side of servo
    } else if (digitalRead(leftPin) == HIGH ) {
         val = analogRead(potPin); 
        leftSide = map(val, 0, 1023, 0, 60);  
        myservo.write(middlePos - leftSide);
         delay(100);     
         EEPROM.write(address + 1, leftSide);
    } else if (digitalRead(rightPin) == HIGH ){
         val = analogRead(potPin); 
         rightSide = map(val, 0, 1023, 0, 60);  
         myservo.write(middlePos + rightSide);
         delay(100);     
         EEPROM.write(address + 2, rightSide);
    } else {
      loop_delay = random(50,1000); 
      sweep_delay = random(1,20);
      target_pos = random(middlePos - leftSide, middlePos + rightSide);
      while (pos != target_pos) {
       (pos < target_pos) ? pos++ : pos--; 
        myservo.write(pos);              // tell servo to go to position in variable 'pos' 
        delay(sweep_delay);
      }  
      delay(loop_delay);                       
      last_pos = target_pos;
    }
} 

