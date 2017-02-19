#include "arduino.h"
#include <ppm_capture.h>
#include <Wire.h>  // Comes with Arduino IDE
#include <LiquidCrystal_I2C.h>

#include <SdFat.h>
SdFat sd;
SdFile myFile;



/*-----( Declare Constants )-----*/
/*-----( Declare objects )-----*/
// set the LCD address to 0x27 for a 20 chars 4 line display
// Set the pins on the I2C chip used for LCD connections:
//                    addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address


/*-----( Declare Variables )-----*/

PPMReceiver r;
void setup()   /*----( SETUP: RUNS ONCE )----*/
{
   Serial.begin(9600);

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
	

}/*--(end setup )---*/

void loop()
{
	
	char out[19];
		  // open the file for write at end like the Native SD library
	 if (!myFile.open("test.txt", O_RDWR | O_CREAT | O_AT_END)) {
		sd.errorHalt("opening test.txt for write failed");
	 }
	for (int i = 0; i < 4; i++) {
		sprintf(out, "Channel %i: %i       ", (i+1), r.get_channel(i+1).current_captured_ppm_value);
		lcd.setCursor(0, i);
		lcd.print(out);
		myFile.println(out);
	}
	myFile.close();
	
}


