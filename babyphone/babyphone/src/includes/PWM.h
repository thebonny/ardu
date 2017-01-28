/*
 * PWM.h
 *
 * Created: 19.01.2017 22:39:34
 *  Author: tmueller
 */ 


#ifndef PWM_H_
#define PWM_H_

void compute_space_vector_PWM(space_vector *sv);
void update_pwm_duty_cycles(space_vector *sv_motor_X, space_vector *sv_motor_Y);

void INIT_PWM(void);

#endif /* PWM_H_ */