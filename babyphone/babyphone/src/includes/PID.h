/*
 * PID.h
 *
 * Created: 19.01.2017 22:44:28
 *  Author: tmueller
 */ 


#ifndef PID_H_
#define PID_H_

typedef struct {
	float X;
	float Y;
	float Z;
	float PWM_u;
	float PWM_v;
	float PWM_w;
	char motor;
	float center_angle;
} space_vector;


void pid_initialize(void);
void calibration_sequence(void);
void adjust_neutral_position(void);
void rotate_motor_Y_degrees_from_start_angle(float start_angle, int no_of_degrees);

#endif /* PID_H_ */