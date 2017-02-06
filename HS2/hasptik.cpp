/*
 * GccLibrary1.cpp
 *
 * Created: 06.02.2017 22:54:51
 * Author : tmueller
 */ 

#include "sam.h"
#include "stdio.h"
#include "math.h"					//			z.B. cos(x)
#include "string.h"
#include "conf_hapstik.h"
#include "stdbool.h"
#include "registers.h"
#include "hapstik.h"

static double PRECISION = 0.001;					//Anzahl Nachkommastellen, -> hier ohne Rundung!

char serial_out_string[50];
bool is_serial_out_dirty = false;




#define ZERO_POWER     0.0
#define QUARTER_POWER 0.25
#define HALF_POWER    0.5
#define FULL_POWER    1.0

void print_to_serial_asynchronously(char outString[]) {
	sprintf(serial_out_string, outString);
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
	if (debug_pin == 0) 
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

void compute_space_vector_PWM(space_vector *sv) {
	if (sv->X >= 0) {
		if (sv->Y >= 0) {
			sv->PWM_w = (1 - sv->Y - sv->X)/2;
			sv->PWM_v = sv->PWM_w + sv->Y;
			sv->PWM_u = sv->PWM_v + sv->X;
			} else {
			if (sv->Z >= 0) {
				sv->PWM_v = (1 - sv->X - sv->Z)/2;
				sv->PWM_u = sv->PWM_v + sv->X;
				sv->PWM_w = sv->PWM_u + sv->Z;
				} else {
				sv->PWM_v = (1 + sv->Y + sv->Z)/2;
				sv->PWM_w = sv->PWM_v - sv->Y;
				sv->PWM_u = sv->PWM_w - sv->Z;
			}
		}
		} else {
		if (sv->Y >= 0) {
			if (sv->Z >= 0) {
				sv->PWM_u = (1 - sv->Z - sv->Y)/2;
				sv->PWM_w = sv->PWM_u + sv->Z;
				sv->PWM_v = sv->PWM_w + sv->Y;
				} else {
				sv->PWM_w = (1+sv->Z+sv->X)/2;
				sv->PWM_u = sv->PWM_w - sv->Z;
				sv->PWM_v = sv->PWM_u - sv->X;
			}
			} else {
			sv->PWM_u = (1+sv->X+sv->Y)/2;
			sv->PWM_v = sv->PWM_u - sv->X;
			sv->PWM_w = sv->PWM_v - sv->Y;
		}
	}
}

void update_pwm_duty_cycles(space_vector *sv_motor) {
	if (sv_motor->motor == 'Y') {
		REG_PWM_CDTYUPD0 = (1 - sv_motor->PWM_u) * 2100;
		REG_PWM_CDTYUPD1 = (1 - sv_motor->PWM_v) * 2100;
		REG_PWM_CDTYUPD2 = (1 - sv_motor->PWM_w) * 2100;
		} else if (sv_motor->motor == 'X') {
		REG_PWM_CDTYUPD3 = (1 - sv_motor->PWM_u) * 2100;
		REG_PWM_CDTYUPD4 = (1 - sv_motor->PWM_v) * 2100;
		REG_PWM_CDTYUPD5 = (1 - sv_motor->PWM_w) * 2100;
		} else if (sv_motor->motor == 'N') {
		// nullvektor auf alle kanäle!
		REG_PWM_CDTYUPD0 = (1 - sv_motor->PWM_u) * 2100;
		REG_PWM_CDTYUPD1 = (1 - sv_motor->PWM_v) * 2100;
		REG_PWM_CDTYUPD2 = (1 - sv_motor->PWM_w) * 2100;
		REG_PWM_CDTYUPD3 = (1 - sv_motor->PWM_u) * 2100;
		REG_PWM_CDTYUPD4 = (1 - sv_motor->PWM_v) * 2100;
		REG_PWM_CDTYUPD5 = (1 - sv_motor->PWM_w) * 2100;
	}
	//	A Duty Cycle Update, Übernahme der Register Enable PWM channels (S.1016)
	//	- Register: PWM_SCUC (Sync Channel Update)
	//	- es gibt nur ein Bit in diesem Register:UPDULOCK (Unlock synchronous channels update)
	//	- wird es 1 gesetzt werden die Register für Duty Cycle ... übernommen

	//	Ausgabe
	REG_PWM_SCUC = 0x00000001u;
}

volatile int    af_count_i	= 0;						// Laufvariable im AF-Array
volatile int	SUM_AF_i_x	= 0;						// Summe MOTOR1
volatile int	SUM_AF_i_y	= 0;						// Summe MOTOR2

ADC_inputs get_oversampled_adc_inputs(void) {
	ADC_inputs inputs = { (int)SUM_AF_i_x / af_count_i, (int)SUM_AF_i_y / af_count_i };  // A1 ist y Poti, A0 ist x Poti
	af_count_i = 0;											// Counter auf Anfang stellen	
	SUM_AF_i_x = 0;											// Summe wieder rücksetzen
	SUM_AF_i_y = 0;
	return inputs;
}


void ADC_Handler(void)
{		
	debug_pulse(0);
	SUM_AF_i_x += REG_ADC_CDR6;						// Summenbildung: aktueller ADC_A1-Wert wird dazu addiert
	SUM_AF_i_y += REG_ADC_CDR7;						// Summenbildung: aktueller ADC_A0-Wert wird dazu addiert
	af_count_i ++;												// "af" -> Average-Filter
}

void adc_initialize(void)
{
/*A 
	ADC-Leitungen (C.2 - C.9) im Prozessor vom PIO-Controller trennen
	- Register: PIO_PDR (PIO Disable), S.634
	- zugehörige Bits auf 1 setzen
	- macht sich im Register: PIO_PSR (PIO Status) als 0 an den zugehörenden Bits bemerkbar!
*/
	REG_PIOC_PDR = REG_PIOC_PDR		|	0x000003FCu;


/*	Clock enable für ADC
	Instanzen (S.38)
	- ID_ADC: 37 (Identifier für die ADC)
		- S.563
		- Register: PMC_PCER1 (Power Management Controller Peripheral Clock Enable Register 1)
*/
	REG_PMC_PCER1 = REG_PMC_PCER1	|	0x00000020u;

//	Reset ADC (S.1332)	
	REG_ADC_CR = 0x00000001;	


//	12 Bit / x4  -> ADC-Werte: 21 (2926 ... 2946)
//	Verteilung VW_MW = 2936;

	REG_ADC_MR = REG_ADC_MR			| 0x1E900209u;				// LOWRES von 1 auf 0 (10Bit -> 12Bit)
//	12 Bit / x4  -> ADC-Werte: 10 (2931 ... 2940) nach 1000 Messungen

	REG_ADC_CHER = REG_ADC_CHER		| 0x000000E0u;

	REG_ADC_CGR = REG_ADC_CGR		| 0x0000F000u;		// Gain = 1 für CH5 (A2) -> Poti zur Spollwertvorgabe
														// Gain = 4 für CH6 (A1)
														// Gain = 4 für CH7 (A0)
														// Gain = 1	für restliche Analogeingänge 													


	REG_ADC_IER = REG_ADC_IER | 0x00000080u;
	DisableIRQ(37);
	ClearPendingIRQ(37);
 	SetPriorityIRQ(37, 0);
	EnableIRQ(37);



}

/*
 * PID.c
 *
 * Created: 19.01.2017 22:44:04
 *  Author: tmueller
 */ 
#include "math.h"		
#include "conf_hapstik.h"			

#define	PI		3.141592654f
#define	WK1		(PI/180)
#define	WK2		(PI*2/3)
#define	WK3		(PI*4/3)

#define UPDATE_CONTROLLER_MILLIS 1  // recalculate all controllers within defined frequency 1/x
#define TICKS_PER_MILLISECOND 42000
#define PID_INTERRUPT_PRIORITY 3


#define	ANGLE_OFFSET_X 23.0		 // HAPStik spezifischer Offset für Mittenposition in Winkelgraden
#define	ANGLE_OFFSET_Y 30.0

#define MAX_NORM_STEPS_THROW 1500
#define ANGLE_DEGREES_PER_NORM_STEPS ((float)(MAX_THROW_DEGREES)/(float)(MAX_NORM_STEPS_THROW))

#define ELECTRICAL_MECHANICAL_GEAR_FACTOR 7  // dies ist vom Motortyp (#Magnete etc.) abhängig

#define CENTER_ROTATION_ANGLE_X (ELECTRICAL_MECHANICAL_GEAR_FACTOR * ANGLE_OFFSET_X)
#define CENTER_ROTATION_ANGLE_Y (ELECTRICAL_MECHANICAL_GEAR_FACTOR * ANGLE_OFFSET_Y)


const int MAX_THROW_DEGREES = 33;

const int POSITIVE_DIRECTION = 1;
const int NEGATIVE_DIRECTION = -1;


int x_max_throw_plus_adc = 0;
int x_max_throw_minus_adc = 0;
int y_max_throw_plus_adc = 0;
int y_max_throw_minus_adc = 0;
int x_null_adc = 0;
int y_null_adc = 0;

float x_minus_prop_factor = 0.0;
float x_plus_prop_factor = 0.0;
float y_minus_prop_factor = 0.0;
float y_plus_prop_factor = 0.0;

typedef struct  {
	float setpoint;
	float input;
	float lastInput;
	double ITerm;
	double outMax;
	double outMin;
	double kp;
	double ki;
	double kd;
} pid_controller;
		
pid_controller motor_X_position_controller = { 0.0, 0.0, 0.0, 0.0, 1.0, -1.0, 0.0015, 0.0, 0.0035 };
pid_controller motor_Y_position_controller = { 0.0, 0.0, 0.0, 0.0, 1.0, -1.0, 0.0015, 0.0, 0.0035 };
	
space_vector motor_X_space_vector = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 'X', CENTER_ROTATION_ANGLE_X};
space_vector motor_Y_space_vector = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 'Y', CENTER_ROTATION_ANGLE_Y };		
space_vector null_space_vector = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 'N', 0.0 };

