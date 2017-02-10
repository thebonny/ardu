#include "hapstik.h"

void setup()
{
  Serial.begin(115200);
	HAPStik myStick;
	myStick.setPositionX(1200);
	myStick.setPositionY(-1200);
}

void loop()
{

}
