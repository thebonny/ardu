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
#include "includes/ppm_capture.h"
#include "includes/utils.h"
#include "includes/registers.h"
#include "math.h"		
#include "conf_debug.h"			

#define	PI		3.141592654f
#define	WK1		(PI/180)
#define	WK2		(PI*2/3)
#define	WK3		(PI*4/3)

#define UPDATE_CONTROLLER_MILLIS 1  // recalculate all controllers within defined frequency 1/x
#define TICKS_PER_MILLISECOND 42000
#define PID_INTERRUPT_PRIORITY 3

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
	if ((TC0_CHANNEL1_SR & TC_SR_CPCS) == TC_SR_CPCS) {
		
			debug_pulse(1);
			cnt_1ms_poll++;

			motor_Y_position_controller.setpoint = get_interpolated_channel_ppm(1, cnt_1ms_poll % 20);
			motor_X_position_controller.setpoint = get_interpolated_channel_ppm(2, cnt_1ms_poll % 20);
			
	//		printf("IP: %d\r\n", get_interpolated_channel_ppm(1, cnt_1ms_poll % 20));

			
			performance_trace_start(0);
			compute_all_controllers();
			performance_trace_stop(0);
		
			if (cnt_1ms_poll % 1000 == 0) {
				 display_debug_output();
			}
		
	}
}




void pid_initialize(void)
{
	uint32_t rc;
	// enable clock for timer
	PMC_PCER0 = PMC_PCER0 | 0x10000000u;
	
    TC0_CHANNEL1_CCR = TC0_CHANNEL1_CCR | 0x00000002u;
	TC0_CHANNEL1_CMR = TC0_CHANNEL1_CMR | TC_CMR_TCCLKS_TIMER_CLOCK1	| TC_CMR_WAVE | TC_CMR_ACPA_CLEAR | TC_CMR_ACPC_SET | TC_CMR_CPCTRG;
	
	TC0_CHANNEL1_RC = TICKS_PER_MILLISECOND * UPDATE_CONTROLLER_MILLIS;

	DisableIRQ(28);
	ClearPendingIRQ(28);
    SetPriorityIRQ(28, PID_INTERRUPT_PRIORITY);
	EnableIRQ(28);
	
	// interrupt on rc compare	
	TC0_CHANNEL1_IER = TC0_CHANNEL1_IER | 0x00000010u;
	// start tc0 channel 1
    TC0_CHANNEL1_CCR = TC0_CHANNEL1_CCR | 0x00000005u;
	
}

