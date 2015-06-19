


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
  
  // switch off PWM Power
  motorPowerOff();

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
  power++;
  if (power >= 255) {
    power = 0;
  }
    

      // set pitch motor pwm
      MoveMotorPosSpeed(i, power); 
      
     

   

}


