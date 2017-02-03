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


#define	ANGLE_OFFSET_X 23.0		 // HAPStik spezifischer Offset für Mittenposition in Winkelgraden
#define	ANGLE_OFFSET_Y 30.0

#define ELECTRICAL_MECHANICAL_GEAR_FACTOR 7  // dies ist vom Motortyp (#Magnete etc.) abhängig
#define MAX_STEPS 1500
#define CENTER_ROTATION_ANGLE_X (ELECTRICAL_MECHANICAL_GEAR_FACTOR * ANGLE_OFFSET_X)
#define CENTER_ROTATION_ANGLE_Y (ELECTRICAL_MECHANICAL_GEAR_FACTOR * ANGLE_OFFSET_Y)

const float ZERO_POWER    = 0.0;
const float QUARTER_POWER = 0.25;
const float HALF_POWER    = 0.5;
const float FULL_POWER    = 1.0;

const int MAX_THROW_DEGREES = 33;

const int POSITIVE_DIRECTION = 1;
const int NEGATIVE_DIRECTION = -1;


int x_max_throw_plus_adc = 0;
int x_max_throw_minus_adc = 0;
int y_max_throw_plus_adc = 0;
int y_max_throw_minus_adc = 0;
int x_null_adc = 0;
int y_null_adc = 0;

float x_minus_prop_factor = 0.0;
float x_plus_prop_factor = 0.0;
float y_minus_prop_factor = 0.0;
float y_plus_prop_factor = 0.0;

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
	
space_vector motor_X_space_vector = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 'X', CENTER_ROTATION_ANGLE_X};
space_vector motor_Y_space_vector = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 'Y', CENTER_ROTATION_ANGLE_Y };		
space_vector null_space_vector = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 'N', 0.0 };

int volatile cnt_1ms_poll = 0;



void compute_space_vector_components(space_vector *sv, float rotation_angle, float power_factor) {
	sv->X = power_factor * cos(rotation_angle * WK1);
	sv->Y = power_factor * cos(rotation_angle * WK1-WK2);
	sv->Z = power_factor * cos(rotation_angle * WK1-WK3);
	compute_space_vector_PWM(sv);
}

void rotate_motor_degrees_from_start_angle(space_vector *sv, float start_angle, int no_of_degrees, int direction_sign) {
	float power_factor = HALF_POWER;
	for (int step =1 ; step <=no_of_degrees; step++) {
		if (step == no_of_degrees)
		{
			// wenn Stick in Zielposition dann max. Power zum "Festhalten"
			power_factor = FULL_POWER;
		}
		float rot_angle = start_angle + (ELECTRICAL_MECHANICAL_GEAR_FACTOR * step * direction_sign); // Winkel pro Step = 1°
		compute_space_vector_components(sv, rot_angle, power_factor);
		update_pwm_duty_cycles(sv);
		delay_ms(2);
	}
}

void adjust_neutral_position(void) {

	compute_space_vector_components(&motor_X_space_vector, motor_X_space_vector.center_angle, QUARTER_POWER);
	compute_space_vector_components(&motor_Y_space_vector, motor_Y_space_vector.center_angle, QUARTER_POWER);
	update_pwm_duty_cycles(&motor_X_space_vector);
	update_pwm_duty_cycles(&motor_Y_space_vector);
}

