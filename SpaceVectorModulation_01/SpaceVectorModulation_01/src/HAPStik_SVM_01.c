
#include "asf.h"
#include "conf_board.h"
#include "fastmath.h";
#include "foc.h";
#include "delay.h"

#define PWM_FREQUENCY   20000   // um 10kHz (20 KHz / 2) bei Center Aligned Waveform zu erhalten, benötigen wir 20 khz Basisfrequenz
#define PERIOD_VALUE       100 
#define INIT_DUTY_VALUE    0
#define MAX_SYNC_UPDATE_PERIOD  2 // nach dieser ANzahl Perioden, updated der PDC automatisch den Duty Cycle von synchronen Channels
#define DUTY_BUFFER_LENGTH    3 // wenn man x (z.B. "3") Kanäle synchron verschaltet hat (inkl. channel 0 reference) benötigt man x Buffer Length, um EINEN PDC update von jeweils EINEM neuen Duty Cycle Value auf allen x Kanälen zu ermöglichen, bei MEHR (n) als EINEM Duty Cycle Value update benötigt man n*x Buffer length


#define STRING_EOL    "\r"
#define STRING_HEADER "-- HAPStik Space Vector Modulation --\r\n" \
		"-- "BOARD_NAME" --\r\n" \
		"-- Compiled: "__DATE__" "__TIME__" --"STRING_EOL

/** Duty cycle buffer for PDC transfer */
uint16_t g_us_duty_buffer[DUTY_BUFFER_LENGTH];

/** PDC transfer packet */
pdc_packet_t g_pdc_tx_packet;

/**
 * \brief Display menu.
 */
static void display_menu(void)
{
	puts("\r\n"
			"===============================================================\r\n"
			"Menu: Enter angle.\r\n"
			"===============================================================\r\n"
			"\r\n");
}

/**
 * \brief Get numeric value from console.
 *
 * \return Integer value from the console.
 */
static uint8_t get_num_value(void)
{
	uint32_t ul_numkey;
	uint8_t uc_key1, uc_key2;


	while (uart_read(CONSOLE_UART, &uc_key1));
	printf("%c", uc_key1);
	while (uart_read(CONSOLE_UART, &uc_key2));
	printf("%c", uc_key2);
	puts("\r\n");

	if ('0' <= uc_key1 && '9' >= uc_key1) {
		ul_numkey = (uc_key1 - '0');
	}
	if ('0' <= uc_key2 && '9' >= uc_key2) {
		ul_numkey *= 10;
		ul_numkey += (uc_key2 - '0');
	}

	return (uint8_t) ul_numkey;
}

void SVPWM(float DCM1_X, float DCM1_Y, float DCM1_Z) {
	/* Fill duty cycle buffer for channel #0 - #2, duty cycle is calculated by fraction of period value */
	g_us_duty_buffer[0] = DCM1_X * (PERIOD_VALUE);
	g_us_duty_buffer[1] = DCM1_Y * (PERIOD_VALUE);
	g_us_duty_buffer[2] = DCM1_Z * (PERIOD_VALUE);
		
	/* Configure the PDC transfer packet and enable PDC transfer */
	g_pdc_tx_packet.ul_addr = (uint32_t) (&(g_us_duty_buffer[0]));
	g_pdc_tx_packet.ul_size = DUTY_BUFFER_LENGTH;
	pdc_tx_init(PDC_PWM, &g_pdc_tx_packet, 0);
	pdc_enable_transfer(PDC_PWM, PERIPH_PTCR_TXTEN);		
}



/**
 *  \brief Configure the Console UART.
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

/**
 * \brief Application entry point for PWM PDC example.
 *
 * \return Unused (ANSI-C compatibility).
 */
