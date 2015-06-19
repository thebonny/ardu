// using a 16-bit timer in arduino is a bit more complex because arduinos standad setup stets them to 8 bit
// the higher the resolution is the slower is the timer .. (it counts always in the same speed .. so it takes 3* longer to count till 30 then it takes to count to10)
// because of this and the fact that MultiiWii dont uses 16-bit for the motor calculations we dont need to set them to thair full resolution
// we set it to 0 - 16383 with that value and the  phase and frequency correct mode we dont need a prescaler.
// to use the MWC's values of 1000-2000 we need to multiply the value by 8 .. (1000*8 = 8000, 2000*8 = 16000)  
// we do it also with bit shifting .. but in the other direction
  
int dutytime = (1500<<3); // this wloud give us a half throttle signal
// X<<1 = X*2, X<<2 = X*4 and X<<3 = X*8.... 
  
void setup(){
   pinMode(9, OUTPUT); // set pin 9 to output
     
   TCCR1A &= ~(1<<WGM10);   // phase correct mode.. to know how to set which register please see the datasheets of the MCU's
   TCNT1 = 0xF380;
   TCCR1B &= ~(1<<CS11); // no prescaler
   ICR1 |= 0xFFFF; // set timers to count to 16383 (hex 3FFF)
   TCCR1A |= _BV(COM1A1); // connect pin 9 to timer 1 channel A
    
   OCR1A = 0x0C7F;
}

void loop(){
  

}
