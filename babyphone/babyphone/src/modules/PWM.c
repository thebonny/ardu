/*
 * PWM.c
 *
 * Created: 19.01.2017 22:39:08
 *  Author: tmueller
 */ 
//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx       SVPWM     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//	SpaceVektorPulsWeitenModulation
//	Gemessene maximale Ausführungszeiten:
//	1.:		20,89us		mit "void SVPWM(float uum, float uvm, float uwm)"
//	2.:		23,69us		mit "void SVPWM(double uum, double uvm, double uwm)"
//	Delta:	 2,80us
//	TIME:	17us

//	Input:
//		- X, Y, Z				Spannungen aus Funktion INV_CLARKE_PARK
//	Output:
//		- PWMu1, PWMv1, PWMw1	"Center aligned PWM-Signale" die direkt auf die Halbbrücken ausgegeben werden
//		- PWMu2, PWMv2, PWMw2

#include "asf.h"
#include "includes/PID.h"



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

void update_pwm_duty_cycles(space_vector *sv_motor_X, space_vector *sv_motor_Y) {
		REG_PWM_CDTYUPD0 = (1 - sv_motor_X->PWM_u) * 2100;
		REG_PWM_CDTYUPD1 = (1 - sv_motor_X->PWM_v) * 2100;
		REG_PWM_CDTYUPD2 = (1 - sv_motor_X->PWM_w) * 2100;
		
		REG_PWM_CDTYUPD3 = (1 - sv_motor_Y->PWM_u) * 2100;
		REG_PWM_CDTYUPD4 = (1 - sv_motor_Y->PWM_v) * 2100;
		REG_PWM_CDTYUPD5 = (1 - sv_motor_Y->PWM_w) * 2100;		
		
		//	A Duty Cycle Update, Übernahme der Register Enable PWM channels (S.1016)
		//	- Register: PWM_SCUC (Sync Channel Update)
		//	- es gibt nur ein Bit in diesem Register:UPDULOCK (Unlock synchronous channels update)
		//	- wird es 1 gesetzt werden die Register für Duty Cycle ... übernommen

		//	Ausgabe
		REG_PWM_SCUC = 0x00000001u;
}
	

void	INIT_PWM(void)
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
