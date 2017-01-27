	#include "asf.h"
	#include "conf_board.h"
	
	#include "stdio_serial.h"			// UART		z.B. printf("MEC22_STEPS:   %i\r\n", ENC1_C);
	#include "conf_clock.h"
	#include "math.h"					//			z.B. cos(x)

	#include "delay.h"
	#include "includes/utils.h"
	#include "includes/PID.h"
	#include "includes/PWM.h"
	#include "includes/ADC.h"
	#include "includes/ppm_capture.h"
	#include "includes/ppm_out.h"
	#include "includes/record_playback.h"

	
	#define REG_ADC_CDR6			(*(__I  uint32_t*)0x400C0068U) // ADC Channel Data Register

	#define REG_ADC_CDR7			(*(__I  uint32_t*)0x400C006CU) // ADC Channel Data Register

	#define	PI		3.141592654f
	#define	WK1		(PI/180)
	#define	WK2		(PI*2/3)
	#define	WK3		(PI*4/3)
		


	float myInput1_1;
	float myInput1_2;

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

	static		int		cnt_1ms_poll = 0;	
	
	char s1[32];
	char s2[32];
	char s3[32];
	


	volatile	float	CH1_WERT1_1			= 0.0;						// Empfänger-Wert
	volatile	float	CH1_WERT1_1_alt		= 0.0;
	volatile	float	CH1_WERT1_1_li		= 0.0;						// linear interpoliert
	volatile	float	CH1_WERT1_1_li_nor	= 0.0;						// linear interpoliert und normiert
	volatile	float	CH1_DELTA			= 0.0;						// Delta-Wert für 1ms
	
	volatile	float	CH1_WERT2_1 = 0.0;
	volatile	int		CH1_WERT3_1 = 0;


	volatile	float	CH2_WERT1_1			= 0.0;						// Empfänger-Wert
	volatile	float	CH2_WERT1_1_alt		= 0.0;
	volatile	float	CH2_WERT1_1_li		= 0.0;						// linear interpoliert
	volatile	float	CH2_WERT1_1_li_nor	= 0.0;						// linear interpoliert und normiert
	volatile	float	CH2_DELTA			= 0.0;						// Delta-Wert für 1ms
	
	volatile	float	CH2_WERT2_1 = 0.0;
	volatile	int		CH2_WERT3_1 = 0;


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


