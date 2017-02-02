#include <ppm_out.h>
#include <utils.h>
#include <registers.h>

#define TICKS_PER_uS 42
#define MIN_PWM_MICROS 600
#define MAX_PWM_MICROS 1600
#define MID_PWM_MICROS 1100
#define THROW_PWM_MICROS 500
#define PPM_PULSE_MICROS 400
#define PPM_FRAME_LENGTH_TOTAL_MICROS 20000 // 20ms frame

volatile static uint32_t ppm_out_channels[NUMBER_OF_RC_CHANNELS];
volatile static unsigned int current_channel = 0;
volatile static uint32_t accumulated_frame_length = 0;


void TC0_Handler(void) {
	// debug_pulse(1);
	uint32_t ra, rc;
	if ((TC0_CHANNEL0_SR & TC_SR_CPCS) == TC_SR_CPCS) {
		if (current_channel >= NUMBER_OF_RC_CHANNELS) {
			// calculate sync pulse pause
			uint32_t sync_pulse_micros = PPM_FRAME_LENGTH_TOTAL_MICROS - accumulated_frame_length;
			ra = sync_pulse_micros * TICKS_PER_uS;
			accumulated_frame_length = 0;
			current_channel = -1;
		} else {
			// pulse pause for single channel
			ra = ppm_out_channels[current_channel] * TICKS_PER_uS;
			accumulated_frame_length = accumulated_frame_length + ppm_out_channels[current_channel];	
		}
		rc = ra + PPM_PULSE_MICROS * TICKS_PER_uS; // 300us Pulse
		accumulated_frame_length = accumulated_frame_length + PPM_PULSE_MICROS;
		TC0_CHANNEL0_RA = ra;
		TC0_CHANNEL0_RC = rc;
		current_channel++;	
	}
}

void set_ppm_out_channel_value(int idx, int value) {
	ppm_out_channels[idx] = value;
}

void ppm_out_initialize(void)
{
	uint32_t rc;
	
		// initialize rc channels using PWM min value
	for (int i = 0; i < NUMBER_OF_RC_CHANNELS; i++) {
		ppm_out_channels[i] = MIN_PWM_MICROS;
	}
	
		// enable clock for TC0 channel 0 (ID 27)
	PMC_PCER0 |= (1 << 27);

	REG_PIOB_PUDR = pin_to_mask(25);
	REG_PIOB_MDDR  = pin_to_mask(25);
	REG_PIOB_IFDR = pin_to_mask(25);
	REG_PIOB_SCIFSR = pin_to_mask(25);
	REG_PIOB_ABSR |= pin_to_mask(25);
	
	REG_PIOB_PDR |= (1 << 25);
	
	TC0_CHANNEL0_CCR |= 0x00000002u;
	TC0_CHANNEL0_CMR |= TC_CMR_TCCLKS_TIMER_CLOCK1	| TC_CMR_WAVE | TC_CMR_ACPA_CLEAR | TC_CMR_ACPC_SET | TC_CMR_CPCTRG;
	
	TC0_CHANNEL0_RC = TICKS_PER_uS * PPM_FRAME_LENGTH_TOTAL_MICROS;
	
	DisableIRQ(27);
	ClearPendingIRQ(27);
	SetPriorityIRQ(27, 0);
	EnableIRQ(27);

	// interrupt on rc compare	
	TC0_CHANNEL0_IER |= 0x00000010u;
	// start tc0 channel 0
    TC0_CHANNEL0_CCR |= 0x00000005u;
}



