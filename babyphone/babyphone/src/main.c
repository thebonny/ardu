







/*	DEBUG_PINS
	Debug_Pin C.25= 1 -> C.25 = 0
				REG_PIOC_SODR	= REG_PIOC_SODR		|	0x02000000u; 		// Ausgang C25 =1 (DUE_Pin 5)	
				delay_us(10);
				REG_PIOC_CODR	= REG_PIOC_CODR		|	0x02000000u; 		// Ausgang C25 =0 (DUE_Pin 5)	

	Debug_Pin C.24= 1 -> C.24 = 0
				REG_PIOC_SODR	= REG_PIOC_SODR		|	0x01000000u; 		// Ausgang C24 =1 (DUE_Pin 6)
				delay_us(10);
				REG_PIOC_CODR	= REG_PIOC_CODR		|	0x01000000u; 		// Ausgang C24 =0 (DUE_Pin 6)
*/





//	ANFANG **********************************************     Includes       *************************************************
	#include "asf.h"
	#include "conf_board.h"
	
	#include "stdio_serial.h"			// UART		z.B. printf("MEC22_STEPS:   %i\r\n", ENC1_C);
	#include "conf_clock.h"
	#include "math.h"					//			z.B. cos(x)
	#include "string.h"
	#include "delay.h"
	#include "modules/aggregat.h"
	#include <modules/ppm_out.h>
	#include <modules/ppm_capture.h>
	#include <modules/record_playback.h>
	
	//	static double PRECISION = 0.00000000000001;
	static double PRECISION = 0.001;					//Anzahl Nachkommastellen, -> hier ohne Rundung!
	
	
	


	static int MAX_NUMBER_STRING_SIZE = 32;
	
		float myInput1_1;
		int myInput2_1;
			float myInput1_2;
			
			double myOutput2_1;
			double myOutput1_1;
			double myOutput1_2;
	
	//	Leistungsfaktor	0 ... 1 Leistung | 0.5 -> 50% Leistung | 1.0 -> 100% Leistung
	volatile	float	LF1	= 1.0;
	volatile	float	LF2	= 1.0;
	
		volatile	float	DWE1		= 0.0;
		volatile	float	DWE_a1		= 0.0;									// alter DWE-Wert

		volatile	float	DWE2		= 0.0;
		volatile	float	DWE_a2		= 0.0;
	
		volatile	float	WKL_OFF_1 =		5.5;							// Winkel_Offset für Poti_MOTOR1 Vertikal		(Nullposition)	-> "+" Stick wandert nach unten
		volatile	float	WKL_OFF_2 =		-16.0;
	
	//	für SVPWM()
	float	X1, Y1, Z1;
	float	X2, Y2, Z2;
		#define	PI		3.141592654f
		#define	WK1		(PI/180)
		#define	WK2		(PI*2/3)
		#define	WK3		(PI*4/3)
	
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

	char s1[32];
	char s2[32];
	char s3[32];
	char s4[32];
	char s5[32];
	
	//	* Double to ASCII prototype. Must be declared before usage in main function !!
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




/**
 * \brief Display the user menu on the UART.
 */
static void display_menu(void)
{
	uint8_t i;
	puts("\n\rMenu :\n\r"
			"------\n\r"
			"  HAPStik Prototype Options:\r");

	printf("  -------------------------------------------\n\r"
			"  r: Record flight sequence\n\r"
			"  p: Playback recorded sequence \n\r"
			"  b: bypass recorder \n\r"
			"  d: double speed\n\r"
			"  h: half speed\n\r"
			"  m: Display menu \n\r"
			"------\n\r\r");
}



//	ENDE ************************************************     FUNKTIONEN     *************************************************

