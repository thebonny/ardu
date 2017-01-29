/*
 * ADC.c
 *
 * Created: 19.01.2017 22:42:13
 *  Author: tmueller
 */ 
#include "asf.h"
#include "stdio.h"
#include "includes/ADC.h"

//	ADC, diese defines fehlen leider in der CMSIS
#define REG_ADC_CDR6			(*(__I  uint32_t*)0x400C0068U) // ADC Channel Data Register
#define REG_ADC_CDR7			(*(__I  uint32_t*)0x400C006CU) // ADC Channel Data Register

//	für Potis an den Analogeingängen A0, A1, A2
volatile	int		ADC_A1_i = 0;									// Integer, Poti HapStik vertikal
volatile	int		ADC_A2_i = 0;									// Integer, Poti HapStik horizontal

// SnapShot der ADC-Werte in der Print-Ausgabe erstellt
volatile	int		SS_ADC_A0_i;
volatile	int		SS_ADC_A1_i;
volatile	int		SS_ADC_A2_i;

volatile	float	ADC_A1_f = 0.0f;								// Float, Poti HapStik vertikal
volatile	float	ADC_A2_f = 0.0f;								// Float, Poti HapStik horizontal


// Mittelwertfilter

static		int		af_count_i	= 0;						// Laufvariable im AF-Array
static		int		SUM_AF_i_1	= 0;						// Summe MOTOR1
static		int		SUM_AF_i_2	= 0;						// Summe MOTOR2

static		int		AF_A0_i		= 0;						// Average_Filterwert für ADC-Kanal A0
static		float	AF_A0_f		= 0;
static		int		AF_A1_i		= 0;						// Average_Filterwert für ADC-Kanal A0
static		float	AF_A1_f		= 0;



ADC_inputs get_oversampled_adc_inputs(void) {
	
	AF_A0_f = SUM_AF_i_1 / af_count_i;							// Addition von 0,5 kann aber auch entfallen, da der Absolutwert nicht so wichtig ist,
	AF_A1_f = SUM_AF_i_2 / af_count_i;							// da ja alle Positionen beim Start des Sticks eingemessen werden
	AF_A0_i = AF_A0_f;										// Ganzzahliger Anteil wird übergeben
	AF_A1_i = AF_A1_f;		
	printf("A: %d\r\n", af_count_i);
	af_count_i = 0;											// Counter auf Anfang stellen
	
	SUM_AF_i_1 = 0;											// Summe wieder rücksetzen
	SUM_AF_i_2 = 0;
	
	ADC_inputs inputs = { AF_A0_i, AF_A1_i };
		
	return inputs;
}


void ADC_Handler(void)
{		
	SS_ADC_A0_i = REG_ADC_CDR7;									// Poti HapStik vertikal
	SS_ADC_A1_i = REG_ADC_CDR6;									// Poti HapStik horizontal
	
	SUM_AF_i_1 = SUM_AF_i_1 + SS_ADC_A0_i;						// Summenbildung: aktueller ADC_A0-Wert wird dazu addiert
	SUM_AF_i_2 = SUM_AF_i_2 + SS_ADC_A1_i;						// Summenbildung: aktueller ADC_A1-Wert wird dazu addiert

	af_count_i ++;												// "af" -> Average-Filter
}

void	INIT_ADC(void)
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
	NVIC_DisableIRQ(ADC_IRQn);
	NVIC_ClearPendingIRQ(ADC_IRQn);
	NVIC_SetPriority(ADC_IRQn, 0);
	//	Enable ADC interrupt, schreibt das Register ISER im NVIC (Nested Vector Interrupt Controller)
	NVIC_EnableIRQ(ADC_IRQn);


}