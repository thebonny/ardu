/*
 * PID.c
 *
 * Created: 19.01.2017 22:44:04
 *  Author: tmueller
 */ 
#include "stdio.h"
#include "PID.h"
#include "ADC.h"
#include "PWM.h"
#include "record_playback.h"
#include "ppm_capture.h"
#include "utils.h"
#include "registers.h"
#include "math.h"		
#include "config/conf_hapstik.h"			

#define	PI		3.141592654f
#define	WK1		(PI/180)
#define	WK2		(PI*2/3)
#define	WK3		(PI*4/3)

#define UPDATE_CONTROLLER_MILLIS 1  // recalculate all controllers within defined frequency 1/x
#define TICKS_PER_MILLISECOND 42000
#define PID_INTERRUPT_PRIORITY 3


#define	ANGLE_OFFSET_X 23.0		 // HAPStik spezifischer Offset für Mittenposition in Winkelgraden
#define	ANGLE_OFFSET_Y 30.0

#define MAX_NORM_STEPS_THROW 1500
#define ANGLE_DEGREES_PER_NORM_STEPS ((float)(MAX_THROW_DEGREES)/(float)(MAX_NORM_STEPS_THROW))

#define ELECTRICAL_MECHANICAL_GEAR_FACTOR 7  // dies ist vom Motortyp (#Magnete etc.) abhängig

#define CENTER_ROTATION_ANGLE_X (ELECTRICAL_MECHANICAL_GEAR_FACTOR * ANGLE_OFFSET_X)
#define CENTER_ROTATION_ANGLE_Y (ELECTRICAL_MECHANICAL_GEAR_FACTOR * ANGLE_OFFSET_Y)


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
	
space_vector motor_X_space_vector = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 'X', CENTER_ROTATION_ANGLE_X};
space_vector motor_Y_space_vector = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 'Y', CENTER_ROTATION_ANGLE_Y };		
space_vector null_space_vector = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 'N', 0.0 };

int volatile cnt_1ms_poll = 0;

typedef struct  {
	uint16_t current_value;
	uint16_t past_value;
	uint8_t li_step;
} interpolated_rc_channel;

interpolated_rc_channel current_raw_channels[NUMBER_OF_RC_CHANNELS];

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
	delay_ms(200);
	//	MOTOR_Y
	//	---------------------------------------------------------------------------------------------------- Stick in Position +Y
	rotate_motor_degrees_from_start_angle(&motor_Y_space_vector, CENTER_ROTATION_ANGLE_Y, MAX_THROW_DEGREES, POSITIVE_DIRECTION);
	delay_ms(200);								// Stick beruhigen lassen
	y_max_throw_plus_adc = REG_ADC_CDR7;					
	//	----------------------------------------------------------------------------------------------------- Stick in Position -Y
	//	HOCH
	//	Schleifendurchläufe 66 (von +33° nach -33°)
	rotate_motor_degrees_from_start_angle(&motor_Y_space_vector, CENTER_ROTATION_ANGLE_Y + MAX_THROW_DEGREES * ELECTRICAL_MECHANICAL_GEAR_FACTOR , (2 * MAX_THROW_DEGREES), NEGATIVE_DIRECTION);
	delay_ms(200);								// Stick beruhigen lassen
	y_max_throw_minus_adc = REG_ADC_CDR7;

	rotate_motor_degrees_from_start_angle(&motor_Y_space_vector, CENTER_ROTATION_ANGLE_Y - MAX_THROW_DEGREES * ELECTRICAL_MECHANICAL_GEAR_FACTOR , MAX_THROW_DEGREES, POSITIVE_DIRECTION);
	delay_ms(200);								// Stick beruhigen lassen
	y_null_adc = REG_ADC_CDR7;

	//	MOTOR_X
	//	---------------------------------------------------------------------------------------------------- Stick in Position +Y
	rotate_motor_degrees_from_start_angle(&motor_X_space_vector, CENTER_ROTATION_ANGLE_X, MAX_THROW_DEGREES, POSITIVE_DIRECTION);
	delay_ms(200);								// Stick beruhigen lassen
	x_max_throw_plus_adc = REG_ADC_CDR6;
	//	----------------------------------------------------------------------------------------------------- Stick in Position -Y
	//	HOCH
	//	Schleifendurchläufe 66 (von +33° nach -33°)
	rotate_motor_degrees_from_start_angle(&motor_X_space_vector, CENTER_ROTATION_ANGLE_X + MAX_THROW_DEGREES * ELECTRICAL_MECHANICAL_GEAR_FACTOR , (2 * MAX_THROW_DEGREES), NEGATIVE_DIRECTION);
	delay_ms(200);								// Stick beruhigen lassen
	x_max_throw_minus_adc = REG_ADC_CDR6;

	rotate_motor_degrees_from_start_angle(&motor_X_space_vector, CENTER_ROTATION_ANGLE_X - MAX_THROW_DEGREES * ELECTRICAL_MECHANICAL_GEAR_FACTOR , MAX_THROW_DEGREES, POSITIVE_DIRECTION);
	delay_ms(200);								// Stick beruhigen lassen
	x_null_adc = REG_ADC_CDR6;
	

	char s1[32];
	char s2[32];
	char s3[32];

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
	x_minus_prop_factor	= (double)MAX_NORM_STEPS_THROW / x_max_throw_minus_adc;
	x_plus_prop_factor	= (double)MAX_NORM_STEPS_THROW / x_max_throw_plus_adc;

	y_minus_prop_factor	= (double)MAX_NORM_STEPS_THROW / y_max_throw_minus_adc;
	y_plus_prop_factor	= (double)MAX_NORM_STEPS_THROW / y_max_throw_plus_adc;
	
	printf("| propfaktor_mX : %15s| propfaktor_pX : %15s|\r\n",
	doubleToString(s1, x_minus_prop_factor), doubleToString(s2, x_plus_prop_factor));	

	printf("| propfaktor_mY : %15s| propfaktor_pY : %15s|\r\n\n",
	doubleToString(s1, y_minus_prop_factor), doubleToString(s2, y_plus_prop_factor));
	
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
	float rotation_angle = ELECTRICAL_MECHANICAL_GEAR_FACTOR * ((ANGLE_DEGREES_PER_NORM_STEPS * input) + ANGLE_OFFSET_X);
	if (power_factor >= 0) {
			rotation_angle += 90;
			compute_space_vector_components(sv, rotation_angle, power_factor);
	} else {
		rotation_angle -= 90;
		compute_space_vector_components(sv, rotation_angle, (-1) * power_factor);
	}	
}

