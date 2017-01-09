/*
 * main.c
 *
 * Created: 09.01.2017 13:31:27
 *  Author: tmueller
 */ 
#include <asf.h>
#include <conf_clock.h>
#include <conf_board.h>
#include <ppm_out.h>
#include <ppm_capture.h>

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
	


	while (1) {
		for (int i = 0; i < NUMBER_OF_RC_CHANNELS; i++) {
			set_rc_channel_value(i, get_channel_value_as_PPM(i));
		}
	}
}