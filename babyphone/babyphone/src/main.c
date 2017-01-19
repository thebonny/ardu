	#include "asf.h"
	#include "conf_board.h"
	
	#include "stdio_serial.h"			// UART		z.B. printf("MEC22_STEPS:   %i\r\n", ENC1_C);
	#include "conf_clock.h"
	#include "math.h"					//			z.B. cos(x)

	#include "delay.h"
	#include <includes/ppm_out.h>
	#include <includes/ppm_capture.h>
	#include <includes/record_playback.h>
	#include <includes/utils.h>
	#include <includes/PID.h>
	#include <includes/PWM.h>
	#include <includes/ADC.h>
	
	#define REG_ADC_CDR6			(*(__I  uint32_t*)0x400C0068U) // ADC Channel Data Register

	#define REG_ADC_CDR7			(*(__I  uint32_t*)0x400C006CU) // ADC Channel Data Register

	#define	PI		3.141592654f
	#define	WK1		(PI/180)
	#define	WK2		(PI*2/3)
	#define	WK3		(PI*4/3)
		


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



//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx     Init SAM system     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx	
	sysclk_init();
	board_init();
	configure_console();
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx     Init SAM system     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	
	ppm_out_initialize();
	ppm_capture_initialize();
	record_playback_initialize();
	
	INIT_PWM();
	INIT_ADC();
	INIT_PID();

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