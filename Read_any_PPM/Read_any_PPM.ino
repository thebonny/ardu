#define PPM_Pin 3

#define DEBUG_PIN1 8
#define DEBUG_PIN2 10
int ppm[9];  //array for storing up to 16 servo signals
int pos = 0; 

int servo1 = 6;
int servo2 = 7;

int pwm;
int lastPPM = 0;

void setup()
{
  Serial1.begin(9600);


  pinMode(servo1, OUTPUT);
  pinMode(servo2, OUTPUT);
  
  pinMode(DEBUG_PIN1, OUTPUT);
  pinMode(DEBUG_PIN2, OUTPUT);
  
  pinMode(PPM_Pin, INPUT);
  attachInterrupt(0, read_ppm, CHANGE);
  
  TCCR3A = 0x00;	   
  TCCR3B = B00000010;    
  TIMSK3 = _BV(ICIE3);  
}

void read_ppm(){
  static unsigned int pulse;
  static unsigned long counter;
  static byte channel;
  
  digitalWrite(DEBUG_PIN1, digitalRead(PPM_Pin)); 

  counter = TCNT3;
  TCNT3 = 0;

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

void loop ()
{
   Serial.println(ppm[3]);
}



void servoPulse (int servo, int pwm)
{
 digitalWrite(servo, HIGH);
 delayMicroseconds(pwm);
 digitalWrite(servo, LOW);
                 
}