int main(void)
{

	/* Initialize the SAM system */
	sysclk_init();
	board_init();

	/* Configure the console uart for debug infomation */
	configure_console();

	/* Output example information */
	puts(STRING_HEADER);

/* Enable PWM peripheral clock */
	pmc_enable_periph_clk(ID_PWM);
	
	/*A Disable PWM channels
	- Register: PWM_DIS (PWM Disable)
	- Kanal 0: Motor1_X
	- Kanal 1: Motor1_Y	
	- Kanal 2: Motor1_Z
	- Kanal 3: Referenzsignals 
	*/
	pwm_channel_disable(PWM, PWM_CHANNEL_0);
	pwm_channel_disable(PWM, PWM_CHANNEL_1);
	pwm_channel_disable(PWM, PWM_CHANNEL_2);
	pwm_channel_disable(PWM, PWM_CHANNEL_3);
	
	/*A PWM-Leitungen (C.2 - C.9) im Prozessor vom
	PIO-Controller trennen und auf peripheral Funktion B setzen */

	for (int pinId = 2; pinId <= 9; pinId++) { 
		pio_set_peripheral(PIOC, PIO_TYPE_PIO_PERIPH_B, (1u << pinId));
	}
	
	/*A Clock einstellen
	- Set PWM clock A for all channels, clock B not used
	 */
	pwm_clock_t clock_setting = {
			.ul_clka = PWM_FREQUENCY * PERIOD_VALUE,
			.ul_clkb = 0,
			.ul_mck = sysclk_get_cpu_hz() // diese Funktion gibt 84 Mhz zurück!
		};
	pwm_init(PWM, &clock_setting);
	
	/*A Kanäle 0,1,2 als synchron festlegen
	*/
	// zunächst die generellen Eigenschaften der synchronen Channels initialisieren 
	pwm_channel_t sync_channel = {
		/* die Motor Kanäle sollen alle Center aligned sein */
		.alignment = PWM_ALIGN_CENTER,
		/* die Motor Kanäle sollen mit Low Polarität starten */
		.polarity = PWM_LOW,
		/* Alle Motor Kanäle sollen Clock A verwenden, da sie mit doppelter Basisfrequenz getaktet werden müssen wegen Center aligned */
		.ul_prescaler = PWM_CMR_CPRE_CLKA,
		/* Periode einstellen */
		.ul_period = PERIOD_VALUE,
		/* Duty cycle initial setzen */
		.ul_duty = INIT_DUTY_VALUE,
		/* der channel soll synchron sein */
		.b_sync_ch = true,
		 
		 .b_deadtime_generator = true,
		 
		 .us_deadtime_pwmh = 1,
		 
		 .us_deadtime_pwml = 1 
	};

	/* als erstes dann den Channel 0 initialisieren, indem nur das Channel Attribut in der Struktur von oben neu gesetzt wird,
	der REst bleibt gleich...
	*/
	sync_channel.channel = PWM_CHANNEL_0;
	pwm_channel_init(PWM, &sync_channel);
	// das gleiche dann mit den beiden anderen zu synchronisierenden Channels
	sync_channel.channel = PWM_CHANNEL_1;
	pwm_channel_init(PWM, &sync_channel);
	sync_channel.channel = PWM_CHANNEL_2;
	pwm_channel_init(PWM, &sync_channel);

	/*
	 * Initialize PWM synchronous channels
	 * Synchronous Update Mode: Automatic update duty cycle value by the PDC
	 * and automatic update of synchronous channels. The update occurs when
	 * the Update Period elapses (MODE 2 --> Vorsicht vor Verwirrung: dies entspricht der "Methode 3" aus dem Datasheet!).
	 * Synchronous Update Period = MAX_SYNC_UPDATE_PERIOD.
	 */
	pwm_sync_init(PWM, PWM_SYNC_UPDATE_MODE_2, MAX_SYNC_UPDATE_PERIOD);

	/*
	 * Request PDC transfer as soon as the synchronous update period elapses
	 */
	pwm_pdc_set_request_mode(PWM, PWM_PDC_UPDATE_PERIOD_ELAPSED, (1 << 0));
	
	/* Aktiviere alle synchronen channel durch aktivieren von channel 0, channel 1 und 2 werden automatisch synchron mit gestartet 
	zusätzlich noch den Refernzchannel "synchron" mitstarten
	die ASF funktion channel_enable erlaubt leider nur die Übergabe eines Kanals, also müssen wir hier wohl mit direktem Zugriff auf das Register arbeiten */
	pwm_channel_enable(PWM, PWM_CHANNEL_0);
  // ref channel funktioniert noch nicht, muss noch debugged werden  pwm_channel_enable(PWM, PWM_CHANNEL_3);
  
  pmc_enable_periph_clk(ID_PIOA);
  pio_set_output(PIOA, PIO_PA23, LOW, DISABLE, ENABLE);
  
  int angle = 0;
  int up = 1;
  float sinus = 0;
  float a;
  
	while (1) {
		
		pio_set(PIOA, PIO_PA23);
		for (int i = 0; i < 1000; i++)
		{
			sinus = sin(0.343543);
			a = sinus;
		}
		
		pio_clear(PIOA, PIO_PA23);
		delay_us(4);
		
		
		/*display_menu();
		uint32_t angle = get_num_value();
		struct SV pwm = get_vector_for_angle(angle);
		SVPWM(pwm.u, pwm.v, pwm.w);
		printf("The angle is %i degrees.", angle);
		
		for (int i = 0; i <= 360; i++) {
			struct SV pwm = get_vector_for_angle(i);
			SVPWM(pwm.u, pwm.v, pwm.w);
			printf("%i Grad\r\n", i);
			delay_ms(40);
			
		}
		for (int i = 360; i >= 0; i--) {
			struct SV pwm = get_vector_for_angle(i);
			SVPWM(pwm.u, pwm.v, pwm.w);
			printf("%i Grad\r\n", i);
			delay_ms(40);
			
		}*/
	}
}
