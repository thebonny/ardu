/*
 * ADC.c
 *
 * Created: 19.01.2017 22:42:13
 *  Author: tmueller
 */ 
#include "stdio.h"
#include "ADC.h"
#include "utils.h"
#include "registers.h"


// Mittelwertfilter

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