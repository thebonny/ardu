/*
 * ppm_capture.h
 *
 * Created: 06.01.2017 19:15:25
 *  Author: tmueller
 */ 


#ifndef PPM_CAPTURE_H_
#define PPM_CAPTURE_H_
#define PIN_TC_CAPTURE     PIO_PC25_IDX  // PIN TIOA6 which is digital PIN 5 on Due

#include "stdint.h"
typedef struct {
	uint16_t current_captured_ppm_value;
	uint16_t last_captured_ppm_value;
} rc_channel;

void ppm_capture_initialize(void);
rc_channel get_captured_raw_channel(int channel_id);

#endif /* PPM_CAPTURE_H_ */ 