void calibration_sequence(void) {
	adjust_neutral_position();
	//	Stick beruhigen lassen
	delay_ms(500);
	//	MOTOR_Y
	//	---------------------------------------------------------------------------------------------------- Stick in Position +Y
	rotate_motor_degrees_from_start_angle(&motor_Y_space_vector, CENTER_ROTATION_ANGLE_Y, MAX_THROW_DEGREES, POSITIVE_DIRECTION);
	delay_ms(500);								// Stick beruhigen lassen
	y_max_throw_plus_adc = get_oversampled_adc_inputs().Y;					
	//	----------------------------------------------------------------------------------------------------- Stick in Position -Y
	//	HOCH
	//	Schleifendurchläufe 66 (von +33° nach -33°)
	rotate_motor_degrees_from_start_angle(&motor_Y_space_vector, CENTER_ROTATION_ANGLE_Y + MAX_THROW_DEGREES * ELECTRICAL_MECHANICAL_GEAR_FACTOR , (2 * MAX_THROW_DEGREES), NEGATIVE_DIRECTION);
	delay_ms(500);								// Stick beruhigen lassen
	y_max_throw_minus_adc = get_oversampled_adc_inputs().Y;	

	rotate_motor_degrees_from_start_angle(&motor_Y_space_vector, CENTER_ROTATION_ANGLE_Y - MAX_THROW_DEGREES * ELECTRICAL_MECHANICAL_GEAR_FACTOR , MAX_THROW_DEGREES, POSITIVE_DIRECTION);
	delay_ms(500);								// Stick beruhigen lassen
	y_null_adc = get_oversampled_adc_inputs().Y;

	//	MOTOR_X
	//	---------------------------------------------------------------------------------------------------- Stick in Position +Y
	rotate_motor_degrees_from_start_angle(&motor_X_space_vector, CENTER_ROTATION_ANGLE_X, MAX_THROW_DEGREES, POSITIVE_DIRECTION);
	delay_ms(500);								// Stick beruhigen lassen
	x_max_throw_plus_adc = get_oversampled_adc_inputs().X;
	//	----------------------------------------------------------------------------------------------------- Stick in Position -Y
	//	HOCH
	//	Schleifendurchläufe 66 (von +33° nach -33°)
	rotate_motor_degrees_from_start_angle(&motor_X_space_vector, CENTER_ROTATION_ANGLE_X + MAX_THROW_DEGREES * ELECTRICAL_MECHANICAL_GEAR_FACTOR , (2 * MAX_THROW_DEGREES), NEGATIVE_DIRECTION);
	delay_ms(500);								// Stick beruhigen lassen
	x_max_throw_minus_adc = get_oversampled_adc_inputs().X;

	rotate_motor_degrees_from_start_angle(&motor_X_space_vector, CENTER_ROTATION_ANGLE_X - MAX_THROW_DEGREES * ELECTRICAL_MECHANICAL_GEAR_FACTOR , MAX_THROW_DEGREES, POSITIVE_DIRECTION);
	delay_ms(500);								// Stick beruhigen lassen
	x_null_adc = get_oversampled_adc_inputs().X;
	

	char s1[32];
	char s2[32];
	char s3[32];
	char s4[32];
	char s5[32];
		//	PRINT
	printf("| minusX_33     : %15s| nullX         : %15s| plusX_33     : %15s|\r\n",
	doubleToString(s1, x_max_throw_minus_adc), doubleToString(s2, x_null_adc), doubleToString(s3, x_max_throw_plus_adc));
	
	printf("| minusY_33     : %15s| nullY         : %15s| plusY_33     : %15s|\r\n\n",
	doubleToString(s1, y_max_throw_minus_adc), doubleToString(s2, y_null_adc), doubleToString(s3, y_max_throw_plus_adc));

	update_pwm_duty_cycles(&null_space_vector);
	delay_ms(2);

	//	----- Kennlinie in Nullpunkt verschieben -> Nullpunkt wird 0
	x_max_throw_minus_adc = x_max_throw_minus_adc - x_null_adc;
	x_max_throw_plus_adc  = x_max_throw_plus_adc  - x_null_adc;

	y_max_throw_minus_adc = y_max_throw_minus_adc - y_null_adc;
	y_max_throw_plus_adc  = y_max_throw_plus_adc  - y_null_adc;

	//	PRINT
	printf("| minusX_33     : %15s| plusX_33      : %15s|\r\n",
	doubleToString(s1, x_max_throw_minus_adc), doubleToString(s3, x_max_throw_plus_adc));
	
	printf("| minusY_33     : %15s| plusY_33      : %15s|\r\n\n\n",
	doubleToString(s1, y_max_throw_minus_adc), doubleToString(s3, y_max_throw_plus_adc));
	
	/* Proportinalitätsfaktoren für "-" und "+"Kennlinienbereiche
	//	Proportionalitätsfaktor für +33° -> +1500 und -33° -> -1500
	*/
	x_minus_prop_factor	= (double)MAX_STEPS / x_max_throw_minus_adc;
	x_plus_prop_factor	= (double)MAX_STEPS / x_max_throw_plus_adc;

	y_minus_prop_factor	= (double)MAX_STEPS / y_max_throw_minus_adc;
	y_plus_prop_factor	= (double)MAX_STEPS / y_max_throw_plus_adc;
	
	//	PRINT
	printf("| propfaktor_mX : %.2f| propfaktor_pX : %.2f|\r\n", x_minus_prop_factor, x_plus_prop_factor);
	printf("| propfaktor_mY : %.2f| propfaktor_pY : %.2f|\r\n\n", y_minus_prop_factor, y_plus_prop_factor);
}




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
	update_pwm_duty_cycles(&motor_X_space_vector);
	update_pwm_duty_cycles(&motor_Y_space_vector);
	
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
		
		//	debug_pulse(1);
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

