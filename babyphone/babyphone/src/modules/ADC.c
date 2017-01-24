/*
 * ADC.c
 *
 * Created: 19.01.2017 22:42:13
 *  Author: tmueller
 */ 
#include "asf.h"
#include "stdio.h"

//	ADC, diese defines fehlen leider in der CMSIS

#define REG_ADC_CDR1			(*(__I  uint32_t*)0x400C0054U) // ADC Channel Data Register
#define REG_ADC_CDR2			(*(__I  uint32_t*)0x400C0058U) // ADC Channel Data Register
#define REG_ADC_CDR3			(*(__I  uint32_t*)0x400C005CU) // ADC Channel Data Register
#define REG_ADC_CDR4			(*(__I  uint32_t*)0x400C0060U) // ADC Channel Data Register
#define REG_ADC_CDR5			(*(__I  uint32_t*)0x400C0064U) // ADC Channel Data Register
#define REG_ADC_CDR6			(*(__I  uint32_t*)0x400C0068U) // ADC Channel Data Register
#define REG_ADC_CDR7			(*(__I  uint32_t*)0x400C006CU) // ADC Channel Data Register
#define REG_ADC_CDR8			(*(__I  uint32_t*)0x400C0070U) // ADC Channel Data Register
#define REG_ADC_CDR9			(*(__I  uint32_t*)0x400C0074U) // ADC Channel Data Register
#define REG_ADC_CDR10			(*(__I  uint32_t*)0x400C0078U) // ADC Channel Data Register
#define REG_ADC_CDR11			(*(__I  uint32_t*)0x400C007CU) // ADC Channel Data Register
#define REG_ADC_CDR12			(*(__I  uint32_t*)0x400C0080U) // ADC Channel Data Register
#define REG_ADC_CDR13			(*(__I  uint32_t*)0x400C0084U) // ADC Channel Data Register
#define REG_ADC_CDR14			(*(__I  uint32_t*)0x400C0088U) // ADC Channel Data Register
#define REG_ADC_CDR15			(*(__I  uint32_t*)0x400C008CU) // ADC Channel Data Register

//	für ADC_ISR()
//	Anzahl Aufrufe
volatile	int		ADC_ISR_CNT_1ms = 0;							// 1ms zählen

volatile	float	U_A0 = 2.5, U_A1 = 2.5, U_A2 = 2.5;
volatile	float	MAX_U_A0 = 0, MIN_U_A0 = 5.0, MIT_U_A0 = 0;
volatile	float	MAX_U_A1 = 0, MIN_U_A1 = 5.0, MIT_U_A1 = 0;
volatile	float	MAX_U_A2 = 0, MIN_U_A2 = 5.0, MIT_U_A2 = 0;
volatile	float	U_GES;
volatile	float	BAND_U_A0 = 0, BAND_U_A1 = 0, BAND_U_A2 = 0;

volatile	int		svpwm_int = 0;									// Flag wird hier gesetzt und in der Endlosschleife
// der MAIN() zurück gesetzt
// -> für synchrone Vorgänge zu ADC_INT

//	für Poti am Analogeingng A0
volatile	float	ADC_POTI = 0.0f;							//

//	MOTOR1
volatile	float	POT_CNT1_1		= 0, POT_CNT2_1 = 0;
//	MOTOR2
volatile	float	POT_CNT1_2		= 0, POT_CNT2_2 = 0;

volatile	float	POT_CNT_3		= 0;
volatile	float	POT_V_1			= 0.0;							// Geschwindigkeit MOTOR1
volatile	float	POT_V_2			= 0.0;							// Geschwindigkeit MOTOR2


//	für Potis an den Analogeingängen A0, A1, A2
volatile	int		ADC_A1_i = 0;									// Integer, Poti HapStik vertikal
volatile	int		ADC_A2_i = 0;									// Integer, Poti HapStik horizontal

// SnapShot der ADC-Werte in der Print-Ausgabe erstellt
volatile	int		SS_ADC_A0_i;
volatile	int		SS_ADC_A1_i;
volatile	int		SS_ADC_A2_i;

volatile	float	ADC_A1_f = 0.0f;								// Float, Poti HapStik vertikal
volatile	float	ADC_A2_f = 0.0f;								// Float, Poti HapStik horizontal

volatile	float	POT_U_ver_NULL1 = 0.0f;							// ADC-Spannungswert in der vertikalen Sticknullposition
volatile	float	POT_U_ver_MAX1 = 0.0f;							// ADC-Spannungswert in der vertikalen Stickobenposition
volatile	float	POT_U_ver_MIN1 = 0.0f;							// ADC-Spannungswert in der vertikalen Stickuntenposition

volatile	float	POT_U_ver_NULL2 = 0.0f;							// ADC-Spannungswert in der vertikalen Sticknullposition
volatile	float	POT_U_ver_MAX2 = 0.0f;							// ADC-Spannungswert in der vertikalen Stickobenposition
volatile	float	POT_U_ver_MIN2 = 0.0f;							// ADC-Spannungswert in der vertikalen Stickuntenposition


