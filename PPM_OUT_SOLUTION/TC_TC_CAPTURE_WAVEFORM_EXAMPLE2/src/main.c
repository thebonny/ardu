/*
 * main.c
 *
 * Created: 09.01.2017 13:31:27
 *  Author: tmueller
 */ 
#include <asf.h>
#include <conf_clock.h>
#include <conf_board.h>
#include <modules/ppm_out.h>
#include <modules/ppm_capture.h>
#include <modules/record_playback.h>


static void configure_console(void)
{
	const usart_serial_options_t uart_serial_options = {
		.baudrate = CONF_UART_BAUDRATE,
		#ifdef CONF_UART_CHAR_LENGTH
		.charlength = CONF_UART_CHAR_LENGTH,
		#endif
		.paritytype = CONF_UART_PARITY,
		#ifdef CONF_UART_STOP_BITS
		.stopbits = CONF_UART_STOP_BITS,
		#endif
	};

	/* Configure console UART. */
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

int main(void)
{
	uint8_t key;
	sysclk_init();
	board_init();
	configure_console();

	ppm_out_initialize();
	ppm_capture_initialize();
	record_playback_initialize();


	
	/* Display menu */
	display_menu();

    while (1) {
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
		
	}
}