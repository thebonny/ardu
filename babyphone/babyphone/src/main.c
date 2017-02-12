#include "asf.h"
#include "conf_board.h"
	
#include "stdio_serial.h"
#include "conf_clock.h"
	
#include "utils.h"
#include "PID.h"
#include "PWM.h"
#include "ADC.h"
#include "ppm_capture.h"
#include "ppm_out.h"
#include "record_playback.h"

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
	// the pwm drives the motors as well as the adc handler, so kick it off first	
	pwm_initialize();
	// adc data is required for calibration, so we start the adc handler next
	adc_initialize();
	// then we have to calibrate the stick
	calibration_sequence();
	// then we start the general pid update cycles	
	pid_initialize();
	
	// init peripheral modules to support rc Tx and simulator playback
	ppm_capture_initialize();
	ppm_out_initialize();
	record_playback_initialize();

	
	char key;
	display_menu();
	
	while (1)
	{
 printf("\n\r1!");
/*		uint16_t mode_channel = get_captured_raw_channel(8).current_captured_ppm_value;
	   if (mode_channel < 900) {
		   if (mode != BYPASS) {
			   puts("\n\rBypass mode!");
				stop_record();
		   }
		} else if (mode_channel >= 900 && mode_channel < 1300) {	
			if (mode != RECORD) {
				puts("\n\rStart recording flight sequence!");
				start_record();
			}
		} else {
			if (mode != PLAYBACK) {
				puts("\n\rStart playback of flight sequence!");
				start_playback();
			}
			
		}*/

	}	
}
		
		
		