void compute_space_vector_motor_Y(space_vector *sv, int input, float power_factor) {
	float rotation_angle = ELECTRICAL_MECHANICAL_GEAR_FACTOR * ((ANGLE_DEGREES_PER_NORM_STEPS * input) + ANGLE_OFFSET_Y);
	if (power_factor >= 0) {
		rotation_angle += 90;
		compute_space_vector_components(sv, rotation_angle, power_factor);
		} else {
		rotation_angle -= 90;
		compute_space_vector_components(sv, rotation_angle, (-1) * power_factor);
	}
}


int get_normalised_input_x(int raw) {
	int norm_input_x = raw - x_null_adc;
	if (norm_input_x >= 0)
	{	
		norm_input_x = norm_input_x * x_plus_prop_factor;
	} 
	else
	{	
		norm_input_x = (-1)	*	norm_input_x * x_minus_prop_factor;
	}	
	return norm_input_x;
}

int get_normalised_input_y(int raw) {
	int norm_input_y = raw - y_null_adc;
	if (norm_input_y >= 0)
	{	
		norm_input_y = norm_input_y * y_plus_prop_factor;
	} 
	else
	{	
		norm_input_y = (-1)	*	norm_input_y * y_minus_prop_factor;
	}	
	return norm_input_y;
}	

	



void compute_all_controllers(void) {
	ADC_inputs raw_inputs = get_oversampled_adc_inputs();
	motor_X_position_controller.input = get_normalised_input_x(raw_inputs.X);
	compute_space_vector_motor_X(&motor_X_space_vector, motor_X_position_controller.input,  pid_compute(&motor_X_position_controller));
	motor_Y_position_controller.input = get_normalised_input_y(raw_inputs.Y);
	compute_space_vector_motor_Y(&motor_Y_space_vector, motor_Y_position_controller.input,  pid_compute(&motor_Y_position_controller));
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

int normalise_channel(uint16_t raw_channel_value) {
	return (raw_channel_value - 1100) * 3;
}

float map_prop_channel_to_zero_one_float(uint16_t raw_channel_value) {
	return (raw_channel_value - 600) * 0.001f;
}

void set_stick_raw_channel(int channel_id, rc_channel *captured_channel) {
	current_raw_channels[channel_id].current_value = captured_channel->current_captured_ppm_value;
	current_raw_channels[channel_id].past_value = captured_channel->last_captured_ppm_value;
	current_raw_channels[channel_id].li_step = 0;
}

int get_linear_interpolated_raw_channel_value(int channel_id) {
	int delta = (current_raw_channels[channel_id].current_value - current_raw_channels[channel_id].past_value) / 20;
	int li_value = current_raw_channels[channel_id].past_value + ( current_raw_channels[channel_id].li_step * delta);
	current_raw_channels[channel_id].li_step++;
	return li_value;
}

void TC1_Handler(void) {
	if ((TC0_CHANNEL1_SR & TC_SR_CPCS) == TC_SR_CPCS) {
			debug_pulse(1);
			cnt_1ms_poll++;
		
			motor_Y_position_controller.setpoint = normalise_channel(get_linear_interpolated_raw_channel_value(3));
			motor_X_position_controller.setpoint = normalise_channel(get_linear_interpolated_raw_channel_value(4));
			
		
			motor_X_position_controller.outMax = map_prop_channel_to_zero_one_float(get_linear_interpolated_raw_channel_value(6));
			motor_Y_position_controller.outMax = map_prop_channel_to_zero_one_float(get_linear_interpolated_raw_channel_value(7));
			motor_X_position_controller.outMin = (-1) * map_prop_channel_to_zero_one_float(get_linear_interpolated_raw_channel_value(6));
			motor_Y_position_controller.outMin = (-1) * map_prop_channel_to_zero_one_float(get_linear_interpolated_raw_channel_value(7));
			
		/*		motor_X_position_controller.outMax = HALF_POWER;
				motor_Y_position_controller.outMax = HALF_POWER;
				motor_X_position_controller.outMin = (-1) * HALF_POWER;
				motor_Y_position_controller.outMin = (-1) * HALF_POWER;
			motor_Y_position_controller.kp = 0.006f * map_prop_channel_to_zero_one_float(current_raw_channel_values[6]);*/
					
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
	
	for (int i = 0; i < NUMBER_OF_RC_CHANNELS; i++)
	{
		current_raw_channels[i].current_value = MID_PWM_MICROS;
		current_raw_channels[i].past_value = MID_PWM_MICROS;
		current_raw_channels[i].li_step = 0;
	}
	
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

