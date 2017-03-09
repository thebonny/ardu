

/*	xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx                  HAPSTIK Modul                xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
DATUM:		09.03.2017
AUTOR:		André Frank
NAME:		Spirale


WIRKUNGSWEISE:
			2 Positionsregler für jede Achse
				- mit um 90° versetzte Sinus angesteuert



------------------------------------------------------------------------------------------------------------------------------
*/
//	ANFANG **********************************************     Includes       *************************************************
	#include "asf.h"
	#include "conf_board.h"
	#include "delay.h"					// z.B.	delay_us(564)	-> warte 564us
										//		delay_ms(20)	-> warte 20ms
										//		delay_s(2)		-> warte 2s
	#include "stdio_serial.h"			// UART		z.B. printf("MEC22_STEPS:   %i\r\n", ENC1_C);
	#include "conf_clock.h"
	#include "math.h"					//			z.B. cos(x)
	#include "string.h"
//	ENDE ************************************************     Includes       *************************************************

//	ANFANG **********************************************     defines        *************************************************
//	ADC, diese defines fehlen leider in der CMSIS
	#define REG_ADC_CDR             (*(__I  uint32_t*)0x400C0050U) // ADC Channel Data Register
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

	#define	PI		3.141592654f
	#define	WK1		(PI/180)
	#define	WK2		(PI*2/3)
	#define	WK3		(PI*4/3)

	#define	NOR1	0.022f
//	ENDE ************************************************     defines        *************************************************

//	ANFANG ********************************************     Global Variables       *******************************************

/*
Grundsätzliches:
volatile:	ist ein Operator, der dem Compiler sagt, dass der Inhalt einer Variablen sich außerhalb
			des Programmkontextes ändern kann.

/*
static:		ist ein Schlüsselwort 
			- vor Funktionen, damit sie nur in der Datei genutzt werden kann, in der sie steht.
			- vor Variablen, damit die Variable von mehreren Funktionen innerhalb der Datei genutzt werden kann
			- die Variable wird auf einer festen Speicheradresse gespeichert. Daraus ergibt sich die Möglichkeit, dass
			  eine Funktion, die mit static-Variablen arbeitet, beim nächsten Durchlauf die Informationen erneut nutzen kann,
		      die in der Variablen gespeichert wurden, wie in einem Gedächtnis.	
			Die Speicherklasse static legt innerhalb einer Funktion fest, daß dauerhaft für
			die Variable Speicherplatz reserviert wird. Normalerweise ist der Wert einer Variablen
			innerhalb einer Funktion bei jeden Aufruf anfänglich undefiniert. Bei einer
			statischen Variable ist das nicht der Fall.
*/


//	Für Tänzchen

	volatile	int		VZ		= 1;							// VorZeichen

	volatile	float	SP_W	= 0.0;							// Winkel
	volatile	float	SP_A	= 0.0;							// Amplitude
	volatile	float	SP_DW	= 0.36;							// Delta_Winkel/ms iin Grad

	
	volatile	int		TAE_1ms = 0;							// 1ms zählen	
	volatile	int		SEQUENZ_1ms = 0;						// 1ms zählen			



//	für Ratsche
//	RATSCHE
//	Konstanten
	volatile	int		R_oben				= -1500;			// oberer  maximaler Ratschenpunkt
	volatile	int		R_unten				= +1500;			// unterer maximaler Ratschenpunkt -> bei normiertem HS-Eingang sind das 3001 Steps
																// bzw. 3001 maximal mögliche R_Points
	volatile	float	R_Anzahl			= 21;				// muss float sein! Anzahl von gewünschten Ratschen-Points
	volatile	float	R_Laenge;								// Ratschen_Länge in Steps

//	für Test
	volatile	int		a1_int	= 0;
	volatile	int		a2_int	= 0;
	volatile	int		a3_int	= 0;
	volatile	int		a4_int	= 0;
	volatile	int		a5_int	= 0;				

	volatile	float	a1_flt	= 0;
	volatile	float	a2_flt	= 0;
	volatile	float	a3_flt	= 0;
	volatile	float	a4_flt	= 0;
	volatile	float	a5_flt	= 0;	
	
		
	
	
//	für Schleifen -> Zählvariablen
	volatile	int		a, b, c, d;


//	Zeitverzögerungen (delay)
	volatile	int		TD1;
	volatile	int		TD2;
	volatile	int		TD3;
	volatile	int		TD4;
	volatile	int		TD5;

		
	volatile	float	P_ADC_A0_f;						// Snapshot von den ADC-Werten für Print
	volatile	float	P_ADC_A1_f;
	volatile	float	P_ADC_A2_f;

// Mittelwertfilter
	static		int		af_count_i	= 0;						// Laufvariable im AF-Array
	static		int		SUM_AF_i_1	= 0;						// Summe MOTOR1
	static		int		SUM_AF_i_2	= 0;						// Summe MOTOR2
		
	static		int		AF_A0_i		= 0;						// Average_Filterwert für ADC-Kanal A0
	static		float	AF_A0_f		= 0;
	static		int		AF_A1_i		= 0;						// Average_Filterwert für ADC-Kanal A0
	static		float	AF_A1_f		= 0;
	
//	für SV_PWM()	
	volatile	float	PWMu1, PWMv1, PWMw1;
	volatile	float	PWMu2, PWMv2, PWMw2;
	
//	für Funktion: MOTOR_DREHT()
	volatile	int		MOTOR_DREHT_CNT = 0;							// Anzahl Aufrufe
	volatile	int		M_STEP_Z	=0;
	volatile	float	DWEy		= 0.0;
	volatile	float	DWE_a1		= 0.0;								// alter DWE-Wert

	volatile	float	DWEx		= 0.0;
	volatile	float	DWE_a2		= 0.0;								// alter DWE-Wert
	
	static		int		PS			= 1;								// Programmsequenz
		
			
//	für ADC_ISR()
//	Anzahl Aufrufe	
	
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
	volatile	float	ADC_POTI = 0.0f;								//

//	für Potis an den Analogeingängen A0, A1, A2
	volatile	int		ADC_A0_i = 0;									// Integer, Poti vom Stick für Sollpositionsvorgabe
	volatile	int		ADC_A1_i = 0;									// Integer, Poti HapStik vertikal
	volatile	int		ADC_A2_i = 0;									// Integer, Poti HapStik horizontal	
	
// SnapShot der ADC-Werte in der Print-Ausgabe erstellt	
	volatile	int		SS_ADC_A0_i;
	volatile	int		SS_ADC_A1_i;	
	volatile	int		SS_ADC_A2_i;	
	
	volatile	float	ADC_A0_f = 0.0f;								// Float, Poti vom Stick für Sollpositionsvorgabe
	volatile	float	ADC_A1_f = 0.0f;								// Float, Poti HapStik vertikal
	volatile	float	ADC_A2_f = 0.0f;								// Float, Poti HapStik horizontal

	volatile	float	nullY = 0.0f;									// ADC-Spannungswert in der vertikalen Sticknullposition 
	volatile	float	plusY_30 = 0.0f;								// ADC-Spannungswert in der vertikalen Stickobenposition 
	volatile	float	minusY_30 = 0.0f;								// ADC-Spannungswert in der vertikalen Stickuntenposition	

	volatile	float	nullX = 0.0f;									// ADC-Spannungswert in der vertikalen Sticknullposition
	volatile	float	plusX_30 = 0.0f;								// ADC-Spannungswert in der vertikalen Stickobenposition
	volatile	float	minusX_30 = 0.0f;								// ADC-Spannungswert in der vertikalen Stickuntenposition


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


	volatile	float	propfaktor_minusX_30 = 1.0;						// Proprtionalitätsfaktor für Kennliniennormierung auf -10000
	volatile	float	propfaktor_minusY_30 = 1.0;						// Proprtionalitätsfaktor für Kennliniennormierung auf -10000

	volatile	float	propfaktor_plusX_30 = 1.0;						// Proprtionalitätsfaktor für Kennliniennormierung auf -10000
	volatile	float	propfaktor_plusY_30 = 1.0;						// Proprtionalitätsfaktor für Kennliniennormierung auf -10000	
		








//	Übernehmen		
//	HS_Nr1
//	volatile	float	offset_winkel_y =		  4.0;
//	volatile	float	offset_winkel_x =		-16.0;
	
//	HS_Nr2
	volatile	float	offset_winkel_y =		30.0;					// Winkel_Offset [°] in für Poti_MOTOR1 Vertikal (Nullposition)
																		// -> "+" Stick wandert nach unten
	volatile	float	offset_winkel_x =		23.0;							// Winkel_Offset für Poti_MOTOR2 Horizontal	(Nullposition)
																		// -> "+" Stick wandert nach rechts






