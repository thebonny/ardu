#include <conf_board.h>
#include <conf_clock.h>
#include <includes/ppm_capture.h>
#include "includes/utils.h"
#include "includes/registers.h"
#include "conf_hapstik.h"

#define PPM_OFFSET 400


volatile rc_channel rc_channels[NUMBER_OF_RC_CHANNELS];
static uint32_t gs_ul_captured_rb;

static int channel_id = 0;

void TC6_Handler(void)
{
	// debug_pulse(1);
	if ((TC2_CHANNEL0_SR & TC_SR_LDRBS) == TC_SR_LDRBS) {
		uint16_t micros = 0;
		gs_ul_captured_rb = TC2_CHANNEL0_RB;
		micros = (gs_ul_captured_rb * 8) / 21; // clock is Master Clock with prescaler 32 at 84 MHz Master Clock
		if (micros > 3000) {
			// PPM sync pulse, recount channels starting with 0
			channel_id = 0;
		//	printf("Current: %d\r\n", rc_channels[1].current_captured_ppm_value);
		//	printf("Last: %d\r\n", rc_channels[1].last_captured_ppm_value);
			return;
		}
		rc_channels[channel_id].last_captured_ppm_value = rc_channels[channel_id].current_captured_ppm_value;
		rc_channels[channel_id].current_captured_ppm_value = micros - PPM_OFFSET;

		channel_id++;
	}
}

int get_interpolated_channel_ppm(int channel_id) {
	
	return rc_channels[channel_id].last_captured_ppm_value;
}



void ppm_capture_initialize(void)
{
	
	for (int i = 0; i < NUMBER_OF_RC_CHANNELS; i++)
	{
		rc_channels[i].current_captured_ppm_value = MID_PWM_MICROS;
		rc_channels[i].last_captured_ppm_value = MID_PWM_MICROS;
	}
	
	REG_PMC_PCER1 = REG_PMC_PCER1 | 0x00000002u;

	TC2_CHANNEL0_CCR = TC2_CHANNEL0_CCR | 0x00000002u;
	TC2_CHANNEL0_CMR = TC_CMR_TCCLKS_TIMER_CLOCK3 | TC_CMR_LDRA_FALLING | TC_CMR_LDRB_RISING 	| TC_CMR_ABETRG | TC_CMR_ETRGEDG_RISING;
	
	DisableIRQ(33);
	ClearPendingIRQ(33);
	
	SetPriorityIRQ(33, 15);
	EnableIRQ(33);
	
	// enable interrupt
	TC2_CHANNEL0_IER = TC2_CHANNEL0_IER | TC_IER_LDRBS;
	// start tc2 channel 0
    TC2_CHANNEL0_CCR = TC2_CHANNEL0_CCR | 0x00000005u;
	
	
}

