/*
 * PID.h
 *
 * Created: 19.01.2017 22:44:28
 *  Author: tmueller
 */ 


#ifndef PID_H_
#define PID_H_

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

typedef struct {
	float X;
	float Y;
	float Z;
	float PWM_u;
	float PWM_v;
	float PWM_w;
} space_vector;

extern pid_controller motor_X_position_controller;
extern pid_controller motor_Y_position_controller;
extern pid_controller motor_X_speed_controller;
extern pid_controller motor_Y_speed_controller;

extern space_vector motor_X_space_vector;
extern space_vector motor_Y_space_vector;

double pid_compute(pid_controller *controller);
void compute_all_controllers(void);

#endif /* PID_H_ */