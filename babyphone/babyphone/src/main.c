#include "asf.h"
#include "conf_board.h"
	
#include "stdio_serial.h"
#include "conf_clock.h"
	
#include "includes/utils.h"
#include "includes/PID.h"
#include "includes/PWM.h"
#include "includes/ADC.h"
#include "includes/ppm_capture.h"
#include "includes/ppm_out.h"
#include "includes/record_playback.h"

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


static void gpio_initialize(void)
{
	// init debug pins c.12 and c.14 for output
	REG_PIOC_PER	= REG_PIOC_PER		|		0x00005000u;
	REG_PIOC_OER	 = REG_PIOC_OER		|		0x00005000u;	
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
	
	// init core HAPStik Modules
	gpio_initialize();
	pwm_initialize();
	adc_initialize();
	pid_initialize();
	
	// init peripheral modules to support rc Tx and simulator playback
	ppm_out_initialize();
	ppm_capture_initialize();
	record_playback_initialize();
	
	char key;
	display_menu();
	
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
		

	}	
}
		
		
		

