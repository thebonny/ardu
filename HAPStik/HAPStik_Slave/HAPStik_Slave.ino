#include <Wire.h>  // Comes with Arduino IDE
// Get the LCD I2C Library here: 
// https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads
// Move any other LCD libraries to another folder or delete them
// See Library "Docs" folder for possible commands etc.
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

Servo myservo; 
String inString = "";    // string to hold input
boolean frameComplete = false;
int value = 0;

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

void setup(){
 

 lcd.begin(20,4);         // initialize the lcd for 20 chars 4 lines
 lcd.backlight();
  lcd.home();
 lcd.print("TEST");
 myservo.attach(9);
  // initialize digital pin 13 as an output.
  Serial1.begin(38400);     // Serial to HC-05 BlueTooth module in "AT" mode always @38k4
}

void serialEvent() {
   while (Serial1.available() > 0 && !frameComplete) {
    char inChar = (char) Serial1.read();
    if (isDigit(inChar)) {
      // convert the incoming byte to a char
      // and add it to the string:
      inString += inChar;
    }
    if (inChar == '\n') {
       frameComplete = true;
       value = map(inString.toInt(), 0, 1023, 0, 180);
       inString = "";
    }
  }
  
}

void loop(){
 // Read serial input:
  if (frameComplete) {
       lcd.clear();
       lcd.print("Stick Position: ");
       lcd.print(value);
     
       // clear the string for new input:
 
      myservo.write(value);
      frameComplete = false;
  }
 
 
}
