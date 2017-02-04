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

#define ZERO_POWER     0.0
#define QUARTER_POWER 0.25
#define HALF_POWER    0.5
#define FULL_POWER    1.0

void set_stick_raw_channel_value(int channel_id, int raw_channel_value);
void pid_initialize(void);
void calibration_sequence(void);
void adjust_neutral_position(void);
void rotate_motor_Y_degrees_from_start_angle(float start_angle, int no_of_degrees);

#endif /* PID_H_ */