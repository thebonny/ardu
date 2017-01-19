/*
 * PID.h
 *
 * Created: 19.01.2017 22:44:28
 *  Author: tmueller
 */ 


#ifndef PID_H_
#define PID_H_

void INIT_PID(void);
void PID1_1(void);
void PID1_2(void);
void PID2_1(void);
void PID2_2(void);
void setSetpointVertical(float setpoint);

#endif /* PID_H_ */