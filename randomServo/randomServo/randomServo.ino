/* Sweep
 by BARRAGAN <http://barraganstudio.com> 
 This example code is in the public domain.

 modified 8 Nov 2013
 by Scott Fitzgerald
 http://arduino.cc/en/Tutorial/Sweep
*/ 

#include <Servo.h>

 
Servo myservo;  // create servo object to control a servo 
                // twelve servo objects can be created on most boards
 
int target_pos = 0;    // variable to store the servo position
int last_pos = 0;  
int pos = 0;
int sweep_delay = 0;
int loop_delay = 0;
 
void setup() 
{ 
  myservo.attach(13);  // attaches the servo on pin 9 to the servo object 
} 
 
void loop() 
{ 
    loop_delay = random(50,1000); 
    sweep_delay = random(1,20);
    target_pos = random(45, 135);
    while (pos != target_pos) {
     (pos < target_pos) ? pos++ : pos--; 
      myservo.write(pos);              // tell servo to go to position in variable 'pos' 
      delay(sweep_delay);
    }  
    delay(loop_delay);                       // waits 15ms for the servo to reach the position 
    last_pos = target_pos;
} 

