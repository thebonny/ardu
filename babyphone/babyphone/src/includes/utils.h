/*
 * utils.h
 *
 * Created: 18.01.2017 00:30:37
 *  Author: tmueller
 */ 


#ifndef UTILS_H_
#define UTILS_H_


char * doubleToString(char *s, double n);
void float_to_string(float n, char *Ergebnis, int afterpoint);
void debug_pulse(int debug_pin);

#endif /* UTILS_H_ */