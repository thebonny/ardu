/*
 * PID.c
 *
 * Created: 19.01.2017 22:44:04
 *  Author: tmueller
 */ 
#include "stdio.h"
//	1.Regler MOTOR1
//	PID1_1 Regler GESCHWINDIGKEIT
float myInput1_1;

float error1_1 = 0.0, input1_1 = 0.0, dInput1_1 = 0.0, lastInput1_1 = 0.0, mySetpoint1_1 = 0.0;

double myOutput1_1;
double ITerm1_1;
double ki1_1 = 0.0;
double kp1_1 = 0.0;
double kd1_1 = 0.0;
double outMax1_1;
double outMin1_1;


//	2.Regler MOTOR1
//	PID2_1 Regler POSITION
int myInput2_1;

float error2_1 = 0.0, input2_1 = 0.0, dInput2_1 = 0.0, lastInput2_1 = 0.0, mySetpoint2_1 = 0.0;

double myOutput2_1;
double ITerm2_1;
double ki2_1 = 0.0;
double kp2_1 = 0.0;
double kd2_1 = 0.0;
double outMax2_1;
double outMin2_1;


//	1.Regler MOTOR2
//	PID1_2 Regler GESCHWINDIGKEIT
float myInput1_2;

float error1_2 = 0.0, input1_2 = 0.0, dInput1_2 = 0.0, lastInput1_2 = 0.0, mySetpoint1_2 = 0.0;

double myOutput1_2;
double ITerm1_2;
double ki1_2 = 0.0;
double kp1_2 = 0.0;
double kd1_2 = 0.0;
double outMax1_2;
double outMin1_2;

//	2.Regler MOTOR2
//	PID2_2 Regler POSITION
int myInput2_2;

float error2_2 = 0.0, input2_2 = 0.0, dInput2_2 = 0.0, lastInput2_2 = 0.0, mySetpoint2_2 = 0.0;

double myOutput2_2;
double ITerm2_2;
double ki2_2 = 0.0;
double kp2_2 = 0.0;
double kd2_2 = 0.0;
double outMax2_2;
double outMin2_2;


void INIT_PID(void) {
		//	--------------------------------------------------------------------------------------- Kalibrierung der Winkel zueinander
		//	Stick muss sich leicht drehen lassen
		//	------------------------------------------------------------------------------------------------ Poti auf Winkel einlernen
		//	Analogwerte an 3 Stickpositionen messen und speichern

		//	mySetpoint1_1	= 800 + 0.6 * SS_ADC_A2_i;			// Führungsgröße MOTOR1 durch Poti "Masterstick" festlegen
		//	mySetpoint1_1	=  2418;							// Führungsgröße manuell festlegen: 2418 -> 0° (mech.)
		outMax1_1		=	1.0;							// Max Geschwindigkeit =  1,0 -> ca. 20°/s  oder 0,2 -> ca. 4°/s
		outMin1_1		=  -1.0;							// Min
		kp1_1			=   0.0015;							// ?
		//	ki1_1			=   0.0;
		kd1_1			=   0.0035;

		//	kp1_1			=   0.0015;							// SUPER gedämpft
		//	ki1_1			=   0.0;
		//	kd1_1			=   0.004;

		//  REGLER GESCHWINDIGKEIT PID2_1
		//	mySetpoint2_1	=  -0.2;							// Wenn nur der Geschwindigkeitsregler arbeitet
		// 1.0 -> ca. 20°/s
		// 0.2 -> ca. 4°/s
		outMax2_1		=  1.0;								// Max Drehmoment MOTOR1
		outMin2_1		= -1.0;

		//	kp2_1			= 0.0005;							// HAMMER
		//	ki2_1			= 0.00125;
		//	kd2_1			= 0.0;

		//	REGLER POSITION PID1_2
		//	mySetpoint1_2	= 800 + 0.6 * SS_ADC_A2_i;			// Führungsgröße MOTOR1 durch Poti "Masterstick" festlegen
		//	mySetpoint1_2	=  2304;							// Führungsgröße manuell festlegen: 2304 -> 0° (mech.)
		outMax1_2		=	1.0;							// Max Geschwindigkeit =  1,0 -> ca. 20°/s  oder 0,2 -> ca. 4°/s
		outMin1_2		=  -1.0;							// Min

		kp1_2			=   0.0015;							// ?
		//	ki1_2			=   0.0;
		kd1_2			=   0.0035;
		//  REGLER GESCHWINDIGKEIT PID2_2
		//	mySetpoint2_2	=  -0.2;							// Wenn nur der Geschwindigkeitsregler arbeitet
		// 1.0 -> ca. 20°/s
		// 0.2 -> ca. 4°/s
		outMax2_2		=  1.0;								// Max Drehmoment MOTOR1
		outMin2_2		= -1.0;

		//	kp2_2			= 0.0005;							// HAMMER
		//	ki2_2			= 0.00125;
		//	kd2_2			= 0.0;
}