int volatile cnt_1ms_poll = 0;

typedef struct  {
	uint16_t current_value;
	uint16_t past_value;
	uint8_t li_step;
} interpolated_rc_channel;

interpolated_rc_channel current_raw_channels[NUMBER_OF_RC_CHANNELS];

void compute_space_vector_components(space_vector *sv, float rotation_angle, float power_factor) {
	sv->X = power_factor * cos(rotation_angle * WK1);
	sv->Y = power_factor * cos(rotation_angle * WK1-WK2);
	sv->Z = power_factor * cos(rotation_angle * WK1-WK3);
	compute_space_vector_PWM(sv);
}

void rotate_motor_degrees_from_start_angle(space_vector *sv, float start_angle, int no_of_degrees, int direction_sign) {
	float power_factor = HALF_POWER;
	for (int step =1 ; step <=no_of_degrees; step++) {
		if (step == no_of_degrees)
		{
			// wenn Stick in Zielposition dann max. Power zum "Festhalten"
			power_factor = FULL_POWER;
		}
		float rot_angle = start_angle + (ELECTRICAL_MECHANICAL_GEAR_FACTOR * step * direction_sign); // Winkel pro Step = 1°
		compute_space_vector_components(sv, rot_angle, power_factor);
		update_pwm_duty_cycles(sv);
	//	delay_ms(2);
	}
}