//	Array von Zeichen für Funktion: float_to_string(...)
	char	Ergebnis[20];

//	Leistungsfaktor	0 ... 1 Leistung | 0.5 -> 50% Leistung | 1.0 -> 100% Leistung
	volatile	float	LF_y	= 0.0;
	volatile	float	LF_x	= 0.0;

	volatile	float	LF_y_r	= 0.0;		// Ratsche
	volatile	float	LF_y_p	= 0.0;		// Position
	
		
//	für SVPWM()
	float	xvec_motorY, yvec_motorY, zvec_motorY;
	float	xvec_motorX, yvec_motorX, zvec_motorX;
		
//	für PI-REGLER()	
	volatile	float	ST = 0.0005;									// 1ms -> SampleTime ADC

//	PI-Regler für d
	volatile	float	Df = 0.0;										// Führungsgröße d

//	für print float
	#define STRING_EOL    "\r"
	#define STRING_HEADER "Test double to ascii conversion"

//	static double PRECISION = 0.00000000000001;
	static double PRECISION = 0.001;									//Anzahl Nachkommastellen, -> hier ohne Rundung!
	static int MAX_NUMBER_STRING_SIZE = 32;

	char s1[32];
	char s2[32];
	char s3[32];
	char s4[32];
	char s5[32];
		
//	* Double to ASCII prototype. Must be declared before usage in main function !!
	char * doubleToString(char *s, double n);
	

//	MOTORY	
//	PID_v_y Regler GESCHWINDIGKEIT
	float myInput1_1;
	
	float error1_1 = 0.0, input1_1 = 0.0, dInput1_1 = 0.0, lastInput1_1 = 0.0, mySetpoint_v_y = 0.0;
			
	double myOutput1_1;
	double ITerm1_1;
	double kp_v_y = 0.0;
	double ki_v_y = 0.0;
	double kd_v_y = 0.0;
	double outMax_v_y;
	double outMin1_1;
	

//	MOTORY	
//	PID_p_y Regler Position Y-Achse
	int myInput_p_y;
		
	float error2_1 = 0.0, input2_1 = 0.0, dInput2_1 = 0.0, lastInput2_1 = 0.0, mySetpoint_p_y = 0.0;

	double myOutput_p_y;
	double ITerm2_1;
	double kp_p_y = 0.0;
	double ki_p_y = 0.0;
	double kd_p_y = 0.0;
	double outMax_p_y;
	double outMin_p_y;
		

//	MOTORY	
//	PID_p_y_r Regler Position Y-Achse Ratsche
	int myInput_p_y_r;
		
	float error3_1 = 0.0, input3_1 = 0.0, dInput3_1 = 0.0, lastInput3_1 = 0.0, mySetpoint_p_y_r = 0.0;

	double myOutput_p_y_r;
	double ITerm3_1;
	double kp_p_y_r = 0.0;
	double ki_p_y_r = 0.0;
	double kd_p_y_r = 0.0;
	double outMax_p_y_r;
	double outMin_p_y_r;
	
	
//	MOTORX	
//	PID_v_x Regler GESCHWINDIGKEIT
	float myInput1_2;

	float error1_2 = 0.0, input1_2 = 0.0, dInput1_2 = 0.0, lastInput1_2 = 0.0, mySetpoint_v_x = 0.0;

	double myOutput1_2;
	double ITerm1_2;
	double kp_v_x = 0.0;
	double ki_v_x = 0.0;
	double kd_v_x = 0.0;
	double outMax_v_x;
	double outMin_v_x;

//	MOTORX	
//	PID_p_x Regler POSITION
	int myInput_p_x;

	float error2_2 = 0.0, input2_2 = 0.0, dInput2_2 = 0.0, lastInput2_2 = 0.0, mySetpoint_p_x = 0.0;

	double myOutput_p_x;
	double ITerm2_2;
	double kp_p_x = 0.0;
	double ki_p_x = 0.0;
	double kd_p_x = 0.0;
	double outMax_p_x;
	double outMin_p_x;

	

	volatile	int		SPRUNG_CNT		= 0;							// Anzahl Aufrufe
	volatile	int		FLAG_SWEEP 		= 0;							// Anzahl Aufrufe

//	* Ende PID Regler Global Variablen
	
	
//	ENDE **********************************************     Global Variables       *******************************************





//	ANFANG **********************************************     FUNKTIONEN     *************************************************


