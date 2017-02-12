/*
 * ppm_out.h
 *
 * Created: 09.01.2017 13:34:55
 *  Author: tmueller
 */ 


#ifndef PPM_OUT_H_
#define PPM_OUT_H_
#include "stdint.h"

#define PIN_PPM_OUT     PIN_TC0_TIOA0  // PIN TIOA1 which is PIN PA2 resp. "A7" on Due
void ppm_out_initialize(void);
void set_ppm_out_channel_value(int idx, uint16_t value);

#endif /* PPM_OUT_H_ */