void adjust_neutral_position(void) {

	compute_space_vector_components(&motor_X_space_vector, motor_X_space_vector.center_angle, QUARTER_POWER);
	compute_space_vector_components(&motor_Y_space_vector, motor_Y_space_vector.center_angle, QUARTER_POWER);
	update_pwm_duty_cycles(&motor_X_space_vector);
	update_pwm_duty_cycles(&motor_Y_space_vector);
}

void calibration_sequence(void) {
	adjust_neutral_position();
	//	Stick beruhigen lassen
//	delay_ms(200);
	//	MOTOR_Y
	//	---------------------------------------------------------------------------------------------------- Stick in Position +Y
	rotate_motor_degrees_from_start_angle(&motor_Y_space_vector, CENTER_ROTATION_ANGLE_Y, MAX_THROW_DEGREES, POSITIVE_DIRECTION);
//	delay_ms(200);								// Stick beruhigen lassen
	y_max_throw_plus_adc = REG_ADC_CDR7;					
	//	----------------------------------------------------------------------------------------------------- Stick in Position -Y
	//	HOCH
	//	Schleifendurchläufe 66 (von +33° nach -33°)
	rotate_motor_degrees_from_start_angle(&motor_Y_space_vector, CENTER_ROTATION_ANGLE_Y + MAX_THROW_DEGREES * ELECTRICAL_MECHANICAL_GEAR_FACTOR , (2 * MAX_THROW_DEGREES), NEGATIVE_DIRECTION);
//	delay_ms(200);								// Stick beruhigen lassen
	y_max_throw_minus_adc = REG_ADC_CDR7;

	rotate_motor_degrees_from_start_angle(&motor_Y_space_vector, CENTER_ROTATION_ANGLE_Y - MAX_THROW_DEGREES * ELECTRICAL_MECHANICAL_GEAR_FACTOR , MAX_THROW_DEGREES, POSITIVE_DIRECTION);
//	delay_ms(200);								// Stick beruhigen lassen
	y_null_adc = REG_ADC_CDR7;

	//	MOTOR_X
	//	---------------------------------------------------------------------------------------------------- Stick in Position +Y
	rotate_motor_degrees_from_start_angle(&motor_X_space_vector, CENTER_ROTATION_ANGLE_X, MAX_THROW_DEGREES, POSITIVE_DIRECTION);
//	delay_ms(200);								// Stick beruhigen lassen
	x_max_throw_plus_adc = REG_ADC_CDR6;
	//	----------------------------------------------------------------------------------------------------- Stick in Position -Y
	//	HOCH
	//	Schleifendurchläufe 66 (von +33° nach -33°)
	rotate_motor_degrees_from_start_angle(&motor_X_space_vector, CENTER_ROTATION_ANGLE_X + MAX_THROW_DEGREES * ELECTRICAL_MECHANICAL_GEAR_FACTOR , (2 * MAX_THROW_DEGREES), NEGATIVE_DIRECTION);
//	delay_ms(200);								// Stick beruhigen lassen
	x_max_throw_minus_adc = REG_ADC_CDR6;

	rotate_motor_degrees_from_start_angle(&motor_X_space_vector, CENTER_ROTATION_ANGLE_X - MAX_THROW_DEGREES * ELECTRICAL_MECHANICAL_GEAR_FACTOR , MAX_THROW_DEGREES, POSITIVE_DIRECTION);
//	delay_ms(200);								// Stick beruhigen lassen
	x_null_adc = REG_ADC_CDR6;
	

	char s1[32];
	char s2[32];
	char s3[32];
	char s4[32];
	char s5[32];
		//	PRINT
	printf("| minusX_33     : %15s| nullX         : %15s| plusX_33     : %15s|\r\n",
	doubleToString(s1, x_max_throw_minus_adc), doubleToString(s2, x_null_adc), doubleToString(s3, x_max_throw_plus_adc));
	
	printf("| minusY_33     : %15s| nullY         : %15s| plusY_33     : %15s|\r\n\n",
	doubleToString(s1, y_max_throw_minus_adc), doubleToString(s2, y_null_adc), doubleToString(s3, y_max_throw_plus_adc));

	update_pwm_duty_cycles(&null_space_vector);
//	delay_ms(2);

	//	----- Kennlinie in Nullpunkt verschieben -> Nullpunkt wird 0
	x_max_throw_minus_adc = x_max_throw_minus_adc - x_null_adc;
	x_max_throw_plus_adc  = x_max_throw_plus_adc  - x_null_adc;

	y_max_throw_minus_adc = y_max_throw_minus_adc - y_null_adc;
	y_max_throw_plus_adc  = y_max_throw_plus_adc  - y_null_adc;

	//	PRINT
	printf("| minusX_33     : %15s| plusX_33      : %15s|\r\n",
	doubleToString(s1, x_max_throw_minus_adc), doubleToString(s3, x_max_throw_plus_adc));
	
	printf("| minusY_33     : %15s| plusY_33      : %15s|\r\n\n\n",
	doubleToString(s1, y_max_throw_minus_adc), doubleToString(s3, y_max_throw_plus_adc));
	
	/* Proportinalitätsfaktoren für "-" und "+"Kennlinienbereiche
	//	Proportionalitätsfaktor für +33° -> +1500 und -33° -> -1500
	*/
	x_minus_prop_factor	= (double)MAX_NORM_STEPS_THROW / x_max_throw_minus_adc;
	x_plus_prop_factor	= (double)MAX_NORM_STEPS_THROW / x_max_throw_plus_adc;

	y_minus_prop_factor	= (double)MAX_NORM_STEPS_THROW / y_max_throw_minus_adc;
	y_plus_prop_factor	= (double)MAX_NORM_STEPS_THROW / y_max_throw_plus_adc;
	
	printf("| propfaktor_mX : %15s| propfaktor_pX : %15s|\r\n",
	doubleToString(s1, x_minus_prop_factor), doubleToString(s2, x_plus_prop_factor));	

	printf("| propfaktor_mY : %15s| propfaktor_pY : %15s|\r\n\n",
	doubleToString(s1, y_minus_prop_factor), doubleToString(s2, y_plus_prop_factor));
	
}