//	MOTORY Geschwindigkeit
//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    REGLER_v_y  xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void PID_v_y(void)
{
	  input1_1 = myInput1_1;
      error1_1 = mySetpoint_v_y - input1_1;
      ITerm1_1+= (ki_v_y * error1_1);
      if(ITerm1_1 > outMax_v_y) ITerm1_1= outMax_v_y;
      else if(ITerm1_1 < outMin1_1) ITerm1_1= outMin1_1;
      dInput1_1 = (input1_1 - lastInput1_1);
 
// Reglerausgang berechnen
      double output1_1 = kp_v_y * error1_1 + ITerm1_1 - kd_v_y * dInput1_1;

// Überläufe kappen      
	  if(output1_1 > outMax_v_y) output1_1 = outMax_v_y;
      else if(output1_1 < outMin1_1) output1_1 = outMin1_1;
	  myOutput1_1 = output1_1;
	  
//	letzten Wert speichern
      lastInput1_1 = input1_1;
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    REGLER_v_y  xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx



//	MOTORX Geschwindigkeit
//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    REGLER_v_x  xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void PID_v_x(void)
{
	input1_2 = myInput1_2;
	error1_2 = mySetpoint_v_x - input1_2;
	ITerm1_2+= (ki_v_x * error1_2);
	if(ITerm1_2 > outMax_v_x) ITerm1_2= outMax_v_x;
	else if(ITerm1_2 < outMin_v_x) ITerm1_2= outMin_v_x;
	dInput1_2 = (input1_2 - lastInput1_2);
	
	// Reglerausgang berechnen
	double output1_2 = kp_v_x * error1_2 + ITerm1_2 - kd_v_x * dInput1_2;

	// Überläufe kappen
	if(output1_2 > outMax_v_x) output1_2 = outMax_v_x;
	else if(output1_2 < outMin_v_x) output1_2 = outMin_v_x;
	myOutput1_2 = output1_2;
	
	//	letzten Wert speichern
	lastInput1_2 = input1_2;
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    REGLER_v_x   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx



//	MOTORY Position_RATSCHE
//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    REGLER_p_y_r xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void PID_p_y_r(void)
{
	input3_1 = myInput_p_y_r;
    error3_1 = mySetpoint_p_y_r - input3_1;
    ITerm3_1+= (ki_p_y_r * error3_1);
    if(ITerm3_1 > outMax_p_y_r) ITerm3_1= outMax_p_y_r;
    else if(ITerm3_1 < outMin_p_y_r) ITerm3_1= outMin_p_y_r;
    dInput3_1 = (input3_1 - lastInput3_1);
 
// Reglerausgang berechnen
    double output3_1 = kp_p_y_r * error3_1 + ITerm3_1 - kd_p_y_r * dInput3_1;

// Überläufe kappen      
	if(output3_1 > outMax_p_y_r) output3_1 = outMax_p_y_r;
    else if(output3_1 < outMin_p_y_r) output3_1 = outMin_p_y_r;
	myOutput_p_y_r = output3_1;
	  
//	letzten Wert speichern
	lastInput3_1 = input3_1;
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    REGLER_p_y_r xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx



//	MOTORY Position
//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    REGLER_p_y  xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void PID_p_y(void)
{
	input2_1 = myInput_p_y;
    error2_1 = mySetpoint_p_y - input2_1;
    ITerm2_1+= (ki_p_y * error2_1);
    if(ITerm2_1 > outMax_p_y) ITerm2_1= outMax_p_y;
    else if(ITerm2_1 < outMin_p_y) ITerm2_1= outMin_p_y;
    dInput2_1 = (input2_1 - lastInput2_1);
 
// Reglerausgang berechnen
    double output2_1 = kp_p_y * error2_1 + ITerm2_1 - kd_p_y * dInput2_1;

// Überläufe kappen      
	if(output2_1 > outMax_p_y) output2_1 = outMax_p_y;
    else if(output2_1 < outMin_p_y) output2_1 = outMin_p_y;
	myOutput_p_y = output2_1;
	  
//	letzten Wert speichern
	lastInput2_1 = input2_1;
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    REGLER_p_y  xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx



//	MOTORX Position
//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    REGLER_p_x  xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void PID_p_x(void)
{
	input2_2 = myInput_p_x;
	error2_2 = mySetpoint_p_x - input2_2;
	ITerm2_2+= (ki_p_x * error2_2);
	if(ITerm2_2 > outMax_p_x) ITerm2_2= outMax_p_x;
	else if(ITerm2_2 < outMin_p_x) ITerm2_2= outMin_p_x;
	dInput2_2 = (input2_2 - lastInput2_2);
	
// Reglerausgang berechnen
	double output2_2 = kp_p_x * error2_2 + ITerm2_2 - kd_p_x * dInput2_2;

// Überläufe kappen
	if(output2_2 > outMax_p_x) output2_2 = outMax_p_x;
	else if(output2_2 < outMin_p_x) output2_2 = outMin_p_x;
	myOutput_p_x = output2_2;
	
//	letzten Wert speichern
	lastInput2_2 = input2_2;
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    REGLER_p_x  xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx



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



//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    Float -> String   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
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



//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx       UART        xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
/*	Einstellungen sind:
		- 115kBaud
		- 8N1
	Output:
		- z.B. Integer Variable (INT_VAR) plotten
		printf("Dies ist eine IntegerVariable:   %i\r\n", INT_VAR);
		
*/
static void configure_console(void)
	{
		const usart_serial_options_t uart_serial_options = {
			.baudrate = CONF_UART_BAUDRATE,
			.paritytype = CONF_UART_PARITY
		};
	/* Configure console UART. */
		sysclk_enable_peripheral_clock(CONSOLE_UART_ID);
		stdio_serial_init(CONF_UART, &uart_serial_options);
	}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx       UART        xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx



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

void SVPWM(float uum1, float uvm1, float uwm1, float uum2, float uvm2, float uwm2)
{
//	MotorY
	if (uum1 >= 0)
	{	if (uvm1 >= 0)
		{	PWMw1 = (1-uvm1-uum1)/2;
			PWMv1 = PWMw1 + uvm1;
			PWMu1 = PWMv1 + uum1;
		}
		else
		{	if (uwm1 >= 0)
			{	PWMv1 = (1-uum1-uwm1)/2;
				PWMu1 = PWMv1 + uum1;
				PWMw1 = PWMu1 + uwm1;
			}
			else
			{	PWMv1 = (1+uvm1+uwm1)/2;
				PWMw1 = PWMv1 - uvm1;
				PWMu1 = PWMw1 - uwm1;
			}
		}
	}
	else
	{	if (uvm1 >= 0)
		{	if (uwm1 >= 0)
			{	PWMu1 = (1-uwm1-uvm1)/2;
				PWMw1 = PWMu1 + uwm1;
				PWMv1 = PWMw1 + uvm1;
			}
			else
			{	PWMw1 = (1+uwm1+uum1)/2;
				PWMu1 = PWMw1 - uwm1;
				PWMv1 = PWMu1 - uum1;
			}
		}
		else
		{	PWMu1 = (1+uum1+uvm1)/2;
			PWMv1 = PWMu1 - uum1;
			PWMw1 = PWMv1 - uvm1;
		}
	}


//	MotorX
	if (uum2 >= 0)
	{	if (uvm2 >= 0)
		{	PWMw2 = (1-uvm2-uum2)/2;
			PWMv2 = PWMw2 + uvm2;
			PWMu2 = PWMv2 + uum2;
		}
		else
		{	if (uwm2 >= 0)
			{	PWMv2 = (1-uum2-uwm2)/2;
				PWMu2 = PWMv2 + uum2;
				PWMw2 = PWMu2 + uwm2;
			}
			else
			{	PWMv2 = (1+uvm2+uwm2)/2;
				PWMw2 = PWMv2 - uvm2;
				PWMu2 = PWMw2 - uwm2;
			}
		}
	}
	else
	{	if (uvm2 >= 0)
		{	if (uwm2 >= 0)
			{	PWMu2 = (1-uwm2-uvm2)/2;
				PWMw2 = PWMu2 + uwm2;
				PWMv2 = PWMw2 + uvm2;
			}
			else
			{	PWMw2 = (1+uwm2+uum2)/2;
				PWMu2 = PWMw2 - uwm2;
				PWMv2 = PWMu2 - uum2;
			}
		}
		else
		{	PWMu2 = (1+uum2+uvm2)/2;
			PWMv2 = PWMu2 - uum2;
			PWMw2 = PWMv2 - uvm2;
		}
	}

//	Ausgabe an den PWM-VController
	REG_PWM_CDTYUPD0 = (1 - PWMu1) * 2100;
	REG_PWM_CDTYUPD1 = (1 - PWMv1) * 2100;
	REG_PWM_CDTYUPD2 = (1 - PWMw1) * 2100;

	REG_PWM_CDTYUPD3 = (1 - PWMu2) * 2100;
	REG_PWM_CDTYUPD4 = (1 - PWMv2) * 2100;
	REG_PWM_CDTYUPD5 = (1 - PWMw2) * 2100;

//	A Duty Cycle Update, Übernahme der Register Enable PWM channels (S.1016)
//	- Register: PWM_SCUC (Sync Channel Update)
//	- es gibt nur ein Bit in diesem Register:UPDULOCK (Unlock synchronous channels update)
//	- wird es 1 gesetzt werden die Register für Duty Cycle ... übernommen

//	Ausgabe
	REG_PWM_SCUC = 0x00000001u;
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx      SVPWM       xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx



//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx       ADC_ISR        xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void ADC_Handler(void)
{		
//	Kommt alle 50us
//	Wird von der PWM Event Line getriggert
//	Impuls auf der EventLine wird mit jedem PWM-Referenzimpuls erzeugt (50us) -> siehe Init_PWM

//	LOKALE VARIABLEN hier deklarieren
		
//	Disable Interrupt (CH7 vom ADC)
//	REG_ADC_IDR = REG_ADC_IDR | 0x00000080u;
//	Disable ADC interrupt (im Register ISER1 vom NVIC, leider hier nicht im Angebot, ASF)
//	NVIC_DisableIRQ(ADC_IRQn);




//	DEBUG
//	Debug_Pin C.24= 1 -> C.24 = 0 -> DUE_Pin6
	REG_PIOC_SODR	= REG_PIOC_SODR		|	0x01000000u; 		// Ausgang C24 =1 (DUE_Pin6)
//	delay_us(2);
	REG_PIOC_CODR	= REG_PIOC_CODR		|	0x01000000u; 		// Ausgang C24 =0 (DUE_Pin6)
	



//	Hier Mittelwertbildung (20-fach) durchführen

//	Analogkanäle lesen 1x/50us -> Snapshot
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
				
		svpwm_int = 1;											// Flag setzen, damit Regelschleife mit 1ms läuft
																// -> zur Übergabe an Endlosschleife in MAIN
/*
//	PRINT
		printf(" A0:  %10d\r\n",	AF_A0_i);					//	nur den Average-Filter-Wert von ADC-Kanal A0 printen
		printf(" A1:  %10d\r\n",	AF_A1_i);					//	nur den Average-Filter-Wert von ADC-Kanal A1 printen
		printf(" A2:  %20d|\r\n\n", SS_ADC_A2_i); 
		delay_ms(500);
*/
	}
//	Enable Interrupt (CH7 vom ADC)
//	REG_ADC_IER = REG_ADC_IER | 0x00000080u;
//	Enable ADC interrupt, schreibt das Register ISER im NVIC (Nested Vector Interrupt Controller)
//	NVIC_EnableIRQ(ADC_IRQn);		
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx       ADC_ISR        xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx



//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx     Init PWM       xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
/*
	PWM (S.979)
			- Ausgabe eines Referenzsignals an PWMH3 (4. PWM-Kanal, Port: C.9):
				- 1us Impuls, Wiederholrate 100us:	
				- ..._______-___________________-___________________-___________________-______________
			- PWM-Kanäle sind auf low 		 
			  
			- PWM Kanäle:	- 0...2 -> center aligned
						  		- Periode 100us
							- 3		-> left	aligned
								- Länge 1us
						  
			|				|				|				|				|				|				|			
			FUNKTION		PWM-Channel		DUE-Pin			PORT			PERIPHERAL
			-------------------------------------------------------------------------------------------------
			X high			H0				35				C.3				B
			X low			L0				34				C.2				B
			Y high			H1				37				C.5				B
			Y low			L1				36				C.4				B
			Z high			H2				39				C.7				B
			Z low			L2				38				C.6				B
			Referenz high	H3				41				C.9				B
			Referenz low	L3				40				C.8				B	
			


Neu für Ansteuerung von 2 Motoren			
			|				|				|				|				|				|				|
			FUNKTION		PWM-Channel		DUE-Pin			PORT			PERIPHERAL
			-------------------------------------------------------------------------------------------------

			Motor_1			
			X low			L0				34				C.2				B
			Y low			L1				36				C.4				B
			Z low			L2				38				C.6				B
			
			Motor_2
			X low			L3				40				C.8				B
			Y low			L4				9				C.21			B
			Z low			L5				8				C.22			B			
							
			Referenz low	L6				7				C.23			B			
*/



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


	Werte für 100us:
	REG_PWM_CPRD0 = REG_PWM_CPRD0	|	0x00001068u;		// 4200
	REG_PWM_CPRD1 = REG_PWM_CPRD1	|	0x00001068u;		// 4200	
	REG_PWM_CPRD2 = REG_PWM_CPRD2	|	0x00001068u;		// 4200

	REG_PWM_CPRD3 = REG_PWM_CPRD3	|	0x00001068u;		// 4200
	REG_PWM_CPRD4 = REG_PWM_CPRD4	|	0x00001068u;		// 4200
	REG_PWM_CPRD5 = REG_PWM_CPRD5	|	0x00001068u;		// 4200
	
	REG_PWM_CPRD6 = REG_PWM_CPRD6	|	0x000020D0u;		// 8400
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

//	Impuls auf der EventLine wird mit jedem 2. PWM-Referenzimpuls erzeugt	
//	REG_PWM_CMPM0 = REG_PWM_CMPM0	|	0x00000101u;

//	Impuls auf der EventLine wird mit jedem 3. PWM-Referenzimpuls erzeugt	
//	REG_PWM_CMPM0 = REG_PWM_CMPM0	|	0x00000201u;	

//	Impuls auf der EventLine wird mit jedem 4. PWM-Referenzimpuls erzeugt
//	REG_PWM_CMPM0 = REG_PWM_CMPM0	|	0x00000301u;

//	Impuls auf der EventLine wird mit jedem 5. PWM-Referenzimpuls erzeugt
//	REG_PWM_CMPM0 = REG_PWM_CMPM0	|	0x00000401u;	

//	Impuls auf der EventLine wird mit jedem 10. PWM-Referenzimpuls erzeugt (10 x 100us = 1ms)
//	REG_PWM_CMPM0 = REG_PWM_CMPM0	|	0x00000901u;

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
	Dead Time festlegen: 1us (S:1051)
	- Register: PWM_DT, die obere Registerhälfte enthält DTL und die untere Hälfte DTH
	- Dead Time = DTH/84MHz x 42 = 500ns -> 0x002A002Au;
	  oder:
	- Dead Time = DTH/84MHz x 84 = 1us -> 0x00540054u;

	REG_PWM_DT0 = 0x00540054u;
	REG_PWM_DT1 = 0x00540054u;		
	REG_PWM_DT2 = 0x00540054u;
	-> keine Dead time für DS8313 nötig, da keine komplementären Ausgänge	
*/

	
/*A 
	Enable PWM channels (S.1007)
	- gleichzeitige Start von Kanal 0 (steht stellvertretend für Kanäle 0...5 und Kanal 6 (Referenzimpuls), damit alle Kanäle 
	  den gleichen zeitlichen Nullpunkt haben.
	- initiale Ausgabe und Freischaltung der PWMs für Motor_1 und Motor_2:
*/
	REG_PWM_ENA = REG_PWM_ENA	|	0x00000041u; 


}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx     Init PWM       xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx



//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx     Init GPIO      xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//	Ports: PC21, PC22, PC23, PC24 und PC25 als Outputs definieren
//	--------------------------------------------------------------------------------------------------------------------------

void	INIT_GPIO(void)
{
/*
1.	Pull-up Resistor Control (Pullup Widerstand am Pin zuschalten)
		- PIO_PUER (Pull-up Enable Register) enable Pullup
		- PIO_PUDR (Pull-up Disable Resistor) disable Pullup
		- PIO_PUSR (Pull-up Status Register), hier steht das Ergebnis
		- nach Reset -> all of the pull-ups are enabled, i.e. PIO_PUSR resets at the value 0x0.
			
2.	I/O Line or Peripheral Function Selection
		- PIO_PER (PIO Enable Register, S.633)
		- PIO_PDR (PIO Disable Register)
		- PIO_PSR (PIO Status Register), hier steht das Ergebnis
			- 0 -> pin is controlled by the corresponding on-chip peripheral selected in the PIO_ABSR (AB Select Register)
			- 1 -> pin is controlled by the PIO controller
		- nach Reset -> generally, the I/O lines are controlled by the PIO controller, i.e. PIO_PSR resets at 1.

		- PC21, PC22, PC23, PC24, PC25 sind I/O Lines -> 0x03E00000u
		-                   PC24, PC25 sind I/O Lines -> 0x03000000u -> die anderen werden für Ansteuerung Motor_2 benötigt
*/
	REG_PIOC_PER	= REG_PIOC_PER		|		0x03000000u;
//	REG_PIOC_PER	= REG_PIOC_PER		|		0x13E00000u; PC28 als Input
	
/*
3.	Output Control
		- wenn die Outputs vom PIO-Controller definiert werden (zugehörenden Bits in PIO_PSR sind 1)
		- PIO_OER (Output Enable Register) treibt den Pin
		- PIO_ODR (Output Disable Register) Pin wird nicht getrieben
		- PIO_OSR (Output Status Register), hier steht das Ergebnis
			- 0 -> I/O line is used as an input
			- 1 -> I/O line is driven by the PIO controller
		- PIO_SODR (Set Output Data Register)
		- PIO_CODR (Clear Output Data Register)
		- PIO_ODSR (Output Data Status Register), , hier steht das Ergebnis
			- 1 -> eine 1 am Ausgang
			- 0 -> eine 0 am Ausgang

		- Ports: PC21, PC22, PC23, PC24 und PC25 als Outputs definieren -> 0x03E00000u
*/
//	REG_PIOC_OER	 = REG_PIOC_OER		|		0x03E00000u;		
	REG_PIOC_OER	 = REG_PIOC_OER		|		0x03000000u;		// -> die anderen werden für Ansteuerung Motor_2 benötigt			
/*			
4.	Synchronous Data Output	(S.623)		
		- nach Reset -> the synchronous data output is disabled on all the I/O lines as PIO_OWSR resets at 0x0.

5.	Multi Drive Control (Open Drain)
		- nach Reset -> the Multi Drive feature is disabled on all pins, i.e. PIO_MDSR resets at value 0x0.

	Zum Debuggen -> 5 Pins ein- und ausschalten:
	
		- PIO Controller Set   Output Data Register (S.642)	
		- PIO Controller Clear Output Data Register (S.643)

	REG_PIOC_SODR	= REG_PIOC_SODR		|	0x03E00000u; 		// Ausgänge C21, C22, C23, C24, C25 auf 1 setzen	
	REG_PIOC_CODR	= REG_PIOC_CODR		|	0x03E00000u; 		// Ausgänge C21, C22, C23, C24, C25 auf 0 setzen	

	REG_PIOC_SODR	= REG_PIOC_SODR		|	0x0200 0000u; 		// Ausgang C25 =1 (DUE_Pin 5)	
	REG_PIOC_CODR	= REG_PIOC_CODR		|	0x0200 0000u; 		// Ausgang C25 =0 (DUE_Pin 5)	
	
	REG_PIOC_SODR	= REG_PIOC_SODR		|	0x0100 0000u; 		// Ausgang C24 =1 (DUE_Pin 6)
	REG_PIOC_CODR	= REG_PIOC_CODR		|	0x0100 0000u; 		// Ausgang C24 =0 (DUE_Pin 6)	


->  diese hier sind nicht mehr verfübar -> werden für Ansteuerung Motor_2 benötigt	
	REG_PIOC_SODR	= REG_PIOC_SODR		|	0x0080 0000u; 		// Ausgang C23 =1 (DUE_Pin 7)
	REG_PIOC_CODR	= REG_PIOC_CODR		|	0x0080 0000u; 		// Ausgang C23 =0 (DUE_Pin 7)
	
	REG_PIOC_SODR	= REG_PIOC_SODR		|	0x0040 0000u; 		// Ausgang C22 =1 (DUE_Pin 8)
	REG_PIOC_CODR	= REG_PIOC_CODR		|	0x0040 0000u; 		// Ausgang C22 =0 (DUE_Pin 8)
	
	REG_PIOC_SODR	= REG_PIOC_SODR		|	0x0020 0000u; 		// Ausgang C21 =1 (DUE_Pin 9)
	REG_PIOC_CODR	= REG_PIOC_CODR		|	0x0020 0000u; 		// Ausgang C21 =0 (DUE_Pin 9)
*/
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx     Init GPIO      xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx



//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx     Init ADC       xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
/*	
	Analoge Eingänge: (S.1320)
	- DUE: A0	SAM: A16		ADC: AD7
	- DUE: A1	SAM: A24		ADC: AD6		-> HS, vertikal ?
	- DUE: A2	SAM: A23		ADC: AD5		-> HS, horizontal ?
	Ergebnisse:
	- REG_ADC_CDR[5]
	- REG_ADC_CDR[6]
	- REG_ADC_CDR[7]
	Start (Software, zunächst):
	- REG_ADC_CR = 0x0000 0002;
	S&H ab Start-Kommando:
	- 476ns
	- 1666ns
	- 2856ns
*/	

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

/*	ADC Mode (S.1333)
	- Register ADC_MR (Mode Register)
		- Hardware_Trigger enable -> PWM Event Line 0
			- ADC_TRIG4 (PWM Event Line 0)
		- 12Bit
		- Sleep aus (ADC zwischen den Wandlungen immer aktiv)
		- Freerun aus -> Normal Mode
		- elektrische Daten ADC auf S.1403
			- max. fADC =22MHz -> 84MHz/4=21MHz -> PRESCAL=1
			- StartUp Time 0, weil ADC immer aktiv -> STARTUP=0
			- Settling Time min. 200ns -> 4,2 ADC-Clockperioden -> SETTLING = 1 (5 periods)
		- kein Wechsel der analogen Einstellungen bei Kanalwechsel -> ANACH = 0
		- TrackTime (wahrscheinlich Einschwingen S&H) zunächst auf 9 -> 9+1 = 10 ... 10 * Periode ADC (47,6ns) = 476ns, -> TRACTIM=9
		- TransferTime (1*2+3)*Periode ADC (47,6ns) = 238ns
			-	1.S&H -> 476ns
				2.S&H -> 476ns  + 1190ns = 1666ns
				3.S&H -> 1666ns + 1190ns = 2856ns und im weiteren immer jeweils nach: ConvTime + TransferTime = 952ns + 238 = 1190ns 	
			-	Gemessen habe ich in Summe, vom Start bis Debug_Pin in ADC_Handler() =1 setzen -> 3660 ns
				-> passt recht ordentlich!
		- USEQ = 0, keine bestimmte Kanalsequenz, Normal Mode: The controller converts channels in a simple numeric order.	
		
		-> ADC_MR = 1E900209H				   

	- Zusammenfassung der Parameter: (neu definiert 06.10.2016)
	- 15kOhm	Quellwiderstand
	- 14MHz		ADC clock
	- 71,43ns	ADCClock period
	- 1,071us	Tracktime
	- 357,15ns	TRANSFER period
	- 357,15ns	SETTLING period (Einschwingen beim Übergang zum nächsten ADC-Kanal, z.B. andere Verstärkung, etc.
*/								
//	REG_ADC_MR = REG_ADC_MR			| 0x19100100u;				// Software_Trigger
//	REG_ADC_MR = REG_ADC_MR			| 0x19100109u;				// Hardware_Trigger -> PWM Event Line 0 
//	REG_ADC_MR = REG_ADC_MR			| 0x1E900209u;				// neu definiert 06.10.2016
	

		
//	TEST
//	2k + 10k als Spannungsteiler direkt über die Referenz gelötet und den Abgriff auf A0 in der Leiste eingespeist
//	Steckbrücke über A0 und A1 (beide sehen das selbe)
//	PGA: x1
	 
//	REG_ADC_MR = REG_ADC_MR			| 0x1E900209u;				// AUSGANGSSITUATION -> neu definiert 06.10.2016

//	REG_ADC_MR = REG_ADC_MR			| 0x1F900209u;				// TRACKTIM	(15 + 1) * 71,4 = 1142ns
																// -> keine Änderung ADC-Hub: 4 
//	REG_ADC_MR = REG_ADC_MR			| 0x10900209u;				// TRACKTIM	(0  + 1) * 71,4 = 71,4ns
																// -> keine Änderung ADC-Hub: 4 
//	REG_ADC_MR = REG_ADC_MR			| 0x3E900209u;				// TRANSFER von 1 auf 3 erhöht (max.) 
																// -> keine Änderung ADC-Hub: 4 
//	REG_ADC_MR = REG_ADC_MR			| 0x1EB00209u;				// SETTLING von 1 auf 3 erhöht (max.) 
																// -> keine Änderung ADC-Hub: 4 
//	REG_ADC_MR = REG_ADC_MR			| 0x1E9F0209u;				// STARTUP von 0 auf 15 erhöht (max.) 
																// -> keine Änderung ADC-Hub: 4 

//	REG_ADC_MR = REG_ADC_MR			| 0x1E90FF09u;				// PRESCAL von 2 auf 15 erhöht (max./2) 
																// -> keine Änderung ADC-Hub: 4 

//	REG_ADC_MR = REG_ADC_MR			| 0x3FBFFF09u;	



//	Einspeisung konstanter Wert: 2k + 10k als Spannungsteiler direkt über die Referenz gelötet und den Abgriff auf A0
//	in die DUE-Leiste eingespeist
//	2,2uF Tantal-Kondensator über AVREF (MAX 6066 -> 2,5V)
//	PRINT_VERT_cnt: > 2000


//	10 Bit / x1  -> ADC-Werte: 2
//	REG_ADC_MR = REG_ADC_MR			| 0x1E900219u;				// LOWRES von 0 auf 1 (12Bit -> 10Bit) 


//	10 Bit / x2  -> ADC-Werte: 3
//	REG_ADC_MR = REG_ADC_MR			| 0x1E900219u;				// LOWRES von 0 auf 1 (12Bit -> 10Bit) 


//	10 Bit / x4  -> ADC-Werte: 6 (731 ... 736)
//	REG_ADC_MR = REG_ADC_MR			| 0x1E900219u;				// LOWRES von 0 auf 1 (12Bit -> 10Bit) 


//	-> DAS BEDEUTET, dass die Änderung des Signals schon vor dem PGA stattfinden muss
//	   Vorausgesetzt natürlich, dass der PGA keinen zusätzlichen Fehler mit rein bringt!





//	12 Bit / x1  -> ADC-Werte: 8 (723 ... 730)
//	Verteilung VW_MW = 727;
//	REG_ADC_MR = REG_ADC_MR			| 0x1E900209u;				// LOWRES von 1 auf 0 (10Bit -> 12Bit)	
//	|	723		|	724		|	725		|	726		|	727		|	728		|	729		|	730		|	731		|	732		|
//	|	2  		|	27 		|	557		|	1307	|	522		|	310		|	42 		|	1  		|	0  		|	0  		|



//	12 Bit / x4  -> ADC-Werte: 21 (2926 ... 2946)
//	Verteilung VW_MW = 2936;

	REG_ADC_MR = REG_ADC_MR			| 0x1E900209u;				// LOWRES von 1 auf 0 (10Bit -> 12Bit)
//	12 Bit / x4  -> ADC-Werte: 10 (2931 ... 2940) nach 1000 Messungen


	
//	REG_ADC_MR = REG_ADC_MR			| 0x3FBFFF09u;				// allet max
//	12 Bit / x4  -> ADC-Werte: 13 (2930 ... 2942) nach 1000 Messungen

//				-> ADC-Werte: 21 (2926 ... 2946)		
//	|	2932	|	2933	|	2934	|	2935	|-	2936   -|	2937	|	2938	|	2939	|	2940	|	2941	|
//	|	38 		|	226		|	749		|	1434	|	1398	|	826		|	336		|	80 		|	18 		|	3  		|


//	-> DAS BEDEUTET auch hier:
//		- dass die Änderung des Signals schon vor dem PGA stattfinden muss
//		- UND, dass die Veränderung der ADC-Parameter, auf "allet max" (siehe oben) auch keine Verbesserung bringt
//		-> damit ist z.B. die Tracktime nicht zu knapp bemessen, etc. ... !!! und 
//		   REG_ADC_MR = REG_ADC_MR			| 0x1E900209u; vollkommen in Ordnun

/*	Kanäle enable
		- Register: ADC_CHER (ADC Channel Enable Register), S.1338
		- Kanäle Ch5, CH6, CH7 erlauben
		-> ist im ADC_CHSR (ADC Channel Status Register) zu sehen		
*/
	REG_ADC_CHER = REG_ADC_CHER		| 0x000000E0u;

/*	Kanäle Verstärkung
		- Register: ADC_CGR (ADC Channel Gain Register), S.1349

	REG_ADC_CGR = REG_ADC_CGR		| 0xFFFFFFFFu;		//Gain = 4 für alle Kanäle
	REG_ADC_CGR = REG_ADC_CGR		| 0x00003C00u;		//Gain = 4 für Kanäle: Ch5, CH6	
	REG_ADC_CGR = REG_ADC_CGR		| 0x0000FC00u;		//Gain = 4 für Kanäle: Ch5, CH6, CH7

	ACHTUNG: bei neuem ADC-Kanal hier noch einmal genau die Zuordnung prüfen:
	Das hier:
	REG_ADC_CGR = REG_ADC_CGR		| 0x00003C00u;		//Gain = 4 für Kanäle: Ch5, CH6	
	... haut nicht hin
	Nur wenn alle Kanäle mit x4 eingestellt werden, erwische ich CH5 und CH6, also die HS-Potis, um die es hier geht
	Tschuldigung, habe jetzt hier keine Zeit (... eher keine Lust ;-) das zu prüfen
	06.10.2016
	Ist geklärt. Entscheidendist das Bit ANACH. Es muss 1 sein, wenn man zwischen den Kanälen z.B. die Verstärkung
	wechselt. Ansonsten werden die Parameter von CH0 für alle Kanäle ünernommen (S.1326). Das ist die Erklärung.
	Achtung, wenn ANACH = 1, dann 
	
*/
	REG_ADC_CGR = REG_ADC_CGR		| 0x0000F000u;		// Gain = 1 für CH5 (A2) -> Poti zur Spollwertvorgabe
														// Gain = 4 für CH6 (A1)
														// Gain = 4 für CH7 (A0)
														// Gain = 1	für restliche Analogeingänge 													

//	REG_ADC_CGR = REG_ADC_CGR		| 0xFFFFFFFFu;		// Gain = 4 für alle Kanäle
//	REG_ADC_CGR = REG_ADC_CGR		| 0x22222222u;		// Gain = 2 für alle Kanäle
//	REG_ADC_CGR = REG_ADC_CGR		| 0x00000000u;		// Gain = 1 für alle Kanäle

//	Kanäle Offset -> nicht benutzt
	
/*	Conversion Result der letzten Wandlung
		- Register: ADC_LCDR (ADC Last Converted Data Register), S.1341
	REG_ADC_LCDR	->   0x0000 | Kanalnummer 4Bit | DATA 12Bit |

	Start Conversion ADC -> nur bei Software_Trigger enable (S.1332)
	REG_ADC_CR = 0x00000002;

	Conversion Result kanalbezogen (untere 12 Bit), S.1351
		- Register: ADC_CDRx (ADC Channel Data Register)
		- *CH_A0
		- *CH_A1
		- *CH_A2

	Ablauf:
			
//	Start Conversion ADC (S.1332), nur bei Software_Trigger
	REG_ADC_CR = 0x00000002;

//	Auf ADC-End of conversion (EOC) testen (Hier wird nur auf CH7 fertig geprüft, weil der ADC fortlaufend, ab CH5 zu wandeln beginnt
//	und mit CH7 endet.
//	Alle 3 EOCs prüfen
//		if ( (REG_ADC_ISR && 0x0000080u) && (REG_ADC_ISR && 0x0000040u) && (REG_ADC_ISR && 0x0000020u) != 0) {
//	Nur das oberste EOC, vom Kanal 7, prüfen
		if (REG_ADC_ISR && 0x0000080u != 0) 
		{					
//	Zweipunkteform der Geraden
			U_A0 = (((3.2715 - 0.005) / (4095.0 - 0.005)) * (REG_ADC_CDR7 - 0.005)) + 0.005;
			U_A1 = (((3.2715 - 0.005) / (4095.0 - 0.005)) * (REG_ADC_CDR6 - 0.005)) + 0.005;
			U_A2 = (((3.2715 - 0.005) / (4095.0 - 0.005)) * (REG_ADC_CDR5 - 0.005)) + 0.005;
		}
*/
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx     Init ADC       xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx


	
//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx  PRINT_START       xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void	PRINT_START(void)
{
	printf("|-----------------------------------------------------------------------------------------------------------------------------------|\r\n");
	printf("|HAPSTIK 22.01.2017     Potiwerte an Max und Min-Positionen / Proportionalitätsfaktoren / xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx|\r\n");
	printf("|-----------------------------------------------------------------------------------------------------------------------------------|\r\n\n");
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx  PRINT_START       xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

//	ENDE ************************************************     FUNKTIONEN     *************************************************



//	**************************************************************************************************************************
//	ANFANG *******************************************          MAIN           ***********************************************
//	**************************************************************************************************************************
int main(void)
{
//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx     Init SAM system     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx	
	sysclk_init();
	board_init();
	configure_console();
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx     Init SAM system     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	INIT_PWM();
	INIT_GPIO();
	INIT_ADC();

//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx     Interrupt      xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
/*
	Wir wollen das End of Conversion des CH7 vom ADC (der 3. gewandelte Kanal für die Stromwerte) interruptfähig machen
	und in einer Interrupt Service Routine (ISR) alle weiteren Berechnungen durchführen.
		-  n	* 100us -> ISR
		- (n+1)	* 100us -> ISR
		- ...			
	- Instance ADC:	37
	- ADC Interrupt erlauben (S.1342)
		- Register: ADC_IER (ADC Interrupt Enable Register)
		- EOC7 erlauben (CH7), der letzte Kanal, der gewandelt wurde 
*/
	REG_ADC_IER = REG_ADC_IER | 0x00000080u;	

//	Aus dem main-init-Code zur korrekten De-/Aktivierung von Interrupts.
//	Hier aus dem Beispielprojekt "TC Capture Waveform" vom ASF Framework:

	NVIC_DisableIRQ(ADC_IRQn);
	NVIC_ClearPendingIRQ(ADC_IRQn);
	NVIC_SetPriority(ADC_IRQn, 0);
//	Enable ADC interrupt, schreibt das Register ISER im NVIC (Nested Vector Interrupt Controller)
	NVIC_EnableIRQ(ADC_IRQn);
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx     Interrupt      xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx



//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx   START                 xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	PRINT_START();

 /*	HS:
				 -Y
				 |
			-X---|---+X
				 |
				 +Y
*/

	TD1 = 2;													// 2ms/1° -> Zeitverzögerung1
	TD2 = 500;													// 500ms am Endpunkt
	
//	--------------------------------------------------------------------------------------- Kalibrierung der Winkel zueinander
//	Voraussetzung: 
//		- Stick muss sich leicht drehen lassen
//		- Stick muss sich etwa in Feldmitte befinden

	

//	--------------------------------------------------------------------------------------------------- Stick in Position NULL
//	Leistungsfaktoren (0...1) MOTOR1, MOTOR2
	LF_y = 0.25;
	LF_x = 0.25;
	
//	Zunächst einmal den Winkeloffset für Nullstellung ermitteln
	DWEy = offset_winkel_y * 7;										// Winkeloffset in °
	DWEx = offset_winkel_x * 7;
		
//	Winkelanteile für Winkeloffset mit Berücksichtigung des Leistungsfaktors berechnen
	xvec_motorY = LF_y * cos(DWEy*WK1);
	yvec_motorY = LF_y * cos(DWEy*WK1-WK2);
	zvec_motorY = LF_y * cos(DWEy*WK1-WK3);
	
	xvec_motorX = LF_x * cos(DWEx*WK1);
	yvec_motorX = LF_x * cos(DWEx*WK1-WK2);
	zvec_motorX = LF_x * cos(DWEx*WK1-WK3);
	
//	Raumvektorausgabe -> Stick steht in Nullstellung	
	SVPWM(xvec_motorY, yvec_motorY, zvec_motorY, xvec_motorX, yvec_motorX, zvec_motorX);

//	Stick beruhigen lassen
	delay_ms(500);





//	Test
	a1_flt = 10.7; 
	a2_flt = 10.3; 

	a1_int = a1_flt;				// 10,7	-> 10	Zuweisung float an int -> Zuweisung des Ganzzahlanteils
	a2_int = a2_flt;				// 10,3	-> 10
//	---
	a1_int = 23;					// Es wird nicht gerundet, auch hier Zuweisung des Ganzzahlanteils
	a2_int = 10;
	a3_int = a1_int / a2_int;		// 2,3	-> 2
//	---
	a1_int = 27;
	a2_int = 10;
	a3_int = a1_int / a2_int;		// 2,7	-> 2






//	MOTOR_Y
//	---------------------------------------------------------------------------------------------------- Stick in Position +Y
//	RUNTER

	DWE_a1 = 7 * offset_winkel_y;						// Offset des Sticks (HS-spezifisch)
	
//	Schleifendurchläufe 30 (von 0° nach +30°)	
	LF_y = 0.5;									// Leistungsfaktor (0...1)
	for (a=1; a<=30; a++) 
	{	if (a == 30)							// wenn Stick in Zielposition dann max. Power zum "Festhalten"
		{	LF_y = 1.0;
		}
		DWEy = DWE_a1 + 7 * a * 1;				// Winkel pro Step = 1°
		xvec_motorY = LF_y * cos(DWEy*WK1);
		yvec_motorY = LF_y * cos(DWEy*WK1-WK2);
		zvec_motorY = LF_y * cos(DWEy*WK1-WK3);
		SVPWM(xvec_motorY, yvec_motorY, zvec_motorY, 0, 0, 0);
		delay_ms(TD1);
	} // for (a=1; a<=30; a++) 
	DWE_a1 = DWEy;								// DWE merken
	delay_ms(TD2);								// Stick beruhigen lassen

	plusY_30 = AF_A0_i;							// Poti_ADC_Wert für Position +Y lesen
	

//	----------------------------------------------------------------------------------------------------- Stick in Position -Y
//	HOCH
//	Schleifendurchläufe 60 (von +30° nach -30°)

	LF_y = 0.5;									// Leistungsfaktor (0...1)
	for (a=1; a<=60; a++) 
	{	if (a == 60)							// wenn Stick in Zielposition dann max. Power zum "Festhalten"
		{	LF_y = 1.0;
		}
		DWEy = DWE_a1 + 7 * a * -1;				//Winkel pro Step = 1°
		xvec_motorY = LF_y * cos(DWEy*WK1);
		yvec_motorY = LF_y * cos(DWEy*WK1-WK2);
		zvec_motorY = LF_y * cos(DWEy*WK1-WK3);
		SVPWM(xvec_motorY, yvec_motorY, zvec_motorY, 0, 0, 0);
		delay_ms(TD1);
	} // for (a=1; a<=60; a++) 
	DWE_a1 = DWEy;
	delay_ms(TD2);								// Stick beruhigen lassen

	minusY_30 = AF_A0_i;						// Poti_ADC_Wert für Position -Y lesen


//	--------------------------------------------------------------------------------------------------- Stick in Position NULL
//	MITTE
//	Schleifendurchläufe 30 (von -30° nach 0°)

	LF_y = 0.5;									// Leistungsfaktor (0...1)
	for (a=1; a<=30; a++) 
	{	if (a == 30)							// wenn Stick in Zielposition dann max. Power zum "Festhalten"
		{	LF_y = 1.0;
		}
		DWEy = DWE_a1 + 7 * a * 1;				//Winkel pro Step = 1°
		xvec_motorY = LF_y * cos(DWEy*WK1);
		yvec_motorY = LF_y * cos(DWEy*WK1-WK2);
		zvec_motorY = LF_y * cos(DWEy*WK1-WK3);
		SVPWM(xvec_motorY, yvec_motorY, zvec_motorY, 0, 0, 0);
		delay_ms(TD1);
	} // for (a=1; a<=30; a++) 
	DWE_a1 = DWEy;
	delay_ms(TD2);								// Stick beruhigen lassen

	nullY = AF_A0_i;							// Poti_ADC_Wert für Position NULL Y lesen



//	MOTOR_X
//	----------------------------------------------------------------------------------------------------- Stick in Position +X
//	RECHTS
//	Schleifendurchläufe 30 (von 0° nach +30°)

	DWE_a2 = 7 * offset_winkel_x;						// Offset des Sticks (installationsspezifisch)

	LF_x = 0.5;									// Leistungsfaktor (0...1)
	for (a=1; a<=30; a++)
	{	if (a == 30)							// wenn Stick in Zielposition dann max. Power zum "Festhalten"
		{	LF_x = 1.0;
		}
		DWEx = DWE_a2 + 7 * a * 1;				// Winkel pro Step = 1°
		xvec_motorX = LF_x * cos(DWEx*WK1);
		yvec_motorX = LF_x * cos(DWEx*WK1-WK2);
		zvec_motorX = LF_x * cos(DWEx*WK1-WK3);
		SVPWM(xvec_motorY, yvec_motorY, zvec_motorY, xvec_motorX, yvec_motorX, zvec_motorX);
		delay_ms(TD1);
	}
	DWE_a2 = DWEx;								// DWE merken
	delay_ms(TD2);								// Stick beruhigen lassen

	plusX_30 = AF_A1_i;							// Poti_ADC_Wert für Position +X lesen

//	----------------------------------------------------------------------------------------------------- Stick in Position -X
//	Schleifendurchläufe 60 (von +30° nach -30°)

	LF_x = 0.5;									// Leistungsfaktor (0...1)
	for (a=1; a<=60; a++)
	{	if (a == 60)							// wenn Stick in Zielposition dann max. Power zum "Festhalten"
		{	LF_x = 1.0;
		}
		DWEx = DWE_a2 + 7 * a * -1;				// Winkel pro Step = 1°
		xvec_motorX = LF_x * cos(DWEx*WK1);
		yvec_motorX = LF_x * cos(DWEx*WK1-WK2);
		zvec_motorX = LF_x * cos(DWEx*WK1-WK3);
		SVPWM(xvec_motorY, yvec_motorY, zvec_motorY, xvec_motorX, yvec_motorX, zvec_motorX);
		delay_ms(TD1);
	}
	DWE_a2 = DWEx;
	delay_ms(TD2);								// Stick beruhigen lassen

	minusX_30 = AF_A1_i;						// Poti_ADC_Wert für Position -X lesen

//	--------------------------------------------------------------------------------------------------- Stick in Position NULL
// Schleifendurchläufe 30 (von -30° nach 0°)

	LF_x = 0.5;									// Leistungsfaktor (0...1)
	for (a=1; a<=30; a++)
	{	if (a == 30)							// wenn Stick in Zielposition dann max. Power zum "Festhalten"
		{	LF_x = 1.0;
		}
		DWEx = DWE_a2 + 7 * a * 1;				// Winkel pro Step = 1°
		xvec_motorX = LF_x * cos(DWEx*WK1);
		yvec_motorX = LF_x * cos(DWEx*WK1-WK2);
		zvec_motorX = LF_x * cos(DWEx*WK1-WK3);
		SVPWM(xvec_motorY, yvec_motorY, zvec_motorY, xvec_motorX, yvec_motorX, zvec_motorX);
		delay_ms(TD1);
	}
	DWE_a2 = DWEx;
	delay_ms(TD2);								// Stick beruhigen lassen

	nullX = AF_A1_i;							// Poti_ADC_Wert für Position NULL Y lesen// Poti2 lesen


	
//	PRINT
	printf("| minusX_30     : %15s| nullX         : %15s| plusX_30     : %15s|\r\n",
	doubleToString(s1, minusX_30), doubleToString(s2, nullX), doubleToString(s3, plusX_30));
	
	printf("| minusY_30     : %15s| nullY         : %15s| plusY_30     : %15s|\r\n\n",
	doubleToString(s1, minusY_30), doubleToString(s2, nullY), doubleToString(s3, plusY_30));



	SVPWM(0,0,0,0,0,0);
	delay_ms(TD1);







//	--------------------------------------------------------------------------------------- Kennlinie in Nullpunkt verschieben
//	-> Nullpunkt wird 0	
	minusX_30	= minusX_30 - nullX;
	plusX_30	= plusX_30  - nullX;	
//	nullX		= 0;

	minusY_30	= minusY_30 - nullY;
	plusY_30	= plusY_30  - nullY;	
//	nullY		= 0;	
	
//	PRINT
	printf("| minusX_30     : %15s| plusX_30      : %15s|\r\n",
	doubleToString(s1, minusX_30), doubleToString(s3, plusX_30));
	
	printf("| minusY_30     : %15s| plusY_30      : %15s|\r\n\n\n",
	doubleToString(s1, minusY_30), doubleToString(s3, plusY_30));
	


//	--------------------------------------------------------------- Proportinalitätsfaktoren für "-" und "+"Kennlinienbereiche
//	Proportionalitätsfaktor für +30° -> +1500 und -30° -> -1500

	propfaktor_minusX_30	= 1363.6363 / minusX_30;				// Referenzwerte wurden für 30° ermittelt 
	propfaktor_plusX_30		= 1363.6363 / plusX_30;					// 1500 Normwert soll aber 30° Auslenkung entsprechen
																	// -> 30 / 33 * 1500 = 1363,6363

	propfaktor_minusY_30	= 1363.6363 / minusY_30;
	propfaktor_plusY_30		= 1363.6363 / plusY_30;
		
//	PRINT
	printf("| propfaktor_mX : %15s| propfaktor_pX : %15s|\r\n",
	doubleToString(s1, propfaktor_minusX_30), doubleToString(s2, propfaktor_plusX_30));	

	printf("| propfaktor_mY : %15s| propfaktor_pY : %15s|\r\n\n",
	doubleToString(s1, propfaktor_minusY_30), doubleToString(s2, propfaktor_plusY_30));





//	----------------------------------------------------------------------------------------------------------- Endlosschleife
	while (1)
	{		
//	Für synchrone Vorgänge passend zum ADC-Interrupt -> svpwm_int wird in der Funktion ADC_ISR -> 1 gesetzt (1ms Zyklus)
		if (svpwm_int == 1)
		{
//	Flag zurück setzen
			svpwm_int = 0;		
			TAE_1ms++;
			SEQUENZ_1ms++;			// 1ms zählen	





	
	
//	1ms ----------------------------------------------------- REGLELUNG -------------------------------------------------------
//	---------------------------------------------------------------------------------------------------------------------------
/*
	Parameter:	
											mySetpoint_p_y:

												[-1500]
												   -Y
									   	
												   |
												   |
												   |
												   |
		mySetpoint_p_x:	 [-1500]	-X  ----------------------- +X [+1500] 
												   |
												   |
												   |
												   |
												   +y	
												[+1500]
*/


//	SPIRALE
//	***** ANFANG *****
//	2x p-Regler:
//		- Regler Y-Achse	Soll: mySetpoint_p_y	IST: myInput_p_y
//		- Regler X-Achse	Soll: mySetpoint_p_x	IST: myInput_p_x

//	Input Regler		Amplitude	sin(Winkel in Grad)	
//	mySetpoint_p_x	=	800	*	SP_A	*	sin( SP_W			*	WK1);
	mySetpoint_p_x	=	1000	*	SP_A	*	sin( SP_W			*	WK1);
	mySetpoint_p_y	=	1000	*	SP_A	*	sin((SP_W + 90)		*	WK1);
	
	SP_W	= SP_W	+	SP_DW;														// nach 0,5s -> 1 Umdrehung
	SP_A	= SP_A	+	0.0001	*	VZ;												// nach 5s -> Amplitude = 1
	

	if (TAE_1ms % 10000 == 0)														// nach 10s 
	{	VZ = -VZ;
	}
	if (TAE_1ms == 20000)															// nach 20s 
	{	SP_DW = 3.6;	
	}	
	if (TAE_1ms == 40000)															// nach 40s 
	{	SP_DW = 0.09;	
	}	
	

//	---------------------------------------------------------------------------------------------------------------------------
//	REGLER Y-ACHSE
//	***** ANFANG *****
//	---------------------------------------------------------------------------------------------------------------------------
//	Sollwert:
//	mySetpoint_p_y kommt aus Funktion für SPIRALE
		
//	Istwert:
//	myInput_p_y kommt vom Analogeingang A0 (aktuelle Stickposition Y) normieren
	myInput_p_y = AF_A0_i - nullY;													// Nullpunktverschiebung
	if (myInput_p_y >= 0)															// Proportionalitätsfaktor für Anstiegsnormierung
	{	myInput_p_y =			myInput_p_y * propfaktor_plusY_30;					// getrennt für plus- und minus-Bereich
	}
		myInput_p_y = (-1)	*	myInput_p_y * propfaktor_minusY_30;

//	Regleroutput-Maxwerte:
	outMax_p_y		=  1.0;							// Max Drehmoment
	outMin_p_y		= -1.0;							// Min Drehmoment

//	Reglerparameter:
	kp_p_y			= 0.002;						// harte Charakteristik
//	ki_p_y			= 0.0;
	kd_p_y			= 0.0035;
	
//	Regler rechnen:
	PID_p_y();

//	Stellgröße:	
	LF_y = myOutput_p_y;							// Zuweisung des Regler-Outputs zum Leistungsfaktor-Position 
//	***** ENDE *****




//	MOTORANSTEUERUNGS-WERTE Y-ACHSE BERECHNEN
//	***** ANFANG *****
//	Wenn der Leistungsfaktor	-> positiv, dann -90° (elektrisch) Vektor raus geben
//								-> negativ, dann +90° (elektrisch) Vektor raus geben
	if (LF_y >= 0)
	{

//	Drehwinkel berechnen und dann 90° subtrahieren
//	1500 -> 33°, 500 -> 11°, ...
		DWEy = 7 * ((NOR1 * myInput_p_y) + offset_winkel_y) + 90;
								
//	Winkelanteile mit Berücksichtigung des Leistungsfaktors berechnen
		xvec_motorY =			LF_y * cos(DWEy*WK1);
		yvec_motorY =			LF_y * cos(DWEy*WK1-WK2);
		zvec_motorY =			LF_y * cos(DWEy*WK1-WK3);
	} // if (LF_y >= 0)
	if (LF_y < 0)
	{
//	Drehwinkel berechnen und dann 90° addieren
		DWEy = 7 * ((NOR1 * myInput_p_y) + offset_winkel_y) - 90;
				
//	Winkelanteile mit Berücksichtigung des Leistungsfaktors berechnen
		xvec_motorY = (-1)	*	LF_y * cos(DWEy*WK1);
		yvec_motorY = (-1)	*	LF_y * cos(DWEy*WK1-WK2);
		zvec_motorY = (-1)	*	LF_y * cos(DWEy*WK1-WK3);
	} // if (LF_y < 0)
//	***** ENDE *****



//	PRINT
//		printf("| AF_A0_i       : %15d| myInput2_1    : %15s| DWEy          : %15s| LF_y          : %15s|\r\n\n",
//		AF_A0_i, doubleToString(s1, myInput2_1), doubleToString(s2, DWEy), doubleToString(s3, LF_y));
	


//	---------------------------------------------------------------------------------------------------------------------------
//	REGLER X-ACHSE
//	***** ANFANG *****
//	---------------------------------------------------------------------------------------------------------------------------
//	REGLER POSITION X-ACHSE
//	Sollwert:
//	mySetpoint_p_x kommt aus Funktion für SPIRALE


//	Istwert:
//	myInput_p_x kommt vom Analogeingang A1 (aktuelle Stickposition X) normieren
	myInput_p_x = AF_A1_i - nullX;													// Nullpunktverschiebung
	if (myInput_p_x >= 0)															// Proportionalitätsfaktor für Anstiegsnormierung
	{	myInput_p_x =			myInput_p_x * propfaktor_plusX_30;					// getrennt für plus- und minus-Bereich
	}
		myInput_p_x = (-1)	*	myInput_p_x * propfaktor_minusX_30;	
		

	
//	Regleroutput-Maxwerte:
	outMax_p_x		=  1.0;							// Max Drehmoment
	outMin_p_x		= -1.0;							// Min Drehmoment

//	Reglerparameter:
	kp_p_x			= 0.002;						// harte Charakteristik
//	ki_p_x			= 0.0;
	kd_p_x			= 0.0035;


	
//	Regler rechnen
	PID_p_x();

//	Stellgröße:	
	LF_x = myOutput_p_x;							// Zuweisung des Regler-Outputs zum Leistungsfaktor-Position 


//	MOTORANSTEUERUNGS-WERTE X-ACHSE BERECHNEN
//	***** ANFANG *****
//	Wenn der Leistungsfaktor	-> positiv, dann -90° (elektrisch) Vektor raus geben
//								-> negativ, dann +90° (elektrisch) Vektor raus geben
	if (LF_x >= 0)
	{
//	Drehwinkel berechnen und dann 90° subtrahieren
//	1500 -> 33°, 500 -> 11°, ...
		DWEx = 7 * ((NOR1 * myInput_p_x) + offset_winkel_x) + 90;
	
//	Winkelanteile mit Berücksichtigung des Leistungsfaktors berechnen
		xvec_motorX =			LF_x * cos(DWEx*WK1);
		yvec_motorX =			LF_x * cos(DWEx*WK1-WK2);
		zvec_motorX =			LF_x * cos(DWEx*WK1-WK3);
	}
	if (LF_x < 0)
	{
//	Drehwinkel berechnen und dann 90° addieren
		DWEx = 7 * ((NOR1 * myInput_p_x) + offset_winkel_x) - 90;
	
//	Winkelanteile mit Berücksichtigung des Leistungsfaktors berechnen
		xvec_motorX = (-1)	*	LF_x * cos(DWEx*WK1);
		yvec_motorX = (-1)	*	LF_x * cos(DWEx*WK1-WK2);
		zvec_motorX = (-1)	*	LF_x * cos(DWEx*WK1-WK3);
	}
//	***** ENDE *****



//	PRINT
//		printf("| AF_A0_i       : %15d| myInput2_1    : %15s| DWEy          : %15s| LF_y          : %15s|\r\n\n",
//		AF_A0_i, doubleToString(s1, myInput2_1), doubleToString(s2, DWEy), doubleToString(s3, LF_y));




		
//	Gemeinsame Raumvektorausgabe ---------------------------------------------------------------------------------------------	
	SVPWM(xvec_motorY, yvec_motorY, zvec_motorY, xvec_motorX, yvec_motorX, zvec_motorX);


		} // if (svpwm_int == 1)
	} // while (1)
} // int main(void)
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx          MAIN           xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
