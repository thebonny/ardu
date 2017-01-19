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
void double_speed(void);
void half_speed(void);
void start_record(void);
void stop_record(void);
void start_playback(void);
void loop_playback(void);

#endif /* RECORD_PLAYBACK_H_ */