double pid_compute(pid_controller *controller)
{
	float error = controller->setpoint - controller->input;
	controller->ITerm += (controller->ki * error);
	
	if(controller->ITerm > controller->outMax) {
		controller->ITerm = controller->outMax;
	} else if(controller->ITerm < controller->outMin) {
		controller->ITerm = controller->outMin;
	}
	float dInput = (controller->input - controller->lastInput);
	
	// Reglerausgang berechnen
	double output = controller->kp * error + controller->ITerm - controller->kd * dInput;

	// Überläufe kappen
	if(output > controller->outMax) {
		output = controller->outMax;
	} else if(output < controller->outMin) {
		output = controller->outMin;
	}
	//	letzten Wert speichern
	controller->lastInput = controller->input;
	return output;
}

void compute_space_vector_motor_X(space_vector *sv, int input, float power_factor) {
	float rotation_angle = ELECTRICAL_MECHANICAL_GEAR_FACTOR * ((ANGLE_DEGREES_PER_NORM_STEPS * input) + ANGLE_OFFSET_X);
	if (power_factor >= 0) {
			rotation_angle += 90;
			compute_space_vector_components(sv, rotation_angle, power_factor);
	} else {
		rotation_angle -= 90;
		compute_space_vector_components(sv, rotation_angle, (-1) * power_factor);
	}	
}

