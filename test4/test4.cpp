#include "arduino.h"
#include <ppm_capture.h>
#include <ppm_out.h>
#include <record_playback.h>
#include <Wire.h>  // Comes with Arduino IDE
#include <LiquidCrystal_I2C.h>
#include "conf_hapstik.h"
#include "stdlib.h"
#include "string.h"

#include <SdFat.h>
SdFat sd;
SdFile myFile;


LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address



PPMReceiver r;
int counter = 0;
char result[100];
void setup()   /*----( SETUP: RUNS ONCE )----*/
{
   Serial.begin(9600);
	ppm_out_initialize();
	record_playback_initialize();
  Serial.print("Initializing SD card...");

  if (!sd.begin(SS, SPI_EIGHTH_SPEED)) sd.initErrorHalt();

  r.init();
  lcd.begin(20,4);         // initialize the lcd for 20 chars 4 lines, turn on backlight

// ------- Quick 3 blinks of backlight  -------------
  for(int i = 0; i< 3; i++)
  {
    lcd.backlight();
    delay(250);
    lcd.noBacklight();
    delay(250);
  }
  lcd.backlight(); // finish with backlight on  

//-------- Write characters on the display ------------------
  // NOTE: Cursor Position: Lines and Characters start at 0  
  lcd.setCursor(3,1); //Start at character 4 on line 0
  lcd.print("HAPStik v0.1");
  delay(2500);
  lcd.clear();
  lcd.setCursor(1,1);
  
  	  // open the file for write at end like the Native SD library
	 if (!myFile.open("hapstik.txt", O_RDWR | O_CREAT | O_AT_END)) {
		sd.errorHalt("opening test.txt for write failed");
	 }

}

void loop()
{
	
	char out[10];
	

	for (int i = 0; i < NUMBER_OF_RC_CHANNELS; i++) {
		sprintf(out, "%4i,%4i;", r.get_channel(i).current_captured_ppm_value, r.get_channel(i).last_captured_ppm_value);
		strcat(result, out);
		if (counter % 10 == 0) {
			lcd.setCursor(0, i);
			lcd.print(out);
		}
		
	}
	if (counter < 1000) {
		 myFile.println(result);
		 strcpy(result, "");
	} else {
		myFile.close();
		lcd.clear();
		lcd.setCursor(0,0);
		lcd.print("Ready!");
		while (1);
	}
	counter++;
	
}


