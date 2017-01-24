/*
 * ADC.h
 *
 * Created: 19.01.2017 22:42:33
 *  Author: tmueller
 */ 


#ifndef ADC_H_
#define ADC_H_


void INIT_ADC(void);
int has_ADC_completed_20_conversions(void);
void reset_ADC(void);

#endif /* ADC_H_ */