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

extern pid_controller motor_X_position;
extern pid_controller motor_Y_position;
extern pid_controller motor_X_speed;
extern pid_controller motor_Y_speed;

double pid_compute(pid_controller *controller);

#endif /* PID_H_ */