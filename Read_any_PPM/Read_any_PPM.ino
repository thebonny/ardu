
/*This program puts the servo values into an array,
 reagrdless of channel number, polarity, ppm frame length, etc...
 You can even change these while scanning!*/
 

#define PPM_Pin 3

#define DEBUG_PIN1 8
#define DEBUG_PIN2 10
int ppm[6];  //array for storing up to 16 servo signals
int pos = 0; 

void setup()
{
 // Serial1.begin(9600);

 
  pinMode(DEBUG_PIN1, OUTPUT);
   pinMode(DEBUG_PIN2, OUTPUT);
  pinMode(PPM_Pin, INPUT);
  attachInterrupt(0, read_ppm, CHANGE);
  
  TCCR1A = 0x00;	   //  COM1A1=0, COM1A0=0 => Disconnect Pin OC1 from Timer/Counter 1 -- PWM11=0,PWM10=0 => PWM Operation disabled
  TCCR1B = B00000010;     //0x02;	   // 16MHz clock with prescaler means TCNT1 increments every .5 uS (cs11 bit set
  TIMSK1 = _BV(ICIE1);   // enable input capture interrupt for timer 1
}

void read_ppm(){
  static unsigned int pulse;
  static unsigned long counter;
  static byte channel;
  
   digitalWrite(DEBUG_PIN1, digitalRead(PPM_Pin)); 

  counter = TCNT1;
  TCNT1 = 0;

  if(counter < 1000){  //must be a pulse
    pulse = counter;
    digitalWrite(DEBUG_PIN2, HIGH);
  }
  else if(counter > 3820){  //sync
    channel = 0;
    digitalWrite(DEBUG_PIN2, LOW);
  }
  else{  //servo values between 810 and 2210 will end up here
    ppm[channel] = (counter + pulse)/2;
    channel++;
    digitalWrite(DEBUG_PIN2, LOW);
  }
}

void loop()
{
 
   // myservo.write(random(45, 135));              // tell servo to go to position in variable 'pos' 
   
   delay (100);
}
