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

int main(void)
{
	sysclk_init();
	board_init();
	configure_console();

	ppm_out_initialize();
	ppm_capture_initialize();
	record_playback_initialize();
	start_record();

	
	printf("Start!\r\n");

    while (1) {
		
		
	}
}