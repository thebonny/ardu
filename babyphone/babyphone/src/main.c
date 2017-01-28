	#include "asf.h"
	#include "conf_board.h"
	
	#include "stdio_serial.h"			// UART		z.B. printf("MEC22_STEPS:   %i\r\n", ENC1_C);
	#include "conf_clock.h"
	

	#include "delay.h"
	#include "includes/utils.h"
	#include "includes/PID.h"
	#include "includes/PWM.h"
	#include "includes/ADC.h"
	#include "includes/ppm_capture.h"
	#include "includes/ppm_out.h"
	#include "includes/record_playback.h"

	static		int		cnt_1ms_poll = 0;	

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

static void display_debug_output() {
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
	
	char key;
	// display_menu();
	
	while (1)
	{
		scanf("%c", (char *)&key);

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
		}
		
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
			
			CH1_WERT1_1_li = CH1_WERT1_1_li + CH1_DELTA;									// CH0-Wert ist alter CH0-Wert + Delta
			CH1_WERT1_1_li_nor = CH1_WERT1_1_li;			// Normierung auf Laufwege HS
			motor_Y_position_controller.setpoint = CH1_WERT1_1_li_nor;												//int wert Übergabe

			CH2_WERT1_1_li = CH2_WERT1_1_li + CH2_DELTA;									// CH0-Wert ist alter CH0-Wert + Delta
			CH2_WERT1_1_li_nor = CH2_WERT1_1_li;			// Normierung auf Laufwege HS
			motor_X_position_controller.setpoint = CH2_WERT1_1_li_nor;												//int wert Übergabe
			
			
			compute_all_controllers();
			if (cnt_1ms_poll % 1000 == 0) {
				// display_debug_output();
			}


		}	

	}	
}
		
		
		

