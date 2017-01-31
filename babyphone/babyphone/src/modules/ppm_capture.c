
#include <asf.h>
#include <conf_board.h>
#include <conf_clock.h>
#include <includes/ppm_capture.h>

#define TC_CAPTURE_TIMER_SELECTION TC_CMR_TCCLKS_TIMER_CLOCK3
/** Use TC Peripheral 2 **/
#define TC  TC2

/** Configure TC2 channel 0 as capture input. **/
#define TC_CHANNEL_CAPTURE 0
#define ID_TC_CAPTURE      ID_TC6  //TC6 is channel 0 of TC2 module!!

// #define PIN_TC_CAPTURE_MUX IOPORT_MODE_MUX_B // peripheral B // TODO warum ist das Anschalten des pins nicht erforderlich??

/** Use TC2_Handler for TC capture interrupt**/
#define TC_Handler  TC6_Handler
#define TC_IRQn     TC6_IRQn

#define PPM_OFFSET 400


volatile rc_channel rc_channels[8];
static uint32_t gs_ul_captured_rb;

static int channel_id = 0;

void TC_Handler(void)
{
	if ((tc_get_status(TC, TC_CHANNEL_CAPTURE) & TC_SR_LDRBS) == TC_SR_LDRBS) {
		uint16_t micros = 0;
		gs_ul_captured_rb = tc_read_rb(TC, TC_CHANNEL_CAPTURE);
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

int get_interpolated_channel_ppm(int channel_id, int step) {
	int delta = (rc_channels[channel_id].current_captured_ppm_value - rc_channels[channel_id].last_captured_ppm_value) / 20;
	return rc_channels[channel_id].last_captured_ppm_value + (delta * step);
}

void ppm_capture_initialize(void)
{
	
	for (int i = 0; i < 8; i++)
	{
		rc_channels[i].current_captured_ppm_value = 0;
		rc_channels[i].last_captured_ppm_value = 0;
	}
	
	pmc_enable_periph_clk(ID_TC_CAPTURE);
	tc_init(TC, TC_CHANNEL_CAPTURE,
	TC_CAPTURE_TIMER_SELECTION /* Clock Selection */
	| TC_CMR_LDRA_FALLING /* RA Loading: rising edge of TIOA */
	| TC_CMR_LDRB_RISING /* RB Loading: falling edge of TIOA */
	| TC_CMR_ABETRG /* External Trigger: TIOA */
	| TC_CMR_ETRGEDG_RISING /* External Trigger Edge: Falling edge */
	);
	
	NVIC_DisableIRQ(TC_IRQn);
	NVIC_ClearPendingIRQ(TC_IRQn);
	NVIC_SetPriority(TC_IRQn, 5);
	NVIC_EnableIRQ(TC_IRQn);
	
	tc_enable_interrupt(TC, TC_CHANNEL_CAPTURE, TC_IER_LDRBS);
	tc_start(TC, TC_CHANNEL_CAPTURE);
	
	
}

