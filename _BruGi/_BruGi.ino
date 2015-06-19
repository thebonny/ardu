


/*************************/
/* Include Header Files  */
/*************************/

#include <Wire.h>

#include "definitions.h"



#include "PinChangeInt.h"
#include "Timer1.h"
#include "variables.h"
#include "BLcontroller.h"         // Motor Movement Functions and Timer Config



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

    CH2_ON

      // set pitch motor pwm
      MoveMotorPosSpeed(1, 1000, 1000); 

    CH2_OFF

}


