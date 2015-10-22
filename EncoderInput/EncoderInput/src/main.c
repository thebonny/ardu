#include <asf.h>
#include <stdio_serial.h>
#include <conf_board.h>
#include <conf_clock.h>


#define STRING_EOL    "\r"
#define STRING_HEADER "--Pin capture Example --\r\n" \
		"-- "BOARD_NAME " --\r\n" \
		"-- Compiled: "__DATE__ " "__TIME__ " --"STRING_EOL


volatile counter = 0;



/**
 *  Configure UART console.
 */
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


static void Encoder_Handler(uint32_t id, uint32_t mask)
{
	if (id == id && mask == mask) {
		counter++;
		printf("counter=%i::", counter);
	}
}
int main(void)
{

	/* Initialize the SAM system */
	sysclk_init();
	board_init();

	/* Initialize the console uart */
	configure_console();
	
	pmc_enable_periph_clk(ID_PIOA);
	
	pio_set_input(PIOA, PIO_PA16, PIO_PULLUP);
	pio_set_debounce_filter(PIOA, PIO_PA16, 1);
	pio_handler_set(PIOA, ID_PIOA, PIO_PA16, 0 /* TODO genaues Flanken/Level-Attribut festlegen */, Encoder_Handler);
	NVIC_EnableIRQ((IRQn_Type) ID_PIOA);
	pio_handler_set_priority(PIOA,(IRQn_Type) ID_PIOA, 0 /* highest priority = 0 */);
	pio_enable_interrupt(PIOA, PIO_PA16);
		
	int counter = 0;
	int active = 1;

	while (1) {
	
	}
}