void compute_space_vector_motor_Y(space_vector *sv, int input, float power_factor) {
	float rotation_angle = ELECTRICAL_MECHANICAL_GEAR_FACTOR * ((ANGLE_DEGREES_PER_NORM_STEPS * input) + ANGLE_OFFSET_Y);
	if (power_factor >= 0) {
		rotation_angle += 90;
		compute_space_vector_components(sv, rotation_angle, power_factor);
		} else {
		rotation_angle -= 90;
		compute_space_vector_components(sv, rotation_angle, (-1) * power_factor);
	}
}


int get_normalised_input_x(int raw) {
	int norm_input_x = raw - x_null_adc;
	if (norm_input_x >= 0)
	{	
		norm_input_x = norm_input_x * x_plus_prop_factor;
	} 
	else
	{	
		norm_input_x = (-1)	*	norm_input_x * x_minus_prop_factor;
	}	
	return norm_input_x;
}

int get_normalised_input_y(int raw) {
	int norm_input_y = raw - y_null_adc;
	if (norm_input_y >= 0)
	{	
		norm_input_y = norm_input_y * y_plus_prop_factor;
	} 
	else
	{	
		norm_input_y = (-1)	*	norm_input_y * y_minus_prop_factor;
	}	
	return norm_input_y;
}	

	



void compute_all_controllers(void) {
	ADC_inputs raw_inputs = get_oversampled_adc_inputs();
	motor_X_position_controller.input = get_normalised_input_x(raw_inputs.X);
	compute_space_vector_motor_X(&motor_X_space_vector, motor_X_position_controller.input,  pid_compute(&motor_X_position_controller));
	motor_Y_position_controller.input = get_normalised_input_y(raw_inputs.Y);
	compute_space_vector_motor_Y(&motor_Y_space_vector, motor_Y_position_controller.input,  pid_compute(&motor_Y_position_controller));
	update_pwm_duty_cycles(&motor_X_space_vector);
	update_pwm_duty_cycles(&motor_Y_space_vector);
	
}

static void display_debug_output() {
	#ifdef CONSOLE_DEBUG_LEVEL
		char s1[32], s2[32], s3[32];
		printf("| X1      : %15s| Y1      : %15s| Z1  : %15s\r\n",
		doubleToString(s1, motor_X_space_vector.X), doubleToString(s2, motor_X_space_vector.Y), doubleToString(s3, motor_X_space_vector.Z));
		printf("| Sollwert      : %15s\r\n",
		doubleToString(s1, motor_X_position_controller.setpoint));
		printf("\r\n");
		printf("| X2      : %15s| Y2      : %15s| Z2  : %15s\r\n",
		doubleToString(s1, motor_Y_space_vector.X), doubleToString(s2, motor_Y_space_vector.Y), doubleToString(s3, motor_Y_space_vector.Z));
		printf("| Sollwert      : %15s\r\n",
		doubleToString(s1, motor_Y_position_controller.setpoint));
		printf("------------\r\n");
	#endif
}

int normalise_channel(uint16_t raw_channel_value) {
	return (raw_channel_value - 1100) * 3;
}

float map_prop_channel_to_zero_one_float(uint16_t raw_channel_value) {
	return (raw_channel_value - 600) * 0.001f;
}

void set_stick_raw_channel(int channel_id, rc_channel *captured_channel) {
	current_raw_channels[channel_id].current_value = captured_channel->current_captured_ppm_value;
	current_raw_channels[channel_id].past_value = captured_channel->last_captured_ppm_value;
	current_raw_channels[channel_id].li_step = 0;
}

int get_linear_interpolated_raw_channel_value(int channel_id) {
	int delta = (current_raw_channels[channel_id].current_value - current_raw_channels[channel_id].past_value) / 20;
	int li_value = current_raw_channels[channel_id].past_value + ( current_raw_channels[channel_id].li_step * delta);
	current_raw_channels[channel_id].li_step++;
	return li_value;
}

