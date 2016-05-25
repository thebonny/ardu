#include <string.h>
#include "asf.h"
#include "conf_board.h"

#define TRACKING_TIME         1
/* Transfer Period */
#define TRANSFER_PERIOD       1

#define ADC_CHANNEL_POTENTIOMETER  ADC_CHANNEL_1

static volatile uint32_t adc_value = 0;

/**
 * \brief Start ADC sample.
 * Initialize ADC, set clock and timing, and set ADC to given mode.
 */
static void start_adc(void)
{
	pmc_enable_periph_clk(ID_ADC);

	/* Initialize ADC. */
	/*
	 * Formula: ADCClock = MCK / ( (PRESCAL+1) * 2 )
	 * For example, MCK = 64MHZ, PRESCAL = 4, then:
	 * ADCClock = 64 / ((4+1) * 2) = 6.4MHz;
	 */
	/* Formula:
	 *     Startup  Time = startup value / ADCClock
	 *     Startup time = 64 / 6.4MHz = 10 us
	 */
	adc_init(ADC, sysclk_get_cpu_hz(), 6400000, ADC_STARTUP_TIME_4);

	/* Formula:
	 *     Transfer Time = (TRANSFER * 2 + 3) / ADCClock
	 *     Tracking Time = (TRACKTIM + 1) / ADCClock
	 *     Settling Time = settling value / ADCClock
	 *
	 *     Transfer Time = (1 * 2 + 3) / 6.4MHz = 781 ns
	 *     Tracking Time = (1 + 1) / 6.4MHz = 312 ns
	 *     Settling Time = 3 / 6.4MHz = 469 ns
	 */
	adc_configure_timing(ADC, TRACKING_TIME, ADC_SETTLING_TIME_3, TRANSFER_PERIOD);
	/* Enable channels. */
	adc_enable_channel(ADC, ADC_CHANNEL_POTENTIOMETER);
	/* Enable Data ready interrupt. */
	adc_enable_interrupt(ADC, ADC_IER_DRDY);
	/* Enable ADC interrupt. */
	NVIC_EnableIRQ(ADC_IRQn);
	adc_configure_trigger(ADC, ADC_TRIG_SW, 0);	/* Disable hardware trigger. */	
	
}


void ADC_Handler(void)
{
	uint32_t i;
	uint32_t ul_temp;
	uint8_t uc_ch_num;

	/* Without PDC transfer */
	if ((adc_get_status(ADC) & ADC_ISR_DRDY) ==
			ADC_ISR_DRDY) {
		ul_temp = adc_get_latest_value(ADC);
	//	printf("%04d mv.    ",(int)(ul_temp));
				adc_value =
						ul_temp &
						ADC_LCDR_LDATA_Msk;
		
	}
	
}
/**
 *  Configure UART console.
 */
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

int main(void)
{
	/* Initialize the SAM system. */
	sysclk_init();
	board_init();
	configure_console();
	start_adc();
	while (1) {
			adc_start(ADC);
			printf("%04d mv.    ",
			(int)(adc_value));
			puts("\r");
		
	}
}
