/*
 * PID.c
 *
 * Created: 19.01.2017 22:44:04
 *  Author: tmueller
 */ 
#include "stdio.h"
#include "includes/PID.h"
#include "includes/ADC.h"
#include "includes/PWM.h"
#include "includes/utils.h"
#include "math.h"					//			z.B. cos(x)



#define	PI		3.141592654f
#define	WK1		(PI/180)
#define	WK2		(PI*2/3)
#define	WK3		(PI*4/3)
		
pid_controller motor_X_position = { 0.0, 0.0, 0.0, 0.0, 1.0, -1.0, 0.0015, 0.0, 0.0035 };
pid_controller motor_Y_position = { 0.0, 0.0, 0.0, 0.0, 1.0, -1.0, 0.0015, 0.0, 0.0035 };
pid_controller motor_X_speed = { 0.0, 0.0, 0.0, 0.0, 1.0, -1.0, 0.0005, 0.00125, 0.0 };
pid_controller motor_Y_speed = { 0.0, 0.0, 0.0, 0.0, 1.0, -1.0, 0.0005, 0.00125, 0.0 };

	//	Leistungsfaktor	0 ... 1 Leistung | 0.5 -> 50% Leistung | 1.0 -> 100% Leistung
	volatile	float	LF1	= 1.0;
	volatile	float	LF2	= 1.0;
	
	volatile	float	DWE1		= 0.0;
	volatile	float	DWE_a1		= 0.0;									// alter DWE-Wert

	volatile	float	DWE2		= 0.0;
	volatile	float	DWE_a2		= 0.0;
	
	volatile	float	WKL_OFF_1 =		5.5;							// Winkel_Offset für Poti_MOTOR1 Vertikal		(Nullposition)	-> "+" Stick wandert nach unten
	volatile	float	WKL_OFF_2 =		-16.0;
	
	//	für SVPWM()
	float	X1, Y1, Z1;
	float	X2, Y2, Z2;
	
		char s1[32];
		char s2[32];
		char s3[32];
	
double pid_compute(pid_controller *controller)
{
	float error = controller->setpoint - controller->input;
	controller->ITerm += (controller->ki * error);
	
	if(controller->ITerm > controller->outMax) {
		controller->ITerm = controller->outMax;
	} else if(controller->ITerm < controller->outMin) {
		controller->ITerm = controller->outMin;
	}
	float dInput = (controller->input - controller->lastInput);
	
	// Reglerausgang berechnen
	double output = controller->kp * error + controller->ITerm - controller->kd * dInput;

	// Überläufe kappen
	if(output > controller->outMax) {
		output = controller->outMax;
	} else if(output < controller->outMin) {
		output = controller->outMin;
	}
	//	letzten Wert speichern
	controller->lastInput = controller->input;
	return output;
}

void compute_all_controllers(void) {
				//	REGLER rechnen

//	REGLER POSITION
	motor_X_position.input = get_average_adc_input_X();								
	LF1 = pid_compute(&motor_X_position);							


//	Ausgabe an MOTOR1
//	Zum Verständnis:
//	Wenn der Leistungsfaktor	-> positiv, dann +90° (elektrisch) Vektor raus geben
//								-> negativ, dann -90° (elektrisch) Vektor raus geben
			if (LF1 >= 0)
			{	DWE1 = 7 * ((33 * (get_average_adc_input_X() - 2418) / 1677.0) + WKL_OFF_1) + 90;	// "-" bei [2]


//	Winkelanteile mit Berücksichtigung des Leistungsfaktors berechnen
				X1 = LF1 * cos(DWE1*WK1);
				Y1 = LF1 * cos(DWE1*WK1-WK2);
				Z1 = LF1 * cos(DWE1*WK1-WK3);
			}
			if (LF1 < 0)
			{
				DWE1 = 7 * ((33 * (get_average_adc_input_X() - 2418) / 1677.0) + WKL_OFF_1) - 90;	// "+" bei [2]						
//	Winkelanteile mit Berücksichtigung des Leistungsfaktors berechnen
				X1 = -LF1 * cos(DWE1*WK1);
				Y1 = -LF1 * cos(DWE1*WK1-WK2);
				Z1 = -LF1 * cos(DWE1*WK1-WK3);
			}

			motor_Y_position.input = get_average_adc_input_Y();
			LF2 = pid_compute(&motor_Y_position);
	

//	Ausgabe an MOTOR2
//	Zum Verständnis:
//	Wenn der Leistungsfaktor	-> positiv (Stick links),	dann -90° (elektrisch) Vektor raus geben
//								-> negativ (Stick rechts),	dann +90° (elektrisch) Vektor raus geben
			if (LF2 >= 0)
			{
				DWE2 = 7 * ((33 * (get_average_adc_input_Y() - 2304) / 1641.0) + WKL_OFF_2) + 90;	
//	Winkelanteile mit Berücksichtigung des Leistungsfaktors berechnen
				X2 = LF2 * cos(DWE2*WK1);
				Y2 = LF2 * cos(DWE2*WK1-WK2);
				Z2 = LF2 * cos(DWE2*WK1-WK3);
			}

			if (LF2 < 0)
			{
				DWE2 = 7 * ((33 * (get_average_adc_input_Y() - 2304) / 1641.0) + WKL_OFF_2) - 90;					

//	Winkelanteile mit Berücksichtigung des Leistungsfaktors berechnen
				X2 = -LF2 * cos(DWE2*WK1);
				Y2 = -LF2 * cos(DWE2*WK1-WK2);
				Z2 = -LF2 * cos(DWE2*WK1-WK3);
			}
	

		
//	Gemeinsame Raumvektorausgabe ---------------------------------------------------------------------------------------------	
		SVPWM(X1, Y1, Z1, X2, Y2, Z2);
		
				printf("| X1      : %15s| Y1      : %15s| Z1  : %15s\r\n",
				doubleToString(s1, X1), doubleToString(s2, Y1), doubleToString(s3, Z1));
				printf("| Sollwert      : %15s\r\n",
				doubleToString(s1, motor_X_position.setpoint));
				printf("\r\n");
				printf("| X2      : %15s| Y2      : %15s| Z2  : %15s\r\n",
				doubleToString(s1, X2), doubleToString(s2, Y2), doubleToString(s3, Z2));
				printf("| Sollwert      : %15s\r\n",
				doubleToString(s1, motor_Y_position.setpoint));
				printf("------------\r\n");
}
