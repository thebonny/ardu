/*
 * utils.c
 *
 * Created: 18.01.2017 00:29:54
 *  Author: tmueller
 */ 
//	* Double to ASCII prototype. Must be declared before usage in main function !!

#include "math.h"					//			z.B. cos(x)
#include "string.h"
#include "conf_hapstik.h"
#include "registers.h"
#include "stdbool.h"
#include "stdio.h"


static double PRECISION = 0.001;					//Anzahl Nachkommastellen, -> hier ohne Rundung!

char * serial_out_string;
bool is_serial_out_dirty = false;

void print_to_serial_asynchronously(char * outString) {
	serial_out_string = outString;
	is_serial_out_dirty = true;
}

char * get_serial_out_string(void) {
	is_serial_out_dirty = false;
	return serial_out_string;
}

bool is_serial_out_ready(void) {
	return is_serial_out_dirty;
}


void SetPriorityIRQ(uint32_t irq_id, uint32_t priority)
{
    NestedIC->IP[irq_id] = ((priority << (8 - __NVIC_PRIO_BITS)) & 0xff);  
}

void DisableIRQ(uint32_t irq_id)
{
  NestedIC->ICER[(irq_id >> 5)] = (1 << (irq_id & 0x1F)); /* disable interrupt */
}

void ClearPendingIRQ(uint32_t irq_id)
{
  NestedIC->ICPR[(irq_id >> 5)] = (1 << (irq_id & 0x1F)); /* Clear pending interrupt */
}

void EnableIRQ(uint32_t irq_id)
{
  NestedIC->ISER[(irq_id >> 5)] = (1 << (irq_id & 0x1F)); /* enable interrupt */
}

uint32_t pin_to_mask(uint32_t pin)
{
	return 1U << (pin & 0x1F);
}

void debug_pulse(int debug_pin) {
	#ifdef PULSE_DEBUG_LEVEL
		if (debug_pin == 0) {
			//	Debug_Pin C.12
			REG_PIOC_SODR	= REG_PIOC_SODR		|	0x00001000u; 		// Ausgang C12 =1 (DUE_Pin6)
			REG_PIOC_CODR	= REG_PIOC_CODR		|	0x00001000u; 		// Ausgang C12 =0 (DUE_Pin6)
		} else if (debug_pin == 1) {
			//	Debug_Pin C.24
			REG_PIOC_SODR	= REG_PIOC_SODR		|	0x00004000u; 		// Ausgang C14 =1 (DUE_Pin6)
			REG_PIOC_CODR	= REG_PIOC_CODR		|	0x00004000u; 		// Ausgang C14 =0 (DUE_Pin6)
		}
	#endif
}

void performance_trace_start(int debug_pin) {
	#ifdef PERFORMANCE_TRACE_LEVEL
	if (debug_pin == 0) {
			//	Debug_Pin C.12
			REG_PIOC_SODR	= REG_PIOC_SODR		|	0x00001000u; 		// Ausgang C12 =1 (DUE_Pin6)
	} else if (debug_pin == 1) {
			//	Debug_Pin C.24
			REG_PIOC_SODR	= REG_PIOC_SODR		|	0x00004000u; 		// Ausgang C14 =1 (DUE_Pin6)
	}
	#endif
}

void performance_trace_stop(int debug_pin) {
	#ifdef PERFORMANCE_TRACE_LEVEL
	if (debug_pin == 0) {
			//	Debug_Pin C.12
			REG_PIOC_CODR	= REG_PIOC_CODR		|	0x00001000u; 		// Ausgang C12 =0 (DUE_Pin6)
	} else if (debug_pin == 1) {
			//	Debug_Pin C.24
			REG_PIOC_CODR	= REG_PIOC_CODR		|	0x00004000u; 		// Ausgang C14 =0 (DUE_Pin6)
	}
	#endif
}


char * doubleToString(char *s, double n);

//	Double to ASCII
char * doubleToString(char *s, double n) {
	// handle special cases
	if (isnan(n)) {
		strcpy(s, "nan");
		} else if (isinf(n)) {
		strcpy(s, "inf");
		} else if (n == 0.0) {
		strcpy(s, "0");
		} else {
		int digit, m, m1;
		char *c = s;
		int neg = (n < 0);
		if (neg)
		n = -n;
		// calculate magnitude
		m = log10(n);
		int useExp = (m >= 14 || (neg && m >= 9) || m <= -9);
		if (neg)
		*(c++) = '-';
		// set up for scientific notation
		if (useExp) {
			if (m < 0)
			m -= 1.0;
			n = n / pow(10.0, m);
			m1 = m;
			m = 0;
		}
		if (m < 1.0) {
			m = 0;
		}
		// convert the number
		while (n > PRECISION || m >= 0) {
			double weight = pow(10.0, m);
			if (weight > 0 && !isinf(weight)) {
				digit = floor(n / weight);
				n -= (digit * weight);
				*(c++) = '0' + digit;
			}
			if (m == 0 && n > 0)
			*(c++) = '.';
			m--;
		}
		if (useExp) {
			// convert the exponent
			int i, j;
			*(c++) = 'e';
			if (m1 > 0) {
				*(c++) = '+';
				} else {
				*(c++) = '-';
				m1 = -m1;
			}
			m = 0;
			while (m1 > 0) {
				*(c++) = '0' + m1 % 10;
				m1 /= 10;
				m++;
			}
			c -= m;
			for (i = 0, j = m-1; i<j; i++, j--) {
				// swap without temporary
				c[i] ^= c[j];
				c[j] ^= c[i];
				c[i] ^= c[j];
			}
			c += m;
		}
		*(c) = '\0';
	}
	return s;
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    Float -> String   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    Float -> String   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//	Aus dem Internet: http://www.geeksforgeeks.org/convert-floating-point-number-string/

// reverses a string 'str' of length 'len'
void reverse(char *str, int len)
{
	int i=0, j=len-1, temp;
	while (i<j)
	{
		temp = str[i];
		str[i] = str[j];
		str[j] = temp;
		i++; j--;
	}
}

//	Converts a given integer x to string str[].  d is the number
//	of digits required in output. If d is more than the number
//	of digits in x, then 0s are added at the beginning.
int intToStr(int x, char str[], int d)
{
	int i = 0;
	while (x)
	{
		str[i++] = (x%10) + '0';
		x = x/10;
	}
	//	If number of digits required is more, then
	//	add 0s at the beginning
	while (i < d)
	str[i++] = '0';
	reverse(str, i);
	str[i] = '\0';
	return i;
}

void float_to_string(float n, char *Ergebnis, int afterpoint);

//	Converts a floating point number to string.
void float_to_string(float n, char *Ergebnis, int afterpoint)
{
	// Extract integer part
	int ipart = (int)n;
	// Extract floating part
	float fpart = n - (float)ipart;
	// convert integer part to string
	int i = intToStr(ipart, Ergebnis, 0);
	// check for display option after point
	if (afterpoint != 0)
	{
		Ergebnis[i] = ',';  // add dot
		// Get the value of fraction part upto given no.
		// of points after dot. The third parameter is needed
		// to handle cases like 233.007
		fpart = fpart * pow(10, afterpoint);
		
		intToStr((int)fpart, Ergebnis + i + 1, afterpoint);
	}
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    Float -> String   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

