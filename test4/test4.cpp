#include "arduino.h"
#include <ppm_capture.h>
#include <ppm_out.h>
#include "record_playback.h"
#include <Wire.h>  // Comes with Arduino IDE
#include <LiquidCrystal_I2C.h>
#include "conf_hapstik.h"
#include "stdlib.h"
#include "string.h"




LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address



PPMReceiver r;
int incomingByte = 0;


/**
 * \brief Display the user menu on the UART.
 */
static void display_menu(void)
{
    uint8_t i;
    puts("\n\rMenu :\n\r"
            "------\n\r"
            "  HAPStik Prototype Options:\r");

    printf("  -------------------------------------------\n\r"
            "  r: Record flight sequence\n\r"
            "  p: Playback recorded sequence \n\r"
            "  b: bypass recorder \n\r"
            "  d: double speed\n\r"
            "  h: half speed\n\r"
            "  m: Display menu \n\r"
            "------\n\r\r");
}



void setup()   /*----( SETUP: RUNS ONCE )----*/
{
   Serial.begin(9600);
	ppm_out_initialize();
	record_playback_initialize();

  r.init();
  lcd.begin(20,4);         // initialize the lcd for 20 chars 4 lines, turn on backlight

  lcd.backlight(); // finish with backlight on  

//-------- Write characters on the display ------------------
  // NOTE: Cursor Position: Lines and Characters start at 0  
  lcd.setCursor(3,1); //Start at character 4 on line 0
  lcd.print("HAPStik v0.1");
  delay(1000);
  lcd.clear();
  lcd.setCursor(1,1);
  display_menu();

}



void loop()
{
	   if (Serial.available() > 0) {

		 incomingByte = Serial.read();
        switch (incomingByte) {
        case 'm':
            display_menu();
            break;

        case 'r':
            puts("\n\rStart recording flight sequence!\r");
            start_record();

            break;

        case 'p':
            puts("\n\rStart playback of flight sequence!\r");
            start_playback();
            break;
            
              
        case 'b':
            puts("\n\rBypass captured PPM Signal directly to PPM out!\r");
            stop_record();
            break;
            
     

        default:
            puts("Not recognized key pressed \r");
            break;
        }
	   }


	
	
}


