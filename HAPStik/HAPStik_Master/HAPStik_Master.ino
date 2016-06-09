#include <Wire.h>  // Comes with Arduino IDE
// Get the LCD I2C Library here: 
// https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads
// Move any other LCD libraries to another folder or delete them
// See Library "Docs" folder for possible commands etc.
#include <LiquidCrystal_I2C.h>



LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

void setup(){
 Serial1.begin(38400);     // Serial to HC-05 BlueTooth module in "AT" mode always @38k4
 delay(1000);
 lcd.begin(20,4);         // initialize the lcd for 20 chars 4 lines
 lcd.backlight();
 lcd.print("TEST");

 delay(3000);
}

void loop(){
 int sensorValue = analogRead(A0);
 

 Serial1.print(sensorValue);
 lcd.clear();
 lcd.print("Stick Position: ");
 lcd.print(sensorValue);
  delay(20);

}
