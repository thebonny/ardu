
#include <asf.h>
#include <conf_board.h>
#include <conf_clock.h>

#define TC_CAPTURE_TIMER_SELECTION TC_CMR_TCCLKS_TIMER_CLOCK3
/** Use TC Peripheral 2 **/
#define TC  TC2

/** Configure TC2 channel 0 as capture input. **/
#define TC_CHANNEL_CAPTURE 0
#define ID_TC_CAPTURE      ID_TC6  //TC6 is channel 0 of TC2 module!!
#define PIN_TC_CAPTURE     PIO_PC26_IDX  // PIN TIOA6 which is digital PIN 5 on Due
#define PIN_TC_CAPTURE_MUX IOPORT_MODE_MUX_B // peripheral B

/** Use TC2_Handler for TC capture interrupt**/
#define TC_Handler  TC6_Handler
#define TC_IRQn     TC6_IRQn


static uint32_t gs_ul_captured_rb;

static int CHANNEL[8];

static int channel_id = 0;

void TC_Handler(void)
{
	if ((tc_get_status(TC, TC_CHANNEL_CAPTURE) & TC_SR_LDRBS) == TC_SR_LDRBS) {
		uint16_t micros = 0;
		gs_ul_captured_rb = tc_read_rb(TC, TC_CHANNEL_CAPTURE);
		micros = (gs_ul_captured_rb * 8) / 21; // clock is Master Clock with prescaler 32 at 84 MHz Master Clock
		if (micros > 3000) {
			// PPM sync pulse, recount channels starting with 0
			channel_id = 0;
			return;
		}
		CHANNEL[channel_id] = micros;
		channel_id++;		
	}
}

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

	stdio_serial_init(CONF_UART, &uart_serial_options);
}

int main(void)
{
	sysclk_init();
	board_init();

	configure_console(); 
	pmc_enable_periph_clk(ID_TC_CAPTURE);
	tc_init(TC, TC_CHANNEL_CAPTURE,
	TC_CAPTURE_TIMER_SELECTION /* Clock Selection */
	| TC_CMR_LDRA_FALLING /* RA Loading: rising edge of TIOA */
	| TC_CMR_LDRB_RISING /* RB Loading: falling edge of TIOA */
	| TC_CMR_ABETRG /* External Trigger: TIOA */
	| TC_CMR_ETRGEDG_RISING /* External Trigger Edge: Falling edge */
	);
	
	NVIC_DisableIRQ(TC_IRQn);
	NVIC_ClearPendingIRQ(TC_IRQn);
	NVIC_SetPriority(TC_IRQn, 0);
	NVIC_EnableIRQ(TC_IRQn);
	
	tc_enable_interrupt(TC, TC_CHANNEL_CAPTURE, TC_IER_LDRBS);
	tc_start(TC, TC_CHANNEL_CAPTURE);
	printf("TESTESTESTESTESTESTESTEST");
	while(1) {
		for (int i = 0; i < 8; i++) {
			printf("%d::", CHANNEL[i]);
			
		}
		printf("\r\n");		
	}
}

