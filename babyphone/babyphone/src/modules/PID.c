/*
 * PID.c
 *
 * Created: 19.01.2017 22:44:04
 *  Author: tmueller
 */ 
#include "asf.h"
#include "stdio.h"
#include "includes/PID.h"
#include "includes/ADC.h"
#include "includes/PWM.h"
#include "includes/ppm_capture.h"
#include "includes/utils.h"
#include "math.h"		
#include "conf_debug.h"			

#define	PI		3.141592654f
#define	WK1		(PI/180)
#define	WK2		(PI*2/3)
#define	WK3		(PI*4/3)

// recalculcation of all controllers is scheduled by timer counter 0, channel 1
#define TC TC0
#define TC_CHANNEL 1  
#define ID_TC   ID_TC1

#define UPDATE_CONTROLLER_MILLIS 1  // recalculate all controllers within defined frequency 1/x
#define TICKS_PER_MILLISECOND 42000
#define PID_INTERRUPT_PRIORITY 5

#define ELECTRICAL_MECHANICAL_GEAR_FACTOR 7  // dies ist vom Motortyp (#Magnete etc.) abhängig
	
const float	ANGLE_OFFSET_X = 5.5;							// Winkel_Offset für Poti_MOTOR1 Vertikal		(Nullposition)	-> "+" Stick wandert nach unten
const float	ANGLE_OFFSET_Y = -16.0;

typedef struct  {
	float setpoint;
	float input;
	float lastInput;
	double ITerm;
	double outMax;
	double outMin;
	double kp;
	double ki;
	double kd;
} pid_controller;
		
pid_controller motor_X_position_controller = { 0.0, 0.0, 0.0, 0.0, 1.0, -1.0, 0.0015, 0.0, 0.0035 };
pid_controller motor_Y_position_controller = { 0.0, 0.0, 0.0, 0.0, 1.0, -1.0, 0.0015, 0.0, 0.0035 };
pid_controller motor_X_speed_controller = { 0.0, 0.0, 0.0, 0.0, 1.0, -1.0, 0.0005, 0.00125, 0.0 };
pid_controller motor_Y_speed_controller = { 0.0, 0.0, 0.0, 0.0, 1.0, -1.0, 0.0005, 0.00125, 0.0 };
	
space_vector motor_X_space_vector = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
space_vector motor_Y_space_vector = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };		



int volatile cnt_1ms_poll = 0;

	volatile	float	CH1_WERT1_1			= 0.0;						// Empfänger-Wert
	volatile	float	CH1_WERT1_1_alt		= 0.0;
	volatile	float	CH1_WERT1_1_li		= 0.0;						// linear interpoliert
	volatile	float	CH1_WERT1_1_li_nor	= 0.0;						// linear interpoliert und normiert
	volatile	float	CH1_DELTA			= 0.0;						// Delta-Wert für 1ms
	
	volatile	float	CH1_WERT2_1 = 0.0;
	volatile	int		CH1_WERT3_1 = 0;


	volatile	float	CH2_WERT1_1			= 0.0;						// Empfänger-Wert
	volatile	float	CH2_WERT1_1_alt		= 0.0;
	volatile	float	CH2_WERT1_1_li		= 0.0;						// linear interpoliert
	volatile	float	CH2_WERT1_1_li_nor	= 0.0;						// linear interpoliert und normiert
	volatile	float	CH2_DELTA			= 0.0;						// Delta-Wert für 1ms
	
	volatile	float	CH2_WERT2_1 = 0.0;
	volatile	int		CH2_WERT3_1 = 0;

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

void compute_space_vector_components(space_vector *sv, float rotation_angle, float power_factor) {
	sv->X = power_factor * cos(rotation_angle * WK1);
	sv->Y = power_factor * cos(rotation_angle * WK1-WK2);
	sv->Z = power_factor * cos(rotation_angle * WK1-WK3);
}


void compute_space_vector_motor_X(space_vector *sv, int input, float power_factor) {
	float rotation_angle = ELECTRICAL_MECHANICAL_GEAR_FACTOR * ((33 * (input - 2418) / 1677.0) + ANGLE_OFFSET_X);
	if (power_factor >= 0) {
			rotation_angle += 90;
			compute_space_vector_components(sv, rotation_angle, power_factor);
	} else {
		rotation_angle -= 90;
		compute_space_vector_components(sv, rotation_angle, (-1) * power_factor);
	}	
}

void compute_space_vector_motor_Y(space_vector *sv, int input, float power_factor) {
	float rotation_angle = ELECTRICAL_MECHANICAL_GEAR_FACTOR * ((33 * (input - 2304) / 1641.0) + ANGLE_OFFSET_Y);
	if (power_factor >= 0) {
		rotation_angle += 90;
		compute_space_vector_components(sv, rotation_angle, power_factor);
		} else {
		rotation_angle -= 90;
		compute_space_vector_components(sv, rotation_angle, (-1) * power_factor);
	}
}