int main(void)
{
	sysclk_init();
	board_init();
	configure_console();
	
	INIT_PWM();
	INIT_ADC();
	ppm_out_initialize();
	ppm_capture_initialize();
	record_playback_initialize();
	

	// display_menu();
	
	while (1)
	{
		
		if ( has_ADC_completed_20_conversions() == 1) {
		
			reset_ADC();
			cnt_1ms_poll++;

			// Lineare Interpolation, um 1ms Werte vom Master zu bekommen, der nur alle 20ms einen aktuellen Wert versendet
			//	alle 20ms
			if (cnt_1ms_poll % 20 == 0)																// 20ms
			{
				//	CH1 (TIOA7)
				CH1_WERT1_1_alt = CH1_WERT1_1;													// alten CH0-Wert retten
				CH1_WERT1_1 = get_captured_channel_value(1) * 2;														// alle 20ms neuen CH0-Wert übernehmen
				CH1_DELTA = (CH1_WERT1_1 - CH1_WERT1_1_alt)/20;

				CH1_WERT1_1_li = CH1_WERT1_1_li - CH1_DELTA;									// weil gleich danach in "jede ms" wieder CH0_DELTA dazu addiert wird

				//	CH2 (TIOA8)

				CH2_WERT1_1_alt = CH2_WERT1_1;													// alten CH0-Wert retten
				CH2_WERT1_1 = get_captured_channel_value(2) * 2;														// alle 20ms neuen CH0-Wert übernehmen
				CH2_DELTA = (CH2_WERT1_1 - CH2_WERT1_1_alt)/20;

				CH2_WERT1_1_li = CH2_WERT1_1_li - CH2_DELTA;									// weil gleich danach in "jede ms" wieder CH0_DELTA dazu addiert wird
			}
			
			
			
			
			//	jede ms
			//	CH1 (TIOA7)
			CH1_WERT1_1_li = CH1_WERT1_1_li + CH1_DELTA;									// CH0-Wert ist alter CH0-Wert + Delta

			
			CH1_WERT1_1_li_nor = CH1_WERT1_1_li;			// Normierung auf Laufwege HS
			motor_Y_position.setpoint = CH1_WERT1_1_li_nor;												//int wert Übergabe

			//	CH2 (TIOA8)
			CH2_WERT1_1_li = CH2_WERT1_1_li + CH2_DELTA;									// CH0-Wert ist alter CH0-Wert + Delta

			
			CH2_WERT1_1_li_nor = CH2_WERT1_1_li;			// Normierung auf Laufwege HS
			motor_X_position.setpoint = CH2_WERT1_1_li_nor;												//int wert Übergabe
			
			
			//	REGLER rechnen

//	REGLER POSITION
	motor_X_position.input = get_average_adc_input_X();								
	LF1 = pid_compute(&motor_X_position);							


//	Ausgabe an MOTOR1
//	Zum Verständnis:
//	Wenn der Leistungsfaktor	-> positiv, dann +90° (elektrisch) Vektor raus geben
//								-> negativ, dann -90° (elektrisch) Vektor raus geben
			if (LF1 >= 0)
			{	DWE1 = 7 * ((33 * (get_average_adc_input_X() - 2418) / 1677.0) + WKL_OFF_1) + 90;	// "-" bei [2]


//	Winkelanteile mit Berücksichtigung des Leistungsfaktors berechnen
				X1 = LF1 * cos(DWE1*WK1);
				Y1 = LF1 * cos(DWE1*WK1-WK2);
				Z1 = LF1 * cos(DWE1*WK1-WK3);
			}
			if (LF1 < 0)
			{
				DWE1 = 7 * ((33 * (get_average_adc_input_X() - 2418) / 1677.0) + WKL_OFF_1) - 90;	// "+" bei [2]						
//	Winkelanteile mit Berücksichtigung des Leistungsfaktors berechnen
				X1 = -LF1 * cos(DWE1*WK1);
				Y1 = -LF1 * cos(DWE1*WK1-WK2);
				Z1 = -LF1 * cos(DWE1*WK1-WK3);
			}

			if (cnt_1ms_poll % 200 == 0)			// 500 x 1ms = 500ms
						{
				printf("| X1      : %15s| Y1      : %15s| Z1  : %15s\r\n",
					doubleToString(s1, X1), doubleToString(s2, Y1), doubleToString(s3, Z1));
				printf("| Sollwert      : %15s\r\n",
					doubleToString(s1, motor_X_position.setpoint));
					printf("\r\n");
				printf("| X2      : %15s| Y2      : %15s| Z2  : %15s\r\n",
				doubleToString(s1, X2), doubleToString(s2, Y2), doubleToString(s3, Z2));
				printf("| Sollwert      : %15s\r\n",
				doubleToString(s1, motor_Y_position.setpoint));	
				printf("------------\r\n");
						}
	

			motor_Y_position.input = get_average_adc_input_Y();
			LF2 = pid_compute(&motor_Y_position);
	

//	Ausgabe an MOTOR2
//	Zum Verständnis:
//	Wenn der Leistungsfaktor	-> positiv (Stick links),	dann -90° (elektrisch) Vektor raus geben
//								-> negativ (Stick rechts),	dann +90° (elektrisch) Vektor raus geben
			if (LF2 >= 0)
			{
				DWE2 = 7 * ((33 * (get_average_adc_input_Y() - 2304) / 1641.0) + WKL_OFF_2) + 90;	
//	Winkelanteile mit Berücksichtigung des Leistungsfaktors berechnen
				X2 = LF2 * cos(DWE2*WK1);
				Y2 = LF2 * cos(DWE2*WK1-WK2);
				Z2 = LF2 * cos(DWE2*WK1-WK3);
			}

			if (LF2 < 0)
			{
				DWE2 = 7 * ((33 * (get_average_adc_input_Y() - 2304) / 1641.0) + WKL_OFF_2) - 90;					

//	Winkelanteile mit Berücksichtigung des Leistungsfaktors berechnen
				X2 = -LF2 * cos(DWE2*WK1);
				Y2 = -LF2 * cos(DWE2*WK1-WK2);
				Z2 = -LF2 * cos(DWE2*WK1-WK3);
			}
	

		
//	Gemeinsame Raumvektorausgabe ---------------------------------------------------------------------------------------------	
		SVPWM(X1, Y1, Z1, X2, Y2, Z2);

		}	// if (svpwm_int == 1)

	}	// while (1)
}
		
		
		
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
