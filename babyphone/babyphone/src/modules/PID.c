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
#include "math.h"					//			z.B. cos(x)

#define	PI		3.141592654f
#define	WK1		(PI/180)
#define	WK2		(PI*2/3)
#define	WK3		(PI*4/3)

space_vector motor_X_space_vector = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
space_vector motor_Y_space_vector = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };	
		
pid_controller motor_X_position_controller = { 0.0, 0.0, 0.0, 0.0, 1.0, -1.0, 0.0015, 0.0, 0.0035 };
pid_controller motor_Y_position_controller = { 0.0, 0.0, 0.0, 0.0, 1.0, -1.0, 0.0015, 0.0, 0.0035 };
pid_controller motor_X_speed_controller = { 0.0, 0.0, 0.0, 0.0, 1.0, -1.0, 0.0005, 0.00125, 0.0 };
pid_controller motor_Y_speed_controller = { 0.0, 0.0, 0.0, 0.0, 1.0, -1.0, 0.0005, 0.00125, 0.0 };

const float	WKL_OFF_1 =	 5.5;							// Winkel_Offset für Poti_MOTOR1 Vertikal		(Nullposition)	-> "+" Stick wandert nach unten
const float	WKL_OFF_2 =	-16.0;

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


void compute_space_vector_motor_X(space_vector *sv, float power_factor) {
	float rotation_angle = 7 * ((33 * (get_average_adc_input_X() - 2418) / 1677.0) + WKL_OFF_1);
	if (power_factor >= 0) {
			rotation_angle += 90;
			compute_space_vector_components(sv, rotation_angle, power_factor);
	} else {
		rotation_angle -= 90;
		compute_space_vector_components(sv, rotation_angle, (-1) * power_factor);
	}	
}

void compute_space_vector_motor_Y(space_vector *sv, float power_factor) {
	float rotation_angle = 7 * ((33 * (get_average_adc_input_Y() - 2304) / 1641.0) + WKL_OFF_2);
	if (power_factor >= 0) {
		rotation_angle += 90;
		compute_space_vector_components(sv, rotation_angle, power_factor);
		} else {
		rotation_angle -= 90;
		compute_space_vector_components(sv, rotation_angle, (-1) * power_factor);
	}
}


void compute_all_controllers(void) {
	motor_X_position_controller.input = get_average_adc_input_X();								
	compute_space_vector_motor_X(&motor_X_space_vector, pid_compute(&motor_X_position_controller));

	motor_Y_position_controller.input = get_average_adc_input_Y();
	compute_space_vector_motor_Y(&motor_Y_space_vector, pid_compute(&motor_Y_position_controller));
	
	compute_space_vector_PWM(&motor_X_space_vector);
	compute_space_vector_PWM(&motor_Y_space_vector);
	
	update_pwm_duty_cycles(&motor_X_space_vector, &motor_Y_space_vector);
}
