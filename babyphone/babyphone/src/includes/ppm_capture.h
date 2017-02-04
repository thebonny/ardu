/*
 * ppm_capture.h
 *
 * Created: 06.01.2017 19:15:25
 *  Author: tmueller
 */ 


#ifndef PPM_CAPTURE_H_
#define PPM_CAPTURE_H_
#define PIN_TC_CAPTURE     PIO_PC25_IDX  // PIN TIOA6 which is digital PIN 5 on Due

typedef struct {
	int current_captured_ppm_value;
	int last_captured_ppm_value;
} rc_channel;

extern volatile rc_channel rc_channels[];

void ppm_capture_initialize(void);
int get_interpolated_channel_ppm(int channel_id);

#endif /* PPM_CAPTURE_H_ */ 