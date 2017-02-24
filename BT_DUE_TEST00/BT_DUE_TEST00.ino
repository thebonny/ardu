#include <Servo.h>

#define ROBOT_NAME "HAPStik-0"

// If you haven't configured your device before use this
#define BLUETOOTH_SPEED 230400 //This is the default baudrate that HC-05 uses

long counter = 0;
Servo myservo;
int val = 0;
int angle = 0;

void setup() {
  myservo.attach(5); 
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  Serial.println("Starting Receiver Slave setup");
  Serial1.begin(BLUETOOTH_SPEED);
  delay(1000);

  Serial.println("Done Receiver Slave setup!");
   
}

void loop() {
  while (Serial1.available()) {
    val = Serial1.parseInt();
    angle = map(val, 0, 1023, 0, 180);
      myservo.write(angle);
      Serial.print(val, DEC);
      Serial.println();
   }
}
