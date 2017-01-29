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
} space_vector;

void pid_initialize(void);

#endif /* PID_H_ */