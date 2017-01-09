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
#define NUMBER_OF_RECORDS 5000
#define MODE_RECORD 0
#define MODE_PLAYBACK 1
#define MODE_BYPASS 2

uint8_t mode = MODE_BYPASS;
uint8_t loop = 0;


uint16_t recorded_flight_records[NUMBER_OF_RECORDS][NUMBER_OF_RC_CHANNELS]; // we record only 5 channels (Gas, Pitch, Roll, Nick, Rudder), we use uint16 to reduce memory usage
int current_record = 0;
int max_recorded_record = 0;


void copy_captured_channels_to_record() {
	for (int i = 0; i < NUMBER_OF_RC_CHANNELS; i++) {
		recorded_flight_records[current_record][i] = get_captured_channel_value(i);
	}
	
}

void TC7_Handler(void) {
	if ((tc_get_status(TC, TC_CHANNEL_WAVEFORM) & TC_SR_CPCS) == TC_SR_CPCS) {
		if (mode == MODE_BYPASS || mode == MODE_RECORD) {
			for (int i = 0; i < NUMBER_OF_RC_CHANNELS; i++) {
				set_ppm_out_channel_value(i, get_captured_channel_value(i));
			}
		}
		if (mode == MODE_RECORD) {
			copy_captured_channels_to_record();
			printf("Frame #: %d\r", current_record);
			current_record++;
			if (current_record >= NUMBER_OF_RECORDS) {
				stop_record();
				printf("REcording stopped as record limit is reached!\n\r");
			}
		} else if (mode == MODE_PLAYBACK) {
			for (int i = 0; i < NUMBER_OF_RC_CHANNELS; i++) {
				set_ppm_out_channel_value(i, recorded_flight_records[current_record][i]);
			}
			printf("Frame #: %d\r", current_record);
			current_record++;
			if (current_record >= max_recorded_record) {
				if (loop == 0) { 
					mode = MODE_BYPASS;
					printf("Playback stopped as max recorded record %d is reached!\n\r", max_recorded_record);
				} else {
					// looped mode
					current_record = 0;r
				}
				
			}
		}
	}
}



void set_master_framerate(int milliseconds) {
	tc_write_rc(TC, TC_CHANNEL_WAVEFORM, milliseconds * RC_PER_MILLISECOND);
}

void double_speed() {
	uint32_t rc = tc_read_rc(TC, TC_CHANNEL_WAVEFORM);
	
	printf("Previous framerate was: %dms, framerate is now %dms\r\n", rc / RC_PER_MILLISECOND, (rc/(RC_PER_MILLISECOND * 2)));
	tc_disable_interrupt(TC, TC_CHANNEL_WAVEFORM, TC_IER_CPCS);
	tc_stop(TC, TC_CHANNEL_WAVEFORM);
	tc_write_rc(TC, TC_CHANNEL_WAVEFORM, rc / 2);
	tc_enable_interrupt(TC, TC_CHANNEL_WAVEFORM, TC_IER_CPCS);
	tc_start(TC, TC_CHANNEL_WAVEFORM);
	
}

void half_speed() {
	uint32_t rc = tc_read_rc(TC, TC_CHANNEL_WAVEFORM);
	printf("Previous framerate was: %dms, framerate is now %dms\r\n", rc / RC_PER_MILLISECOND, (rc/(RC_PER_MILLISECOND /2)));
	tc_disable_interrupt(TC, TC_CHANNEL_WAVEFORM, TC_IER_CPCS);
	tc_stop(TC, TC_CHANNEL_WAVEFORM);
	tc_write_rc(TC, TC_CHANNEL_WAVEFORM, rc * 2);
	tc_enable_interrupt(TC, TC_CHANNEL_WAVEFORM, TC_IER_CPCS);
	tc_start(TC, TC_CHANNEL_WAVEFORM);
}

void start_record() {
	current_record = 0;
	mode = MODE_RECORD;
}

void stop_record() {
	mode = MODE_BYPASS;
	max_recorded_record = current_record;
}

void start_playback() {
	if (mode == MODE_RECORD) {
		stop_record();
	}
	loop = 0;
	current_record = 0;
	mode = MODE_PLAYBACK;
}

void loop_playback() {
	loop = 1;
	
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
	tc_start(TC, TC_CHANNEL_WAVEFORM);

}
