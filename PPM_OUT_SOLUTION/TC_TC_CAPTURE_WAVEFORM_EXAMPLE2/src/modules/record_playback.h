/*
 * record_playback.h
 *
 * Created: 09.01.2017 14:12:41
 *  Author: tmueller
 */ 


#ifndef RECORD_PLAYBACK_H_
#define RECORD_PLAYBACK_H_


void record_playback_initialize(void);
void set_master_framerate(int milliseconds);
void double_speed();
void half_speed();
void start_record();
void stop_record();

#endif /* RECORD_PLAYBACK_H_ */