/*
 * ppm_capture.h
 *
 * Created: 06.01.2017 19:15:25
 *  Author: tmueller
 */ 


#ifndef PPM_CAPTURE_H_
#define PPM_CAPTURE_H_
#define PIN_TC_CAPTURE     PIO_PC26_IDX  // PIN TIOA6 which is digital PIN 5 on Due



void ppm_capture_initialize(void);
int get_channel_value_as_PWM(int channel_idx);
int get_channel_value_as_PPM(int channel_idx);

#endif /* PPM_CAPTURE_H_ */ 