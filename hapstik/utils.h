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
#include "cycle_counter.h"

#define F_CPU 84000000

/**
 * @def delay_ms
 * @brief Delay in milliseconds.
 * @param delay Delay in milliseconds
 */
#define delay_ms(delay)     ((delay) ? cpu_delay_ms(delay, F_CPU) : cpu_delay_us(1, F_CPU))

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

void gpio_initialize(void);

uint32_t pin_to_mask(uint32_t pin);

#endif /* UTILS_H_ */