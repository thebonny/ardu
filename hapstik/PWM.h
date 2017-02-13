/*
 * PWM.h
 *
 * Created: 19.01.2017 22:39:34
 *  Author: tmueller
 */ 


#ifndef PWM_H_
#define PWM_H_
#include "PID.h"
void compute_space_vector_PWM(space_vector *sv);
void update_pwm_duty_cycles(space_vector *sv_motor);

void pwm_initialize(void);

#endif /* PWM_H_ */