void compute_all_controllers(void) {
	ADC_inputs inputs = get_oversampled_adc_inputs();
	motor_X_position_controller.input = inputs.X;
	compute_space_vector_motor_X(&motor_X_space_vector, inputs.X, pid_compute(&motor_X_position_controller));

	motor_Y_position_controller.input = inputs.Y;
	compute_space_vector_motor_Y(&motor_Y_space_vector, inputs.Y, pid_compute(&motor_Y_position_controller));
	
	compute_space_vector_PWM(&motor_X_space_vector);
	compute_space_vector_PWM(&motor_Y_space_vector);
	
	update_pwm_duty_cycles(&motor_X_space_vector, &motor_Y_space_vector);
}

static void display_debug_output() {
	#ifdef CONSOLE_DEBUG_LEVEL
		char s1[32], s2[32], s3[32];
		printf("| X1      : %15s| Y1      : %15s| Z1  : %15s\r\n",
		doubleToString(s1, motor_X_space_vector.X), doubleToString(s2, motor_X_space_vector.Y), doubleToString(s3, motor_X_space_vector.Z));
		printf("| Sollwert      : %15s\r\n",
		doubleToString(s1, motor_X_position_controller.setpoint));
		printf("\r\n");
		printf("| X2      : %15s| Y2      : %15s| Z2  : %15s\r\n",
		doubleToString(s1, motor_Y_space_vector.X), doubleToString(s2, motor_Y_space_vector.Y), doubleToString(s3, motor_Y_space_vector.Z));
		printf("| Sollwert      : %15s\r\n",
		doubleToString(s1, motor_Y_position_controller.setpoint));
		printf("------------\r\n");
	#endif
}



void TC1_Handler(void) {
	if ((tc_get_status(TC, TC_CHANNEL) & TC_SR_CPCS) == TC_SR_CPCS) {
		
		//	debug_pulse(1);
			cnt_1ms_poll++;
				// Lineare Interpolation, um 1ms Werte vom Master zu bekommen, der nur alle 20ms einen aktuellen Wert versendet
			//	alle 20ms
			if (cnt_1ms_poll % 20 == 0)																// 20ms
			{
				//	CH1 (TIOA7)
				CH1_WERT1_1_alt = CH1_WERT1_1;													// alten CH0-Wert retten
				CH1_WERT1_1 = get_captured_channel_value(1) * 2;														// alle 20ms neuen CH0-Wert übernehmen
				CH1_DELTA = (CH1_WERT1_1 - CH1_WERT1_1_alt)/20;

				CH1_WERT1_1_li = CH1_WERT1_1_li - CH1_DELTA;									// weil gleich danach in "jede ms" wieder CH0_DELTA dazu addiert wird

				//	CH2 (TIOA8)

				CH2_WERT1_1_alt = CH2_WERT1_1;													// alten CH0-Wert retten
				CH2_WERT1_1 = get_captured_channel_value(2) * 2;														// alle 20ms neuen CH0-Wert übernehmen
				CH2_DELTA = (CH2_WERT1_1 - CH2_WERT1_1_alt)/20;

				CH2_WERT1_1_li = CH2_WERT1_1_li - CH2_DELTA;									// weil gleich danach in "jede ms" wieder CH0_DELTA dazu addiert wird
			}
			
			CH1_WERT1_1_li = CH1_WERT1_1_li + CH1_DELTA;									// CH0-Wert ist alter CH0-Wert + Delta
			CH1_WERT1_1_li_nor = CH1_WERT1_1_li;			// Normierung auf Laufwege HS
			motor_Y_position_controller.setpoint = CH1_WERT1_1_li_nor;												//int wert Übergabe

			CH2_WERT1_1_li = CH2_WERT1_1_li + CH2_DELTA;									// CH0-Wert ist alter CH0-Wert + Delta
			CH2_WERT1_1_li_nor = CH2_WERT1_1_li;			// Normierung auf Laufwege HS
			motor_X_position_controller.setpoint = CH2_WERT1_1_li_nor;												//int wert Übergabe
			compute_all_controllers();
		
			if (cnt_1ms_poll % 200 == 0) {
				 display_debug_output();
			}
		
	}
}



void pid_initialize(void)
{
	uint32_t rc;
	// enable clock for timer
	sysclk_enable_peripheral_clock(ID_TC);
	tc_init(TC, TC_CHANNEL,
	TC_CMR_TCCLKS_TIMER_CLOCK1
	| TC_CMR_WAVE /* Waveform mode is enabled */
	| TC_CMR_ACPA_CLEAR /* RA Compare Effect: set */
	| TC_CMR_ACPC_SET /* RC Compare Effect: clear */
	| TC_CMR_CPCTRG  /* UP mode with automatic trigger on RC Compare */
	);

	tc_write_rc( TC, TC_CHANNEL, TICKS_PER_MILLISECOND * UPDATE_CONTROLLER_MILLIS);

	NVIC_DisableIRQ(TC1_IRQn);
	NVIC_ClearPendingIRQ(TC1_IRQn);
	NVIC_SetPriority(TC1_IRQn, PID_INTERRUPT_PRIORITY);
	NVIC_EnableIRQ(TC1_IRQn);
	tc_enable_interrupt(TC, TC_CHANNEL, TC_IER_CPCS);
	tc_start(TC, TC_CHANNEL);

}