void TC1_Handler(void) {
	if ((TC0_CHANNEL1_SR & TC_SR_CPCS) == TC_SR_CPCS) {
		//	debug_pulse(1);
			cnt_1ms_poll++;
		
			motor_Y_position_controller.setpoint = normalise_channel(get_linear_interpolated_raw_channel_value(3));
			motor_X_position_controller.setpoint = normalise_channel(get_linear_interpolated_raw_channel_value(4));
			
		
			motor_X_position_controller.outMax = map_prop_channel_to_zero_one_float(get_linear_interpolated_raw_channel_value(6));
			motor_Y_position_controller.outMax = map_prop_channel_to_zero_one_float(get_linear_interpolated_raw_channel_value(7));
			motor_X_position_controller.outMin = (-1) * map_prop_channel_to_zero_one_float(get_linear_interpolated_raw_channel_value(6));
			motor_Y_position_controller.outMin = (-1) * map_prop_channel_to_zero_one_float(get_linear_interpolated_raw_channel_value(7));
			
		/*		motor_X_position_controller.outMax = HALF_POWER;
				motor_Y_position_controller.outMax = HALF_POWER;
				motor_X_position_controller.outMin = (-1) * HALF_POWER;
				motor_Y_position_controller.outMin = (-1) * HALF_POWER;
			motor_Y_position_controller.kp = 0.006f * map_prop_channel_to_zero_one_float(current_raw_channel_values[6]);*/
					
			performance_trace_start(0);
			compute_all_controllers();
			performance_trace_stop(0);
		
			if (cnt_1ms_poll % 1000 == 0) {
				 display_debug_output();
			}
		
	}
}




void pid_initialize(void)
{
	
	for (int i = 0; i < NUMBER_OF_RC_CHANNELS; i++)
	{
		current_raw_channels[i].current_value = MID_PWM_MICROS;
		current_raw_channels[i].past_value = MID_PWM_MICROS;
		current_raw_channels[i].li_step = 0;
	}
	
	uint32_t rc;
	// enable clock for timer
	PMC_PCER0 = PMC_PCER0 | 0x10000000u;
	
    TC0_CHANNEL1_CCR = TC0_CHANNEL1_CCR | 0x00000002u;
	TC0_CHANNEL1_CMR = TC0_CHANNEL1_CMR | TC_CMR_TCCLKS_TIMER_CLOCK1	| TC_CMR_WAVE | TC_CMR_ACPA_CLEAR | TC_CMR_ACPC_SET | TC_CMR_CPCTRG;
	
	TC0_CHANNEL1_RC = TICKS_PER_MILLISECOND * UPDATE_CONTROLLER_MILLIS;

	DisableIRQ(28);
	ClearPendingIRQ(28);
    SetPriorityIRQ(28, PID_INTERRUPT_PRIORITY);
	EnableIRQ(28);
	
	// interrupt on rc compare	
	TC0_CHANNEL1_IER = TC0_CHANNEL1_IER | 0x00000010u;
	// start tc0 channel 1
    TC0_CHANNEL1_CCR = TC0_CHANNEL1_CCR | 0x00000005u;
	
}




	

