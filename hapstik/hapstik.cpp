/*
 * GccLibrary1.cpp
 *
 * Created: 06.02.2017 22:54:51
 * Author : tmueller
 */ 

#include "hapstik.h"
#include "ADC.h"
#include "PWM.h"
#include "PID.h"



HAPStik::HAPStik() {
	pwm_initialize();
	adc_initialize();
	calibration_sequence();
	pid_initialize();
}

HAPStik::~HAPStik() {}

void HAPStik::setPositionX(int position) {
	
}

void HAPStik::setPositionY(int position) {
	
}