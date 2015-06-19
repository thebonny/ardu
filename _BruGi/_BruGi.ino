
/*****************************************************************************************************************************
Brushless Gimbal Controller Software by Christian Winkler and Alois Hahn (C) 2013

Brushless Gimbal Controller Hardware and Software support 
by Ludwig Fäerber, Alexander Rehfeld and Martin Eckart

Special Contributions:
  Michael Schätzel

Project homepage: http://brushlessgimbal.de/
Discussions:
http://fpv-community.de/showthread.php?20795-Brushless-Gimbal-Controller-SOFTWARE
http://fpv-community.de/showthread.php?22617-Gimbal-Brushless-Controller-V3-0-50x50mm-by-Martinez
http://fpv-community.de/showthread.php?19252-Brushless-Gimbal-Controller

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
any later version. see <http://www.gnu.org/licenses/>

Anyhow, if you start to commercialize our work, please read on http://code.google.com/p/brushless-gimbal/ on how to contribute

// I2Cdev library collection - MPU6050 I2C device class
// Based on InvenSense MPU-6050 register map document rev. 2.0, 5/19/2011 (RM-MPU-6000A-00)
// 10/3/2011 by Jeff Rowberg <jeff@rowberg.net>
// Updates should (hopefully) always be available at https://github.com/jrowberg/i2cdevlib
*****************************************************************************************************************************/

/* Note: software has been tested with Arduino Version 1.5.4 */

#define VERSION_STATUS B // A = Alpha; B = Beta , N = Normal Release
#define VERSION "v50"
#define REVISION "r202"
#define VERSION_EEPROM 15 // change this number when eeprom data structure has changed


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

  // just for debugging
#ifdef STACKHEAPCHECK_ENABLE
  stackCheck();
  heapCheck();
#endif

  LEDPIN_PINMODE
  
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


/*****************************************************************

main loop execution time budget


exected each iteration (main tick)
  time(us)   function
  --------------------------------
   330        readGyros
   175        updateGyroAttitude
    21        updateACCAttitude
   372        getAttiduteAngles
    92        pid pitch
    92        pid roll
     6        motor update
    84        RC low pass
  --------------------------------
  1172        sum
  1250        measured, real, no RC

executed each 10th iteration (sub tick)
  time(us)   function
  --------------------------------
   330        readACC
   120        updateACC
   210        voltage compensation
    56        gimbal state
    26        RC roll
    26        RC pitch
   286        evaluate RC
  --------------------------------
   330        maximum of all
   --        measured, real, no RC
  --------------------------------


total
  time(us)    function
  --------------------------------
   1172       main tick execution time
    330       sub tick maximum
  --------------------------------
   1502       sum 
  ================================


motor ISR duration
  version r161
  10 us every 31 us
  18 us every 2000 us
  
  --> improved
   6.3 us every 31 us
   
RC interrupt duration
   43 us, every 1000 ms (one RC channel)
   54 us, every 1000 ms, worst case, two RC channel

*******************************************************************/


/**********************************************/
/* Main Loop                                  */
/**********************************************/
void loop() 
{ 

  static char pOutCnt = 0;
  static char tOutCnt = 0;
  static char tOutCntSub = 0;
  static int stateCount = 0;
  static uint8_t ledBlinkCnt = 0;
  static uint8_t ledBlinkOnTime = 10;
  static uint8_t ledBlinkPeriod = 20;

  if (motorUpdate) // loop runs with motor ISR update rate (500 Hz)
  {
   
    motorUpdate = false;

    CH2_ON
    
   

  
   

    

 
  
    
    // motor update t=6us (*)
    if (enableMotorUpdates)
    {
      // set pitch motor pwm
      MoveMotorPosSpeed(config.motorNumberPitch, pitchMotorDrive, maxPWMmotorPitchScaled); 
     
    }

  

   
           
 
      
    

  
  

  
        



   

    CH2_OFF
  }

}