void pwm_initialize(void)
{
	
/*A	
	Instanzen (S.38)
	- ID_PWM: 36 (Identifier für die Pulsweitenmodulation PWM)
	- Clock enable für PWM
		- S.563
		- Register: PMC_PCER1 (Power Management Controller Peripheral Clock Enable Register 1)
*/
	REG_PMC_PCER1 = REG_PMC_PCER1	|	0x00000010u;

/*A	
	Disable PWM channels (S.1008)
	- Register: PWM_DIS (PWM Disable)
	- Kanal 0: Motor1_X
	- Kanal 1: Motor1_Y	
	- Kanal 2: Motor1_Z
	
	- Kanal 3: Motor2_X
	- Kanal 4: Motor2_Y
	- Kanal 5: Motor2_Z	
	
	- Kanal 6: Referenzsignal
*/
	REG_PWM_DIS = REG_PWM_DIS		|	0x0000007Fu;

/*A 
	Clock einstellen
	- PWM_CLK (DIVA, PREA, DIVB, PREB)
	- PWM_CMRx 
	- entfällt hier, da alle PWM-Kanäle mit MCK (84 MHz) getaktet werden
*/

/* 
	Synchronität, Kanäle 0,1,2,3,4,5 als synchrone Kanäle (S.1014) definieren:
	"They have the same source clock, the same period, the same alignment and are started together."
	
	- Register: PWM_SCM (Sync Channel Mode)

	- Synchronous Channels Update Mode2
	REG_PWM_SCM = REG_PWM_SCM		|	0x00020007u;
	- Synchronous Channels Update Mode0: "Manual write of double buffer registers and manual update of synchronous channels"
*/
	REG_PWM_SCM = REG_PWM_SCM		|	0x0000003Fu;

/* 
	Update Period Register (S.1017)
	Defines the time between each update of the synchronous channels if automatic trigger of the update is activated
*/
//	REG_PWM_SCUP = REG_PWM_SCUP		|	0x00000000u;

/*A 
	Kanal Mode, Aligment, Polarität, Dead Time (S.1044)
	- Register: PWM_CMRx (Channel Mode)
	- Dead Time:
		- Überlappungsfreiheit von komplementären Ausgängen, damit nicht beide MOSFETs kurz gleichzeitig eingeschalten
		- DTE=1 (Dead Time Generator Enable)
*/	
//	- Kanal 0:
//		- center aligned, Start mit high-Level, hier Dead Time enable für Ansteuerung Renesas-Board
//	REG_PWM_CMR0 = REG_PWM_CMR0		|	0x00010300u;	


//	- Kanal 0:
//		- center aligned, Start mit high-Level, hier Dead Time disable für Ansteuerung DS8313 (Leistungstreiber H-Brücke)
	REG_PWM_CMR0 = REG_PWM_CMR0		|	0x00000300u;

//	- Kanal 1:
//		- center aligned, Start mit high-Level, Dead Time disabled
	REG_PWM_CMR1 = REG_PWM_CMR1		|	0x00000300u;

//	- Kanal 2:
//		- center aligned, Start mit high-Level, Dead Time disabled
	REG_PWM_CMR2 = REG_PWM_CMR2		|	0x00000300u;

//	- Kanal 3:
//		- center aligned, Start mit high-Level, Dead Time disabled
	REG_PWM_CMR3 = REG_PWM_CMR3		|	0x00000300u;

//	- Kanal 4:
//		- center aligned, Start mit high-Level, Dead Time disabled
	REG_PWM_CMR4 = REG_PWM_CMR4		|	0x00000300u;

//	- Kanal 5:
//		- center aligned, Start mit high-Level, Dead Time disabled
	REG_PWM_CMR5 = REG_PWM_CMR5		|	0x00000300u;

//	- Kanal 6:
//		- left   aligned, Start mit high-Level, Dead Time disabled
	REG_PWM_CMR6 = REG_PWM_CMR6		|	0x00000200u;

/*A 
	PWM Periode festlegen (S.1048)
	Erklärung: waveform period (S.978)
	- Register: PWM_CPRD
	- Periode = CPRD/84MHz = 8400/84MHz = 100us
	- Periode = CPRD/84MHz = 4200/84MHz =  50us		

*/

//	Werte für 50us:
	REG_PWM_CPRD0 = REG_PWM_CPRD0	|	0x00000834u;		// 2100
	REG_PWM_CPRD1 = REG_PWM_CPRD1	|	0x00000834u;		// 2100	
	REG_PWM_CPRD2 = REG_PWM_CPRD2	|	0x00000834u;		// 2100

	REG_PWM_CPRD3 = REG_PWM_CPRD3	|	0x00000834u;		// 2100
	REG_PWM_CPRD4 = REG_PWM_CPRD4	|	0x00000834u;		// 2100
	REG_PWM_CPRD5 = REG_PWM_CPRD5	|	0x00000834u;		// 2100
	
	REG_PWM_CPRD6 = REG_PWM_CPRD6	|	0x00001068u;		// 4200
	
		
/*
	PWM Duty Cycle (S.1046)
	- Register: PWM_CDTY (untere 16 Bit)
	- hier nur den initialen Wert einstellen
	- ursprünglich:
		- PWM->PWM_CH_NUM[0].PWM_CDTY = (1 - DC_M1_X) * 4200;
		- da aber DC_M1_X = 0 kann man kürzer schreiben:
	PWM->PWM_CH_NUM[0].PWM_CDTY = 4200;	
*/

	REG_PWM_CDTY0 = REG_PWM_CDTY0	|	0x00000834u;		// 2100
	REG_PWM_CDTY1 = REG_PWM_CDTY1	|	0x00000834u;		// 2100
	REG_PWM_CDTY2 = REG_PWM_CDTY2	|	0x00000834u;		// 2100	

	REG_PWM_CDTY3 = REG_PWM_CDTY3	|	0x00000834u;		// 2100
	REG_PWM_CDTY4 = REG_PWM_CDTY4	|	0x00000834u;		// 2100
	REG_PWM_CDTY5 = REG_PWM_CDTY5	|	0x00000834u;		// 2100
	
	REG_PWM_CDTY6 = REG_PWM_CDTY6	|	0x00000054u;		// 84	für 1us-Impuls (Referenzsignal für Oszi)

//	----------			
/* 
	Wir wollen nach dem Start eines SVPWM-Zyklus, 1us vergehen lassen und dann den ADC starten
	Das ist ein günstiger Zeitpunkt für die Strommessungen!!!
	
	PWM-Comparison Unit  (S.993)
		- Register: PWM Comparison x Mode Register (S.1042)
			- comparison x (x= 0...7, 8 PWM-Kanäle) erlauben, Bit CEN = 1
			- wir benutzen den Comparison 0
*/


//	Impuls auf der EventLine wird mit jedem 1. PWM-Referenzimpuls erzeugt (1  x 50us = 50us)
	REG_PWM_CMPM0 = REG_PWM_CMPM0	|	0x00000001u;

/*	
	PWM Comparison x Value Register (S.1040)
		- Register: PWM_CMPVx
		- comparison x value to be compared with the counter of the channel 0
		- wenn dieser Wert erreicht wird, wird "Comparison x" aktiv, was als Input für den Start des ADC genutzt wird
		- ACHTUNG:
			- der Wert wird auf 1us oder 12ns gesetzt 
				-> der optimale Abtastzeitpunkt muss mit dem Oszi ermittelt werden 
			- wenn also auf dem Oszi die fallende Flanke des Referenzsignals zu sehen ist, so wird in diesem Moment
			  der ADC gestartet -> ist hier noch anpassbar
		- Bit CVM: the comparison x between the counter of the channel 0 and the comparison x value is performed when this counter is
		  incrementing (bei center aligned, so ist ja Kanal 0 eingestellt, kann man hier auch "decrementing" wählen)	  
*/
	REG_PWM_CMPV0 = REG_PWM_CMPV0	|	0x00000001u;		// 0x00000054u -> 1us,  dann Start des ADC
															// 0x00000001u -> 12ns, dann Start des ADC (ist der kleinste Wert)	

/*A 	
	PWM Event Line x Register (S.995)
		- Register: PWM_ELMRx (S.1035)
		- a pulse is generated on the event line x when the comparison 0...7 match
		- wir benutzen den Comparison 0
*/
	REG_PWM_ELMR = REG_PWM_ELMR		|	0x00000001u;		// Comparison 0 ausgewählt
//	----------	

/*A 
	PWM-Leitungen (C.2, C.4, C.6, C.8, C.21, C.22, C.23) im Prozessor vom PIO-Controller trennen
	- Register: PIO_PDR (PIO Disable), S.634
	- zugehörige Bits auf 1 setzen
	- macht sich im Register: PIO_PSR (PIO Status) als 0 an den zugehörenden Bits bemerkbar!
*/
//	REG_PIOC_PDR = REG_PIOC_PDR		|	0x000003FCu;		// -> Renesas
	REG_PIOC_PDR = REG_PIOC_PDR		|	0x00E00154u;		// -> DS8313

/*A 
	PWM-Leitungen (C.2, C.4, C.6, C.8, C.21, C.22, C.23) auf "peripheral B function" stellen
	- Register: PIO_ABSR (PIO AB Select), S.656
	- zugehörige Bits auf 1 setzen
*/
//	REG_PIOC_ABSR = REG_PIOC_ABSR	|	0x000003FCu;		// -> Renesas
	REG_PIOC_ABSR = REG_PIOC_ABSR	|	0x00E00154u;		// -> DS8313	

	
/*A 
	Enable PWM channels (S.1007)
	- gleichzeitige Start von Kanal 0 (steht stellvertretend für Kanäle 0...5 und Kanal 6 (Referenzimpuls), damit alle Kanäle 
	  den gleichen zeitlichen Nullpunkt haben.
	- initiale Ausgabe und Freischaltung der PWMs für Motor_1 und Motor_2:
*/
	REG_PWM_ENA = REG_PWM_ENA	|	0x00000041u; 

	//	Ausgabe an den PWM-VController

	REG_PWM_CDTYUPD0 = (1 - 0.5) * 2100;
	REG_PWM_CDTYUPD1 = (1 - 0.5) * 2100;
	REG_PWM_CDTYUPD2 = (1 - 0.5) * 2100;

	REG_PWM_CDTYUPD3 = (1 - 0.5) * 2100;
	REG_PWM_CDTYUPD4 = (1 - 0.5) * 2100;
	REG_PWM_CDTYUPD5 = (1 - 0.5) * 2100;
	//	A Duty Cycle Update, Übernahme der Register Enable PWM channels (S.1016)
	//	- Register: PWM_SCUC (Sync Channel Update)
	//	- es gibt nur ein Bit in diesem Register:UPDULOCK (Unlock synchronous channels update)
	//	- wird es 1 gesetzt werden die Register für Duty Cycle ... übernommen

	//	Ausgabe
	REG_PWM_SCUC = 0x00000001u;
}

HAPStik::HAPStik() {
	pwm_initialize();
	adc_initialize();
	calibration_sequence();
	pid_initialize();
}

HAPStik::~HAPStik() {}

void HAPStik::setPositionX(int position) {
	motor_X_position_controller.setpoint = position;
}

void HAPStik::setPositionY(int position) {
	motor_Y_position_controller.setpoint = position;
}