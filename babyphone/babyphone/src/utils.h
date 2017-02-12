/*
 * utils.h
 *
 * Created: 18.01.2017 00:30:37
 *  Author: tmueller
 */ 


#ifndef UTILS_H_
#define UTILS_H_
#include "stdint.h"
#include "stdbool.h"

void print_to_serial_asynchronously(char *outString);
char * get_serial_out_string(void);
bool is_serial_out_ready(void);

char * doubleToString(char *s, double n);
void float_to_string(float n, char *Ergebnis, int afterpoint);
void debug_pulse(int debug_pin);

void performance_trace_start(int debug_pin);
void performance_trace_stop(int debug_pin);

void SetPriorityIRQ(uint32_t irq_id, uint32_t priority);
void DisableIRQ(uint32_t irq_id);
void ClearPendingIRQ(uint32_t irq_id);
void EnableIRQ(uint32_t irq_id);

uint32_t pin_to_mask(uint32_t pin);

#endif /* UTILS_H_ */