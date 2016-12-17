#include <Wire.h>  // Comes with Arduino IDE
// Get the LCD I2C Library here: 
// https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads
// Move any other LCD libraries to another folder or delete them
// See Library "Docs" folder for possible commands etc.
#include <LiquidCrystal_I2C.h>

int timer1_counter;
int ShouldSend = 0;
static volatile int sensorValue;

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

void setup(){
 Serial.begin(38400);     // Serial to HC-05 BlueTooth module in "AT" mode always @38k4
 delay(1000);
 lcd.begin(20,4);         // initialize the lcd for 20 chars 4 lines
 lcd.backlight();
 lcd.print("TEST");
   noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;

  // Set timer1_counter to the correct value for our interrupt interval
  //timer1_counter = 64886;   // preload timer 65536-16MHz/256/100Hz
  timer1_counter = 63036;   // preload timer 65536-8MHz/64/45,454545Hz 22ms
  // timer1_counter = 34286;   // preload timer 65536-16MHz/256/2Hz
  
  
  TCNT1 = timer1_counter;   // preload timer
  TCCR1B |= (1 << CS11) | (1 << CS10) ;    // 256 prescaler 
  TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
  interrupts();             // enable all interrupts



}

ISR(TIMER1_OVF_vect)        // interrupt service routine 
{
  TCNT1 = timer1_counter;   // preload timer
  ShouldSend = 1;
}


void loop(){

// Serial1.write('\n');
 //lcd.clear();
 //lcd.print("Stick Position: ");
 //lcd.print(sensorValue);
 //delay(20);
  if (ShouldSend == 1) {
    sensorValue = analogRead(A0);
    Serial.println(sensorValue);
    ShouldSend = 0;
  }
}
