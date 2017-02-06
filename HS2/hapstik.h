/*
 * hapstik.h
 *
 * Created: 06.02.2017 23:23:42
 *  Author: tmueller
 */ 


#ifndef HAPSTIK_H_
#define HAPSTIK_H_

#include <Arduino.h>

class HAPStik {
	public:
		HAPStik();
		~HAPStik();
		void setPositionX(int position);
		void setPositionY(int position);
	};
	
	

typedef struct {
	float X;
	float Y;
	float Z;
	float PWM_u;
	float PWM_v;
	float PWM_w;
	char motor;
	float center_angle;
} space_vector;

typedef struct {
	int X;
	int Y;
} ADC_inputs;

typedef struct {
	uint16_t current_captured_ppm_value;
	uint16_t last_captured_ppm_value;
} rc_channel;

void compute_space_vector_PWM(space_vector *sv);
void update_pwm_duty_cycles(space_vector *sv_motor);
void pwm_initialize(void);
void set_stick_raw_channel(int channel_id, rc_channel *captured_channel);
void pid_initialize(void);
void calibration_sequence(void);
void adjust_neutral_position(void);
void rotate_motor_Y_degrees_from_start_angle(float start_angle, int no_of_degrees);
void adc_initialize(void);
ADC_inputs get_oversampled_adc_inputs(void);



#endif /* HAPSTIK_H_ */