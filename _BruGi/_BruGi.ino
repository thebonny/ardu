#include <LiquidCrystal_I2C.h>




/*************************/
/* Include Header Files  */
/*************************/

#include <Wire.h>

#include "definitions.h"



#include "PinChangeInt.h"
#include "Timer1.h"
#include "variables.h"
#include "BLcontroller.h"         // Motor Movement Functions and Timer Config

int i = 0;
int power = 0;
uint8_t dir = 0;

/**********************************************/
/* Initialization                             */
/**********************************************/
void setup() 
{

  CH2_PINMODE
  CH3_PINMODE

  // Start Serial Port
  Serial.begin(115200);

  // Init BL Controller
  initBlController();
  // Init Sinus Arrays
  initMotorStuff();
  
 

  LEDPIN_OFF
  CH2_OFF
  CH3_OFF

}





/**********************************************/
/* Main Loop                                  */
/**********************************************/
void loop() 
{ 
i++;
    int sensorValue = analogRead(A0);
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  int pwm = map(sensorValue,0,1023,0,2048);
  // print out the value you read:

MoveMotorPosSpeed(i, 30);
delay(100);



   

}


