/*
 * aggregat.h
 *
 * Created: 13.01.2017 12:46:06
 *  Author: AndreFrank2
 */ 


#ifndef AGGREGAT_H_
#define AGGREGAT_H_



void aggregat_init(void);
void setSetpointVertical(float setpoint);
void PID1_1(void);
void PID2_1(void);
void PID1_2(void);
void PID2_2(void);
void SVPWM(float uum1, float uvm1, float uwm1, float uum2, float uvm2, float uwm2);


#endif /* AGGREGAT_H_ */