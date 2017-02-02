 #include <asf.h>
#include <includes/ppm_out.h>
#include "includes/utils.h"
#include "includes/registers.h"



/** Use TC Peripheral 0 **/
#define TC  TC0

/** Configure TC0 channel 1 as waveform output. **/
#define TC_CHANNEL_WAVEFORM 0
#define ID_TC_WAVEFORM      ID_TC0

#define PIN_TC_WAVEFORM_MUX PIN_TC0_TIOA0_MUX

/** Use TC1_Handler for TC waveform interrupt**/
#define TC_Handler  TC0_Handler // (TC0 channel 1)
#define TC_IRQn     TC0_IRQn

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


void TC_Handler(void) {
	// debug_pulse(1);
	uint32_t ra, rc;
	if ((tc_get_status(TC, TC_CHANNEL_WAVEFORM) & TC_SR_CPCS) == TC_SR_CPCS) {
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
		tc_write_ra(TC, TC_CHANNEL_WAVEFORM, ra);
		tc_write_rc(TC, TC_CHANNEL_WAVEFORM, rc);
		current_channel++;	
	}
}

void set_ppm_out_channel_value(int idx, int value) {
	ppm_out_channels[idx] = value;
}

void ppm_out_initialize(void)
{
	uint32_t rc;
	// enable clock for TC0 channel 0 (ID 27)
	PMC_PCER0 |= (1 << 27);
	// configure PPM Out Pin
	ioport_set_pin_mode(PIN_PPM_OUT, PIN_TC_WAVEFORM_MUX);
	
	REG_PIOB_PDR |= (1 << 25);
	
	tc_init(TC, TC_CHANNEL_WAVEFORM,
			TC_CMR_TCCLKS_TIMER_CLOCK1
			| TC_CMR_WAVE /* Waveform mode is enabled */
			| TC_CMR_ACPA_CLEAR /* RA Compare Effect: set */
			| TC_CMR_ACPC_SET /* RC Compare Effect: clear */
		    | TC_CMR_CPCTRG  /* UP mode with automatic trigger on RC Compare */
	);

	rc = sysclk_get_peripheral_bus_hz(TC) /
			2 /
			50;
	tc_write_rc(TC, TC_CHANNEL_WAVEFORM, rc);
	
	NVIC_DisableIRQ(TC_IRQn);
	NVIC_ClearPendingIRQ(TC_IRQn);
	NVIC_SetPriority(TC_IRQn, 0);
	NVIC_EnableIRQ(TC_IRQn);
	tc_enable_interrupt(TC, TC_CHANNEL_WAVEFORM, TC_IER_CPCS);
	
	// initialize rc channels using PWM min value
	for (int i = 0; i < NUMBER_OF_RC_CHANNELS; i++) {
		ppm_out_channels[i] = MIN_PWM_MICROS;
	}
	
	tc_start(TC, TC_CHANNEL_WAVEFORM);
	
	
		// enable clock for timer
	/* PMC_PCER0 = PMC_PCER0 | 0x10000000u;
	
    TC0_CHANNEL1_CCR = TC0_CHANNEL1_CCR | 0x00000002u;
	TC0_CHANNEL1_CMR = TC0_CHANNEL1_CMR | TC_CMR_TCCLKS_TIMER_CLOCK1	| TC_CMR_WAVE | TC_CMR_ACPA_CLEAR | TC_CMR_ACPC_SET | TC_CMR_CPCTRG;
	
	TC0_CHANNEL1_RC = TICKS_PER_MILLISECOND * UPDATE_CONTROLLER_MILLIS;

	ICER0 = ICER0 |  0x10000000u;
	ICPR0 = ICPR0 |  0x10000000u;
 //	NVIC_SetPriority(TC1_IRQn, PID_INTERRUPT_PRIORITY);
	ISER0 = ISER0 | 0x10000000u;
	// interrupt on rc compare	
	TC0_CHANNEL1_IER = TC0_CHANNEL1_IER | 0x00000010u;
	// start tc0 channel 1
    TC0_CHANNEL1_CCR = TC0_CHANNEL1_CCR | 0x00000005u;
*/

}



