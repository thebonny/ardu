#include "stdio.h"
#include <record_playback.h>
#include <ppm_capture.h>
#include <ppm_out.h>
#include <PID.h>
#include "utils.h"
#include "registers.h"
#include "conf_hapstik.h"


#define RC_PER_MILLISECOND 42000
#define DEFAULT_FRAMERATE_MILLIS 20
#define NUMBER_OF_RECORDS 2300
#define MODE_RECORD 0
#define MODE_PLAYBACK 1
#define MODE_BYPASS 2

uint8_t mode = MODE_BYPASS;
uint8_t loop = 0;


rc_channel recorded_flight_records[NUMBER_OF_RECORDS][NUMBER_OF_RC_CHANNELS]; // we record only 5 channels (Gas, Pitch, Roll, Nick, Rudder), we use uint16 to reduce memory usage

int current_record = 0;
int max_recorded_record = 0;



void copy_captured_channels_to_record() {
	for (int i = 0; i < NUMBER_OF_RC_CHANNELS; i++) {
		 recorded_flight_records[current_record][i] = get_captured_raw_channel(i);
	}
	
}

void TC7_Handler(void) {
	// debug_pulse(1);
	if ((TC2_CHANNEL1_SR & TC_SR_CPCS) == TC_SR_CPCS) {
		char *out;
		if (mode == MODE_BYPASS || mode == MODE_RECORD) {
			for (int i = 0; i < NUMBER_OF_RC_CHANNELS; i++) {
				rc_channel channel = get_captured_raw_channel(i);
				set_ppm_out_channel_value(i, channel.current_captured_ppm_value);
				set_stick_raw_channel(i, &channel);
			}
		}
		if (mode == MODE_RECORD) {
			copy_captured_channels_to_record();
			print_to_serial_asynchronously(sprintf(out,"Frame #: %d\r\n", current_record));
			current_record++;
			if (current_record >= NUMBER_OF_RECORDS) {
				stop_record();
				print_to_serial_asynchronously(sprintf(out,"REcording stopped as record limit is reached!\n\r"));
			}
		} else if (mode == MODE_PLAYBACK) {
			for (int i = 0; i < NUMBER_OF_RC_CHANNELS; i++) {
				set_ppm_out_channel_value(i, recorded_flight_records[current_record][i].current_captured_ppm_value);
				set_stick_raw_channel(i, &recorded_flight_records[current_record][i]);
			}
			print_to_serial_asynchronously(sprintf(out,"Frame #: %d\r", current_record));
			current_record++;
			if (current_record >= max_recorded_record) {
				if (loop == 0) { 
					mode = MODE_BYPASS;
					print_to_serial_asynchronously(sprintf(out,"Playback stopped as max recorded record %d is reached!\n\r", max_recorded_record));
				} else {
					// looped mode
					current_record = 0;
				}
				
			}
		}
	}
}



void set_master_framerate(int milliseconds) {
	TC2_CHANNEL1_RC = milliseconds * RC_PER_MILLISECOND;
}

void double_speed() {
	// print_to_serial_asynchronously("Previous framerate was: %fms, framerate is now %ldms\r\n", TC2_CHANNEL1_RC / RC_PER_MILLISECOND, (TC2_CHANNEL1_RC/(RC_PER_MILLISECOND * 2)));
	TC2_CHANNEL1_RC = TC2_CHANNEL1_RC / 2;
}

void half_speed() {
	// print_to_serial_asynchronously("Previous framerate was: %fms, framerate is now %ldms\r\n", TC2_CHANNEL1_RC / RC_PER_MILLISECOND, (TC2_CHANNEL1_RC/(RC_PER_MILLISECOND /2)));
	TC2_CHANNEL1_RC = TC2_CHANNEL1_RC * 2;
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
	// enable clock for timer
	// TC2 channel 1 is peripheral ID = 34, second bit in PCER1
	REG_PMC_PCER1 |= (1 << 2);
	
	TC2_CHANNEL1_CCR |= 0x00000002u;
	TC2_CHANNEL1_CMR |= TC_CMR_TCCLKS_TIMER_CLOCK1	| TC_CMR_WAVE | TC_CMR_ACPA_CLEAR | TC_CMR_ACPC_SET | TC_CMR_CPCTRG;
	TC2_CHANNEL1_RC = DEFAULT_FRAMERATE_MILLIS * RC_PER_MILLISECOND;

	DisableIRQ(34);
	ClearPendingIRQ(34);
	SetPriorityIRQ(34, 5);
	EnableIRQ(34);
	
	// interrupt on rc compare	
	TC2_CHANNEL1_IER |= 0x00000010u;
	// start tc2 channel 1
    TC2_CHANNEL1_CCR |= 0x00000005u;

}
