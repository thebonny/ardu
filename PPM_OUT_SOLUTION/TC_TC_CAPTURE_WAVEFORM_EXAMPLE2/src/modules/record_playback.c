#include <asf.h>
#include <modules/record_playback.h>
#include <modules/ppm_capture.h>
#include <modules/ppm_out.h>

/** Use TC Peripheral 2 **/
#define TC  TC2

/** Configure TC2 channel 2 as waveform output. **/
#define TC_CHANNEL_WAVEFORM 1
#define ID_TC_WAVEFORM      ID_TC7

#define RC_PER_MILLISECOND 42000
#define DEFAULT_FRAMERATE_MILLIS 20

uint16_t recorded_flight_records[5000][NUMBER_OF_RC_CHANNELS]; // we record only 5 channels (Gas, Pitch, Roll, Nick, Rudder), we use uint16 to reduce memory usage
int current_record = 0;


void copy_captured_channels_to_record() {
	for (int i = 0; i < NUMBER_OF_RC_CHANNELS; i++) {
		recorded_flight_records[current_record][i] = get_captured_channel_value(i);
	}
	
}

void TC7_Handler(void) {
	if ((tc_get_status(TC, TC_CHANNEL_WAVEFORM) & TC_SR_CPCS) == TC_SR_CPCS) {
		
		for (int i = 0; i < NUMBER_OF_RC_CHANNELS; i++) {
			set_ppm_out_channel_value(i, get_captured_channel_value(i));
		}
	/*	if (0) {
			copy_captured_channels_to_record();
			current_record++;
		}*/
		printf("d\r\n");
	}
}



void set_master_framerate(int milliseconds) {
	tc_write_rc(TC, TC_CHANNEL_WAVEFORM, milliseconds * RC_PER_MILLISECOND);
}

void double_speed() {
	tc_write_rc(TC, TC_CHANNEL_WAVEFORM, tc_read_rc(TC, TC_CHANNEL_WAVEFORM) / 2);
}

void half_speed() {
	tc_write_rc(TC, TC_CHANNEL_WAVEFORM, tc_read_rc(TC, TC_CHANNEL_WAVEFORM) * 2);
}

void start_record() {
	current_record = 0;
	tc_start(TC, TC_CHANNEL_WAVEFORM);
}

void stop_record() {
	tc_stop(TC, TC_CHANNEL_WAVEFORM);
	current_record = 0;
}

void record_playback_initialize(void)
{
	uint32_t rc;
	// enable clock for timer
	sysclk_enable_peripheral_clock(ID_TC_WAVEFORM);
	tc_init(TC, TC_CHANNEL_WAVEFORM,
	TC_CMR_TCCLKS_TIMER_CLOCK1
	| TC_CMR_WAVE /* Waveform mode is enabled */
	| TC_CMR_ACPA_CLEAR /* RA Compare Effect: set */
	| TC_CMR_ACPC_SET /* RC Compare Effect: clear */
	| TC_CMR_CPCTRG  /* UP mode with automatic trigger on RC Compare */
	);

	rc = DEFAULT_FRAMERATE_MILLIS * RC_PER_MILLISECOND;
		
	tc_write_rc( TC, TC_CHANNEL_WAVEFORM, rc);

	NVIC_DisableIRQ(TC7_IRQn);
	NVIC_ClearPendingIRQ(TC7_IRQn);
	NVIC_SetPriority(TC7_IRQn, 1);
	NVIC_EnableIRQ(TC7_IRQn);
	tc_enable_interrupt(TC, TC_CHANNEL_WAVEFORM, TC_IER_CPCS);
	

}
