#define ROBOT_NAME "HAPStik-1"

// If you haven't configured your device before use this
#define BLUETOOTH_SPEED 230400 //This is the default baudrate that HC-05 uses

int potPin = 2;
int val = 0;
unsigned char high;
unsigned char low;

void setup() {
  Serial.begin(230400);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  Serial.println("Starting Sender MASTER setup");
  Serial1.begin(BLUETOOTH_SPEED);
  delay(1000);


  Serial.println("Done Sender MASTER setup!");
}

void loop() {
  val = analogRead(potPin);
  Serial.print(val, DEC);
  Serial.println();       
  high = (unsigned char)(val>>8);
  low  = val & 0xff;
  Serial1.write(low);
  Serial1.write(high);
  Serial1.write(',');
  
  Serial1.flush(); 
}
