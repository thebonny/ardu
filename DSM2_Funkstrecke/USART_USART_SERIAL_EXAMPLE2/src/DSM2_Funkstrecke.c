

#include <string.h>
#include "asf.h"
#include "stdio_serial.h"
#include "conf_board.h"
#include "conf_clock.h"
#include "conf_example.h"


#define sendDX5eBindData

#define	CODE_NORMAL_SENDING	0x18		// DSMX all 2.4GHz channels
#define CODE_BINDING		0x98
#define CODE_LOW_POWER		0x38


#define CENTER_DSM2    511		        // DSM2 HF-Module value for Servo-middle

// ----- Send Mode -----
typedef enum {
	NULL_MD = -1, SEND_DSM2, BINDING, LOW_PWR
} Mode_t;

#define DSM2_CHANNELS     6                 // Max number of DSM2 Channels transmitted

// ------------------------------------------ Data --------------------------------------------------
static Mode_t  Mode;			    // TX-Mode null, send_dsm2, binding, low_power
struct DSM2_Data {		            // TX data struct
	uint8_t Header[2];
	uint8_t	Channel[DSM2_CHANNELS * 2];
} DSM2_Data;

// --- DX5e Daten im binding Mode ---
#ifdef sendDX5eBindData
// DX5e Bind Data
static uint8_t DX5eBindData[(DSM2_CHANNELS * 2) + 2] = {CODE_BINDING, 0x00, 0x00, 0x00, 0x05, 0xFF, 0x09, 0xFF, 0x0D, 0xFF, 0x10, 0xAA, 0x14, 0xAA};
#endif

/** All interrupt mask. */
#define ALL_INTERRUPT_MASK  0xffffffff

/** Byte mode read buffer. */
static uint32_t gs_ul_read_buffer = 0;

/** DSM2 frame frequency in Hz */
#define TC_FREQ             50

static void sendDSM2() {

	
	if (Mode == SEND_DSM2) {
		DSM2_Data.Header[0] = CODE_NORMAL_SENDING;
	}
	else if (Mode == BINDING) {
		DSM2_Data.Header[0] = CODE_BINDING;
		
	}
	else {
		DSM2_Data.Header[0] = CODE_LOW_POWER;
		
	}

	DSM2_Data.Header[1] = 0x00;              		// second header byte
	

	// alle 6 Channel Werte holen
	for (uint8_t i = 0; i < DSM2_CHANNELS; i++) {	// get receiver data
		uint16_t temp = 511;
		DSM2_Data.Channel[i * 2]   = (uint8_t)(i << 2) | (temp >> 8);
		DSM2_Data.Channel[i * 2 + 1] = temp;
	}



	if (Mode == BINDING) {
		// die 2 HeaderBytes und die Channel-Werte senden
		//   Serial1.write(DX5eBindData, sizeof(DX5eBindData));
		} else {
		// die 2 HeaderBytes und die Channel-Werte senden
		usart_putchar(BOARD_USART, DSM2_Data.Header[0]);
		usart_putchar(BOARD_USART, DSM2_Data.Header[1]);
		for (int i = 0; i < 12; i++) {
			usart_putchar(BOARD_USART, DSM2_Data.Channel[i]);
		}
		
		
	}


}

void TC3_Handler(void)
{
	uint32_t ul_status;
	uint32_t ul_byte_total = 0;

	/* Read TC0 Status. */
	ul_status = tc_get_status(TC1, 0);

	/* RC compare. */
	if ((ul_status & TC_SR_CPCS) == TC_SR_CPCS)  {
		sendDSM2();
	}
}

static void configure_usart(void)
{
	const sam_usart_opt_t usart_console_settings = {
		BOARD_USART_BAUDRATE,
		US_MR_CHRL_8_BIT,
		US_MR_PAR_NO,
		US_MR_NBSTOP_1_BIT,
		US_MR_CHMODE_NORMAL,
		/* This field is only used in IrDA mode. */
		0
	};

	/* Enable the peripheral clock in the PMC. */
	sysclk_enable_peripheral_clock(BOARD_ID_USART);

	/* Configure USART in serial mode. */
	usart_init_rs232(BOARD_USART, &usart_console_settings,
			sysclk_get_cpu_hz());

	/* Disable all the interrupts. */
	usart_disable_interrupt(BOARD_USART, ALL_INTERRUPT_MASK);

	usart_enable_tx(BOARD_USART);
}

static void configure_tc(void)
{
	uint32_t ul_div;
	uint32_t ul_tcclks;
	static uint32_t ul_sysclk;

	/* Get system clock. */
	ul_sysclk = sysclk_get_cpu_hz();

	/* Configure PMC. */
	pmc_enable_periph_clk(ID_TC3);

	/* Configure TC for a 50Hz frequency and trigger on RC compare. */
	tc_find_mck_divisor(TC_FREQ, ul_sysclk, &ul_div, &ul_tcclks, ul_sysclk);
	tc_init(TC1, 0, ul_tcclks | TC_CMR_CPCTRG);
	tc_write_rc(TC1, 0, (ul_sysclk / ul_div) / TC_FREQ);

	/* Configure and enable interrupt on RC compare. */
	NVIC_EnableIRQ((IRQn_Type)ID_TC3);
	tc_enable_interrupt(TC1, 0, TC_IER_CPCS);
}

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
 * \brief Application entry point for usart_serial example.
 *
 * \return Unused (ANSI-C compatibility).
 */
int main(void)
{

	/* Initialize the SAM system. */
	sysclk_init();
	board_init();

	/* Configure UART for debug message output. */
	configure_console();

	/* Configure USART. */
	configure_usart();

	configure_tc();
	
	tc_start(TC1, 0);
	while (1) {
	
	}
}