//	**************************************************************************************************************************
//	ANFANG *******************************************          MAIN           ***********************************************
//	**************************************************************************************************************************
int main(void)
{


uint8_t key;
//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx     Init SAM system     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx	
	sysclk_init();
	board_init();
	configure_console();
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx     Init SAM system     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	
	ppm_out_initialize();
	ppm_capture_initialize();
	record_playback_initialize();
	
	aggregat_init();


	display_menu();

    while (1) {
		
		
		//	----------------------------------------------------------------------------------------------------------- Endlosschleife
	while (1)
	{

//	1ms ----------------------------------------------------- REGLELUNG -------------------------------------------------------
//	---------------------------------------------------------------------------------------------------------------------------


	delay_ms(1);


//	Reglerkaskade PID1 (Position) -> PID2 (Geschwindigkeit)

//	MOTOR1 -------------------------------------------------------------------------------------------------------------------




//	REGLER rechnen

//	REGLER POSITION
	myInput1_1 = REG_ADC_CDR7;								// Input: Positionsdaten:
	PID1_1();											//		-> Kanal0 ADC (Average-Filter, 20-fach Oversampling) 
//	mySetpoint2_1 = myOutput1_1;						// Ausgang ist Führungsgröße für REGLER GESCHWINDIGKEIT
	LF1 = myOutput1_1;									// Wenn nur POSITIONS-REGLER
	
//	REGLER GESCHWINDIGKEIT (REG_TC0_CV0)
//	myInput2_1 = POT_V_1;
//	PID2_1();
//	LF1 = myOutput2_1;


/*
//	PRINT
//	Poti lesen, erfolgt in der ADC-Interrupt Routine
			if (TAE_1ms % 500 == 0)			// 500 x 1ms = 500ms
			{
				printf("|POT_V_1: %10s|LF1    : %10s|mSp2_1 : %10s|mI1_1  : %10s|mO1_1  : %10s|     \r\n",
				doubleToString(s1, POT_V_1), doubleToString(s2, LF1), doubleToString(s3, mySetpoint2_1), doubleToString(s4, myInput1_1), doubleToString(s5, myOutput1_1)  );
			}
*/






//	Ausgabe an MOTOR1
//	Zum Verständnis:
//	Wenn der Leistungsfaktor	-> positiv, dann +90° (elektrisch) Vektor raus geben
//								-> negativ, dann -90° (elektrisch) Vektor raus geben
			if (LF1 >= 0)
			{	DWE1 = 7 * ((33 * (REG_ADC_CDR7 - 2418) / 1677.0) + WKL_OFF_1) + 90;	
//	Winkelanteile mit Berücksichtigung des Leistungsfaktors berechnen
				X1 = LF1 * cos(DWE1*WK1);
				Y1 = LF1 * cos(DWE1*WK1-WK2);
				Z1 = LF1 * cos(DWE1*WK1-WK3);
			}
			if (LF1 < 0)
			{
				DWE1 = 7 * ((33 * (REG_ADC_CDR7 - 2418) / 1677.0) + WKL_OFF_1) - 90;						
//	Winkelanteile mit Berücksichtigung des Leistungsfaktors berechnen
				X1 = -LF1 * cos(DWE1*WK1);
				Y1 = -LF1 * cos(DWE1*WK1-WK2);
				Z1 = -LF1 * cos(DWE1*WK1-WK3);
			}












//	MOTOR2 -------------------------------------------------------------------------------------------------------------------





//	REGLER rechnen

//	REGLER POSITION
	myInput1_2 = REG_ADC_CDR6;								// Input: Positionsdaten:
	PID1_2();											//		-> Kanal0 ADC (Average-Filter, 20-fach Oversampling) 
//	mySetpoint2_2 = myOutput1_2;						// Ausgang ist Führungsgröße für REGLER GESCHWINDIGKEIT
	LF2 = myOutput1_2;									// Wenn nur POSITIONS-REGLER
	
//	REGLER GESCHWINDIGKEIT
//	myInput2_2 = POT_V_2;
//	PID2_2();
//	LF2 = myOutput2_2;


/*
//	PRINT
//	Poti lesen, erfolgt in der ADC-Interrupt Routine
			if (TAE_1ms % 500 == 0)			// 500 x 1ms = 500ms
			{
				printf("|POT_V_2: %10s|LF2    : %10s|mSp2_2 : %10s|mI1_2  : %10s|mO1_2  : %10s|     \r\n\n",
				doubleToString(s1, POT_V_2), doubleToString(s2, LF2), doubleToString(s3, mySetpoint2_2), doubleToString(s4, myInput1_2), doubleToString(s5, myOutput1_2)  );						
			}
*/






//	Ausgabe an MOTOR2
//	Zum Verständnis:
//	Wenn der Leistungsfaktor	-> positiv (Stick links),	dann -90° (elektrisch) Vektor raus geben
//								-> negativ (Stick rechts),	dann +90° (elektrisch) Vektor raus geben
			if (LF2 >= 0)
			{
				DWE2 = 7 * ((33 * (REG_ADC_CDR6 - 2304) / 1641.0) + WKL_OFF_2) + 90;	

//	Winkelanteile mit Berücksichtigung des Leistungsfaktors berechnen
				X2 = LF2 * cos(DWE2*WK1);
				Y2 = LF2 * cos(DWE2*WK1-WK2);
				Z2 = LF2 * cos(DWE2*WK1-WK3);
			}

			if (LF2 < 0)
			{
				DWE2 = 7 * ((33 * (REG_ADC_CDR6 - 2304) / 1641.0) + WKL_OFF_2) - 90;						

//	Winkelanteile mit Berücksichtigung des Leistungsfaktors berechnen
				X2 = -LF2 * cos(DWE2*WK1);
				Y2 = -LF2 * cos(DWE2*WK1-WK2);
				Z2 = -LF2 * cos(DWE2*WK1-WK3);
			}
	

		
//	Gemeinsame Raumvektorausgabe ---------------------------------------------------------------------------------------------	
		SVPWM(X1, Y1, Z1, X2, Y2, Z2);
			
			
	

























//	}	// if (TAE_1ms % 1 == 0)

//	}	// if(iPT == 2)

	}	// if (svpwm_int == 1)

	}	// while (1)
		
		
		
	/*	scanf("%c", (char *)&key);

		switch (key) {
		case 'm':
		display_menu();
		break;

		case 'r':
		puts("\n\rStart recording flight sequence!\r");
		start_record();

		break;

		case 'p':
		puts("\n\rStart playback of flight sequence!\r");
		start_playback();
		break;
		
		case 'l':
		puts("\n\rLooped playback of flight sequence!\r");
		loop_playback();
		break;
		
		case 'b':
		puts("\n\rBypass captured PPM Signal directly to PPM out!\r");
		stop_record();
		break;
		
		case 'd':
		puts("\n\rDouble up record/playback speed!\r");
		double_speed();
		break;
		
		case 'h':
		puts("\n\rHalf record/playback speed!\r");
		half_speed();
		break;

		default:
		puts("Not recognized key pressed \r");
		break;
		}*/
		
	
}