//	MOTOR1
//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    REGLER1_1   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void PID1_1(void)
{
	input1_1 = myInput1_1;
	error1_1 = mySetpoint1_1 - input1_1;
	ITerm1_1+= (ki1_1 * error1_1);
	if(ITerm1_1 > outMax1_1) ITerm1_1= outMax1_1;
	else if(ITerm1_1 < outMin1_1) ITerm1_1= outMin1_1;
	dInput1_1 = (input1_1 - lastInput1_1);
	
	// Reglerausgang berechnen
	double output1_1 = kp1_1 * error1_1 + ITerm1_1 - kd1_1 * dInput1_1;

	// Überläufe kappen
	if(output1_1 > outMax1_1) output1_1 = outMax1_1;
	else if(output1_1 < outMin1_1) output1_1 = outMin1_1;
	myOutput1_1 = output1_1;
	
	//	letzten Wert speichern
	lastInput1_1 = input1_1;
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    REGLER1_1   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx


//	MOTOR2
//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    REGLER1_2   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void PID1_2(void)
{
	input1_2 = myInput1_2;
	error1_2 = mySetpoint1_2 - input1_2;

	ITerm1_2+= (ki1_2 * error1_2);
	if(ITerm1_2 > outMax1_2) ITerm1_2= outMax1_2;
	else if(ITerm1_2 < outMin1_2) ITerm1_2= outMin1_2;
	dInput1_2 = (input1_2 - lastInput1_2);
	
	// Reglerausgang berechnen
	double output1_2 = kp1_2 * error1_2 + ITerm1_2 - kd1_2 * dInput1_2;

	// Überläufe kappen
	if(output1_2 > outMax1_2) output1_2 = outMax1_2;
	else if(output1_2 < outMin1_2) output1_2 = outMin1_2;
	myOutput1_2 = output1_2;
	
	//	letzten Wert speichern
	lastInput1_2 = input1_2;
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    REGLER1_2   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx


//	MOTOR1
//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    REGLER2_1   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void PID2_1(void)
{
	input2_1 = myInput2_1;
	error2_1 = mySetpoint2_1 - input2_1;
	ITerm2_1+= (ki2_1 * error2_1);
	if(ITerm2_1 > outMax2_1) ITerm2_1= outMax2_1;
	else if(ITerm2_1 < outMin2_1) ITerm2_1= outMin2_1;
	dInput2_1 = (input2_1 - lastInput2_1);
	
	// Reglerausgang berechnen
	double output2_1 = kp2_1 * error2_1 + ITerm2_1 - kd2_1 * dInput2_1;

	// Überläufe kappen
	if(output2_1 > outMax2_1) output2_1 = outMax2_1;
	else if(output2_1 < outMin2_1) output2_1 = outMin2_1;
	myOutput2_1 = output2_1;
	
	//	letzten Wert speichern
	lastInput2_1 = input2_1;
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    REGLER2_1   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx


//	MOTOR2
//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    REGLER2_2   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void PID2_2(void)
{
	input2_2 = myInput2_2;
	error2_2 = mySetpoint2_2 - input2_2;
	ITerm2_2+= (ki2_2 * error2_2);
	if(ITerm2_2 > outMax2_2) ITerm2_2= outMax2_2;
	else if(ITerm2_2 < outMin2_2) ITerm2_2= outMin2_2;
	dInput2_2 = (input2_2 - lastInput2_2);
	
	// Reglerausgang berechnen
	double output2_2 = kp2_2 * error2_2 + ITerm2_2 - kd2_2 * dInput2_2;

	// Überläufe kappen
	if(output2_2 > outMax2_2) output2_2 = outMax2_2;
	else if(output2_2 < outMin2_2) output2_2 = outMin2_2;
	myOutput2_2 = output2_2;
	
	//	letzten Wert speichern
	lastInput2_2 = input2_2;
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    REGLER2_2   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx


void setSetpointVertical(float setpoint) {
	mySetpoint1_1 = setpoint;

}

void setSetpointHorizontal(float setpoint) {
	mySetpoint1_2 = setpoint;

}
