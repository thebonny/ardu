/*
 * ADC.h
 *
 * Created: 19.01.2017 22:42:33
 *  Author: tmueller
 */ 


#ifndef ADC_H_
#define ADC_H_

typedef struct {
	int X;
	int Y;
} ADC_inputs;

void adc_initialize(void);
ADC_inputs get_oversampled_adc_inputs(void);

#endif /* ADC_H_ */