volatile	float	POT_PF_ver = 0.0f;								// Proportionalitätsfaktor zur akzeullen Winkelberechnung
volatile	float	POT_ver_WKL = 0.0f;								// Winkel Stick vertikal

volatile	float	POT_ver_WKL_n = 0.0f;							// neuer Winkel Rotor vertikal
volatile	float	POT_ver_WKL_a = 0.0f;							// alter Winkel Rotor vertikal

volatile	float	POT_ver_V = 0.0f;								// Drehgeschwindigkeit U/s vertikales StickPoti
volatile	float	POT_hor_V = 0.0f;								// Drehgeschwindigkeit U/s horizontales StickPoti


volatile	float	POT_U_hor_NULL1 = 0.0f;							// ADC-Spannungswert in der horizontalen Sticknullposition
volatile	float	POT_U_hor_MAX1 = 0.0f;							// ADC-Spannungswert in der horizontalen Stickobenposition
volatile	float	POT_U_hor_MIN1 = 0.0f;							// ADC-Spannungswert in der horizontalen Stickuntenposition

volatile	float	POT_U_hor_NULL2 = 0.0f;							// ADC-Spannungswert in der horizontalen Sticknullposition
volatile	float	POT_U_hor_MAX2 = 0.0f;							// ADC-Spannungswert in der horizontalen Stickobenposition
volatile	float	POT_U_hor_MIN2 = 0.0f;							// ADC-Spannungswert in der horizontalen Stickuntenposition

// Mittelwertfilter

static		int		af_count_i	= 0;						// Laufvariable im AF-Array
static		int		SUM_AF_i_1	= 0;						// Summe MOTOR1
static		int		SUM_AF_i_2	= 0;						// Summe MOTOR2

static		int		AF_A0_i		= 0;						// Average_Filterwert für ADC-Kanal A0
static		float	AF_A0_f		= 0;
static		int		AF_A1_i		= 0;						// Average_Filterwert für ADC-Kanal A0
static		float	AF_A1_f		= 0;

//	für Anzeige der Verteilung von Werten

volatile	int		PRINT_VERT_cnt_i = 0;			// Zählvariable für PRINT_WERT Ausgaben

int has_ADC_completed_20_conversions(void) {
	return svpwm_int;
}

void reset_ADC(void) {
	svpwm_int = 0;
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

	REG_ADC_MR = REG_ADC_MR			| 0x1E900205u;				// LOWRES von 1 auf 0 (10Bit -> 12Bit)
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
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx     Init ADC       xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx       ADC_ISR        xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

void ADC_Handler(void)
{		
	printf("ADC!!!\r\n");
//	Kommt alle 50us (20kHz)
//	Wird von der PWM Event Line getriggert
//	Impuls auf der EventLine wird mit jedem 10. PWM-Referenzimpuls erzeugt (1 x 100us = 100us) -> siehe Init_PWM


//	Hier wird jetzt die Mittelwertbildung (10-fach) durchgeführt

//	Analogkanäle lesen 1x/100us -> Snapshot
	SS_ADC_A0_i = REG_ADC_CDR7;									// Poti HapStik vertikal
	SS_ADC_A1_i = REG_ADC_CDR6;									// Poti HapStik horizontal
	SS_ADC_A2_i = REG_ADC_CDR5;									// Poti, Sollwertvorgabe
	
/*
//	PRINT
//	Jeden ADC-Wert printen
	printf("| SS_ADC_A0_i   : %15d|\r\n",
	SS_ADC_A0_i);
*/


	SUM_AF_i_1 = SUM_AF_i_1 + SS_ADC_A0_i;						// Summenbildung: aktueller ADC_A0-Wert wird dazu addiert
	SUM_AF_i_2 = SUM_AF_i_2 + SS_ADC_A1_i;						// Summenbildung: aktueller ADC_A1-Wert wird dazu addiert

	af_count_i ++;												// "af" -> Average-Filter
																
	PRINT_VERT_cnt_i = PRINT_VERT_cnt_i + 1;					// Zähler der Print-Ausgaben

		


	if (af_count_i == 20)										// 20 ADC-Werte wurden addiert, 1ms ist vorbei
	{
		
//		printf("|WERTE-VERTEILUNG-------------------------------------------------------------------------------------------------------------------|\r\n");

//		AF_A0_f = 0.5 + SUM_AF_i / 10.0;						// hier 0,5 zur Rundung dazu addieren
		
		AF_A0_f = SUM_AF_i_1 / 20.0;							// Addition von 0,5 kann aber auch entfallen, da der Absolutwert nicht so wichtig ist,
		AF_A1_f = SUM_AF_i_2 / 20.0;							// da ja alle Positionen beim Start des Sticks eingemessen werden

		AF_A0_i = AF_A0_f;										// Ganzzahliger Anteil wird übergeben
		AF_A1_i = AF_A1_f;
		
		af_count_i = 0;											// Counter auf Anfang stellen
		SUM_AF_i_1 = 0;											// Summe wieder rücksetzen
		SUM_AF_i_2 = 0;
				
		svpwm_int = 1;
																// -> zur Übergabe an Endlosschleife in MAIN

	}
	

	
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx       ADC_ISR        xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx