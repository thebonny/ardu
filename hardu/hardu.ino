#include <utils.h>
#include <registers.h>
#include <record_playback.h>
#include <PWM.h>
#include <ppm_out.h>
#include <ppm_capture.h>
#include <PID.h>
#include <hapstik.h>
#include <cycle_counter.h>
#include <ADC.h>




/*
 * hardu.ino
 *
 * Created: 2/14/2017 4:50:27 PM
 * Author: tmueller
 */ 

void setup()
{

Serial.begin(115200);
	gpio_initialize();
	pwm_initialize();
	adc_initialize();
	pid_initialize();
	
	 
	 
	 

}

void loop()
{

	  /* add main program code here, this code starts again each time it ends */

}
