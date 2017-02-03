#include <SdFat.h>

#include "stdio.h"
#include "math.h"
#include "string.h"


// SD chip select pin
const uint8_t chipSelect = 10;

// file system object
SdFat sd;

// create Serial stream
ArduinoOutStream cout(Serial);

char fileName[] = "hapstik.txt";

const uint8_t spiSpeed = SPI_EIGHTH_SPEED;
#define error(s) sd.errorHalt_P(PSTR(s))

#define PULSE_DEBUG_LEVEL
//  #define CONSOLE_DEBUG_LEVEL
// #define PERFORMANCE_TRACE_LEVEL

typedef struct {
	int X;
	int Y;
} ADC_inputs;

typedef struct {
	float X;
	float Y;
	float Z;
	float PWM_u;
	float PWM_v;
	float PWM_w;
} space_vector;


// Mittelwertfilter

volatile int af_count_i	= 0;						// Laufvariable im AF-Array
volatile int	SUM_AF_i_1	= 0;						// Summe MOTOR1
volatile int	SUM_AF_i_2	= 0;						// Summe MOTOR2

volatile int	AF_A0_i		= 0;						// Average_Filterwert f�r ADC-Kanal A0
volatile float	AF_A0_f		= 0;
volatile int	AF_A1_i		= 0;						// Average_Filterwert f�r ADC-Kanal A0
volatile float	AF_A1_f		= 0;




#define	PI		3.141592654f
#define	WK1		(PI/180)
#define	WK2		(PI*2/3)
#define	WK3		(PI*4/3)

#define UPDATE_CONTROLLER_MILLIS 1  // recalculate all controllers within defined frequency 1/x
#define TICKS_PER_MILLISECOND 42000
#define PID_INTERRUPT_PRIORITY 3

#define ELECTRICAL_MECHANICAL_GEAR_FACTOR 7  // dies ist vom Motortyp (#Magnete etc.) abh�ngig

const float	ANGLE_OFFSET_X = 5.5;							// Winkel_Offset f�r Poti_MOTOR1 Vertikal		(Nullposition)	-> "+" Stick wandert nach unten
const float	ANGLE_OFFSET_Y = -16.0;

typedef struct  {
	float setpoint;
	float input;
	float lastInput;
	double ITerm;
	double outMax;
	double outMin;
	double kp;
	double ki;
	double kd;
} pid_controller;

pid_controller motor_X_position_controller = { 0.0, 0.0, 0.0, 0.0, 1.0, -1.0, 0.0015, 0.0, 0.0035 };
pid_controller motor_Y_position_controller = { 0.0, 0.0, 0.0, 0.0, 1.0, -1.0, 0.0015, 0.0, 0.0035 };
pid_controller motor_X_speed_controller = { 0.0, 0.0, 0.0, 0.0, 1.0, -1.0, 0.0005, 0.00125, 0.0 };
pid_controller motor_Y_speed_controller = { 0.0, 0.0, 0.0, 0.0, 1.0, -1.0, 0.0005, 0.00125, 0.0 };

space_vector motor_X_space_vector = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
space_vector motor_Y_space_vector = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

int volatile cnt_1ms_poll = 0;

typedef struct {
	int current_captured_ppm_value;
	int last_captured_ppm_value;
} rc_channel;




#define PPM_OFFSET 400


volatile rc_channel rc_channels[8];
static uint32_t gs_ul_captured_rb;

static int channel_id = 0;



#define TICKS_PER_uS 42
#define MIN_PWM_MICROS 600
#define MAX_PWM_MICROS 1600
#define MID_PWM_MICROS 1100
#define THROW_PWM_MICROS 500
#define PPM_PULSE_MICROS 400
#define PPM_FRAME_LENGTH_TOTAL_MICROS 20000 // 20ms frame




#define NUMBER_OF_RC_CHANNELS 6
#define PIN_PPM_OUT     PIN_TC0_TIOA0  // PIN TIOA1 which is PIN PA2 resp. "A7" on Due

volatile static uint32_t ppm_out_channels[NUMBER_OF_RC_CHANNELS];
volatile static unsigned int current_channel = 0;
volatile static uint32_t accumulated_frame_length = 0;

// PWM registers
#define REG_PWM_DIS      (*(volatile uint32_t*)0x40094008U) /**< \brief (PWM) PWM Disable Register */
#define REG_PWM_SCM      (*(volatile uint32_t*)0x40094020U) /**< \brief (PWM) PWM Sync Channels Mode Register */
#define REG_PWM_CMR0     (*(volatile uint32_t*)0x40094200U) /**< \brief (PWM) PWM Channel Mode Register (ch_num = 0) */
#define REG_PWM_CMR1     (*(volatile uint32_t*)0x40094220U) /**< \brief (PWM) PWM Channel Mode Register (ch_num = 1) */
#define REG_PWM_CMR2     (*(volatile uint32_t*)0x40094240U) /**< \brief (PWM) PWM Channel Mode Register (ch_num = 2) */
#define REG_PWM_CMR3     (*(volatile uint32_t*)0x40094260U) /**< \brief (PWM) PWM Channel Mode Register (ch_num = 3) */
#define REG_PWM_CMR4     (*(volatile uint32_t*)0x40094280U) /**< \brief (PWM) PWM Channel Mode Register (ch_num = 4) */
#define REG_PWM_CMR5     (*(volatile uint32_t*)0x400942A0U) /**< \brief (PWM) PWM Channel Mode Register (ch_num = 5) */
#define REG_PWM_CMR6     (*(volatile uint32_t*)0x400942C0U) /**< \brief (PWM) PWM Channel Mode Register (ch_num = 6) */

#define REG_PWM_CPRD0    (*(volatile uint32_t*)0x4009420CU) /**< \brief (PWM) PWM Channel Period Register (ch_num = 0) */
#define REG_PWM_CPRD1    (*(volatile uint32_t*)0x4009422CU) /**< \brief (PWM) PWM Channel Period Register (ch_num = 1) */
#define REG_PWM_CPRD2    (*(volatile uint32_t*)0x4009424CU) /**< \brief (PWM) PWM Channel Period Register (ch_num = 2) */
#define REG_PWM_CPRD3    (*(volatile uint32_t*)0x4009426CU) /**< \brief (PWM) PWM Channel Period Register (ch_num = 3) */
#define REG_PWM_CPRD4    (*(volatile uint32_t*)0x4009428CU) /**< \brief (PWM) PWM Channel Period Register (ch_num = 4) */
#define REG_PWM_CPRD5    (*(volatile uint32_t*)0x400942ACU) /**< \brief (PWM) PWM Channel Period Register (ch_num = 5) */
#define REG_PWM_CPRD6    (*(volatile uint32_t*)0x400942CCU) /**< \brief (PWM) PWM Channel Period Register (ch_num = 6) */

#define REG_PWM_CDTY0    (*(volatile uint32_t*)0x40094204U) /**< \brief (PWM) PWM Channel Duty Cycle Register (ch_num = 0) */
#define REG_PWM_CDTY1    (*(volatile uint32_t*)0x40094224U) /**< \brief (PWM) PWM Channel Duty Cycle Register (ch_num = 1) */
#define REG_PWM_CDTY2    (*(volatile uint32_t*)0x40094244U) /**< \brief (PWM) PWM Channel Duty Cycle Register (ch_num = 2) */
#define REG_PWM_CDTY3    (*(volatile uint32_t*)0x40094264U) /**< \brief (PWM) PWM Channel Duty Cycle Register (ch_num = 3) */
#define REG_PWM_CDTY4    (*(volatile uint32_t*)0x40094284U) /**< \brief (PWM) PWM Channel Duty Cycle Register (ch_num = 4) */
#define REG_PWM_CDTY5    (*(volatile uint32_t*)0x400942A4U) /**< \brief (PWM) PWM Channel Duty Cycle Register (ch_num = 5) */
#define REG_PWM_CDTY6    (*(volatile uint32_t*)0x400942C4U) /**< \brief (PWM) PWM Channel Duty Cycle Register (ch_num = 6) */

#define REG_PWM_CDTYUPD0 (*(volatile uint32_t*)0x40094208U) /**< \brief (PWM) PWM Channel Duty Cycle Update Register (ch_num = 0) */
#define REG_PWM_CDTYUPD1 (*(volatile uint32_t*)0x40094228U) /**< \brief (PWM) PWM Channel Duty Cycle Update Register (ch_num = 1) */
#define REG_PWM_CDTYUPD2 (*(volatile uint32_t*)0x40094248U) /**< \brief (PWM) PWM Channel Duty Cycle Update Register (ch_num = 2) */
#define REG_PWM_CDTYUPD3 (*(volatile uint32_t*)0x40094268U) /**< \brief (PWM) PWM Channel Duty Cycle Update Register (ch_num = 3) */
#define REG_PWM_CDTYUPD4 (*(volatile uint32_t*)0x40094288U) /**< \brief (PWM) PWM Channel Duty Cycle Update Register (ch_num = 4) */
#define REG_PWM_CDTYUPD5 (*(volatile uint32_t*)0x400942A8U) /**< \brief (PWM) PWM Channel Duty Cycle Update Register (ch_num = 5) */

#define REG_PWM_CMPM0    (*(volatile uint32_t*)0x40094138U) /**< \brief (PWM) PWM Comparison 0 Mode Register */
#define REG_PWM_CMPV0    (*(volatile uint32_t*)0x40094130U) /**< \brief (PWM) PWM Comparison 0 Value Register */
#define REG_PWM_ELMR     (*(volatile uint32_t*)0x4009407CU) /**< \brief (PWM) PWM Event Line 0 Mode Register */
#define REG_PIOC_PDR     (*(volatile uint32_t*)0x400E1204U) /**< \brief (PIOC) PIO Disable Register */
#define REG_PIOC_ABSR    (*(volatile uint32_t*)0x400E1270U) /**< \brief (PIOC) Peripheral AB Select Register */
#define REG_PWM_ENA      (*(volatile uint32_t*)0x40094004U) /**< \brief (PWM) PWM Enable Register */
#define REG_PWM_SCUC     (*(volatile uint32_t*)0x40094028U) /**< \brief (PWM) PWM Sync Channels Update Control Register */


//	ADC registers
#define REG_ADC_CDR6	(*(volatile  uint32_t*)0x400C0068U) // ADC Channel Data Register
#define REG_ADC_CDR7	(*(volatile  uint32_t*)0x400C006CU) // ADC Channel Data Register

// NVIC registers
#define ISER0  (*(volatile uint32_t*)0xE000E100U)
#define ICER0  (*(volatile uint32_t*)0xE000E180U)
#define ICPR0  (*(volatile uint32_t*)0xE000E280U)
#define ISER1  (*(volatile uint32_t*)0xE000E104U)
#define ICER1  (*(volatile uint32_t*)0xE000E184U)
#define ICPR1  (*(volatile uint32_t*)0xE000E284U)

typedef struct
{
	volatile uint32_t ISER[8];                 /*!< Offset: 0x000 (R/W)  Interrupt Set Enable Register           */
	uint32_t RESERVED0[24];
	volatile uint32_t ICER[8];                 /*!< Offset: 0x080 (R/W)  Interrupt Clear Enable Register         */
	uint32_t RSERVED1[24];
	volatile uint32_t ISPR[8];                 /*!< Offset: 0x100 (R/W)  Interrupt Set Pending Register          */
	uint32_t RESERVED2[24];
	volatile uint32_t ICPR[8];                 /*!< Offset: 0x180 (R/W)  Interrupt Clear Pending Register        */
	uint32_t RESERVED3[24];
	volatile uint32_t IABR[8];                 /*!< Offset: 0x200 (R/W)  Interrupt Active bit Register           */
	uint32_t RESERVED4[56];
	volatile uint8_t  IP[240];                 /*!< Offset: 0x300 (R/W)  Interrupt Priority Register (8Bit wide) */
	uint32_t RESERVED5[644];
	volatile  uint32_t STIR;                    /*!< Offset: 0xE00 ( /W)  Software Trigger Interrupt Register     */
}  NestedIC_Type;

#define SCS_BASE            (0xE000E000UL)                            /*!< System Control Space Base Address  */
#define NVIC_BASE           (SCS_BASE +  0x0100UL)                    /*!< NVIC Base Address                  */
#define __NVIC_PRIO_BITS       4      /**< SAM3X8H uses 4 Bits for the Priority Levels */
#define NestedIC                ((NestedIC_Type      *)     NVIC_BASE     )   /*!< NVIC configuration struct          */


// PIO registers
#define REG_PIOC_PDR	(*(volatile uint32_t*)0x400E1204U)
#define REG_PIOC_SODR   (*(volatile uint32_t*)0x400E1230U) /**< \brief (PIOC) Set Output Data Register */
#define REG_PIOC_CODR   (*(volatile uint32_t*)0x400E1234U) /**< \brief (PIOC) Clear Output Data Register */
#define REG_PIOC_PER    (*(volatile uint32_t*)0x400E1200U) /**< \brief (PIOC) PIO Enable Register */
#define REG_PIOC_OER    (*(volatile uint32_t*)0x400E1210U) /**< \brief (PIOC) Output Enable Register */


#define REG_PIOB_PUDR  (*(volatile uint32_t*)0x400E1060U)
#define REG_PIOB_MDDR  (*(volatile uint32_t*)0x400E1054U)
#define REG_PIOB_IFDR  (*(volatile uint32_t*)0x400E1024U)
#define REG_PIOB_SCIFSR (*(volatile uint32_t*)0x400E1080U)
#define REG_PIOB_ABSR  (*(volatile uint32_t*)0x400E1070U)
#define REG_PIOB_PDR   (*(volatile uint32_t*)0x400E1004U) /**< \brief (PIOB) PIO Disable Register */


// ADC registers
#define REG_ADC_CR		(*(volatile uint32_t*)0x400C0000U)
#define REG_ADC_MR		(*(volatile uint32_t*)0x400C0004U)
#define REG_ADC_CHER	(*(volatile uint32_t*)0x400C0010U)
#define REG_ADC_CGR		(*(volatile uint32_t*)0x400C0048U)
#define REG_ADC_IER		(*(volatile uint32_t*)0x400C0024U)

// PMC registers
#define PMC_PCER0		(*(volatile uint32_t*)0x400E0610U)
#define REG_PMC_PCER1	(*(volatile uint32_t*)0x400E0700U)


// TC0 channel 0 registers (PPM out)
#define TC0_CHANNEL0_CCR  (*(volatile uint32_t*)0x40080000U)
#define TC0_CHANNEL0_IER  (*(volatile uint32_t*)0x40080024U)
#define TC0_CHANNEL0_CMR  (*(volatile uint32_t*)0x40080004U)
#define TC0_CHANNEL0_RC   (*(volatile uint32_t*)0x4008001CU)
#define TC0_CHANNEL0_RA   (*(volatile uint32_t*)0x40080014U)
#define TC0_CHANNEL0_SR   (*(volatile uint32_t*)0x40080020U)

// TC0 channel 1 registers (PID controller update)
#define TC0_CHANNEL1_CCR  (*(volatile uint32_t*)0x40080040U)
#define TC0_CHANNEL1_IER  (*(volatile uint32_t*)0x40080064U)
#define TC0_CHANNEL1_CMR  (*(volatile uint32_t*)0x40080044U)
#define TC0_CHANNEL1_RC   (*(volatile uint32_t*)0x4008005CU)
#define TC0_CHANNEL1_SR   (*(volatile uint32_t*)0x40080060U)

// TC2 channel 0 registers (ppm capture)
#define TC2_CHANNEL0_CCR  (*(volatile uint32_t*)0x40088000U)
#define TC2_CHANNEL0_IER  (*(volatile uint32_t*)0x40088024U)
#define TC2_CHANNEL0_CMR  (*(volatile uint32_t*)0x40088004U)
#define TC2_CHANNEL0_SR   (*(volatile uint32_t*)0x40088020U)
#define TC2_CHANNEL0_RB   (*(volatile uint32_t*)0x40088018U)

// TC2 channel 1 registers (record playback)
#define TC2_CHANNEL1_CCR  (*(volatile uint32_t*)0x40088040U)
#define TC2_CHANNEL1_IER  (*(volatile uint32_t*)0x40088064U)
#define TC2_CHANNEL1_CMR  (*(volatile uint32_t*)0x40088044U)
#define TC2_CHANNEL1_RC   (*(volatile uint32_t*)0x4008805CU)
#define TC2_CHANNEL1_SR   (*(volatile uint32_t*)0x40088060U)

// TC control bits
#define TC_CMR_TCCLKS_TIMER_CLOCK1 (0x0u << 0)
#define TC_CMR_WAVE (0x1u << 15)
#define TC_CMR_ACPA_CLEAR (0x2u << 16)
#define TC_CMR_ACPC_SET (0x1u << 18)
#define TC_CMR_CPCTRG (0x1u << 14)
#define TC_SR_CPCS (0x1u << 4) /**< \brief (TC_SR) RC Compare Status */

#define TC_CMR_TCCLKS_TIMER_CLOCK3 (0x2u << 0) /**< \brief (TC_CMR) Clock selected: TCLK3 */
#define TC_CMR_LDRA_FALLING (0x2u << 16) /**< \brief (TC_CMR) Falling edge of TIOA */
#define TC_CMR_LDRB_RISING (0x1u << 18) /**< \brief (TC_CMR) Rising edge of TIOA */
#define TC_CMR_ABETRG (0x1u << 10) /**< \brief (TC_CMR) TIOA or TIOB External Trigger Selection */
#define TC_CMR_ETRGEDG_RISING (0x1u << 8) /**< \brief (TC_CMR) Rising edge */

// TC interrupt bits
#define TC_IER_LDRBS (0x1u << 6) /**< \brief (TC_IER) RB Loading */
#define TC_SR_LDRBS (0x1u << 6) /**< \brief (TC_SR) RB Loading Status */

static double PRECISION = 0.001;					//Anzahl Nachkommastellen, -> hier ohne Rundung!


#define RC_PER_MILLISECOND 42000
#define DEFAULT_FRAMERATE_MILLIS 20
#define NUMBER_OF_RECORDS 5000
#define MODE_RECORD 0
#define MODE_PLAYBACK 1
#define MODE_BYPASS 2

uint8_t mode = MODE_BYPASS;
uint8_t looping = 0;


uint16_t recorded_flight_records[NUMBER_OF_RECORDS][NUMBER_OF_RC_CHANNELS]; // we record only 5 channels (Gas, Pitch, Roll, Nick, Rudder), we use uint16 to reduce memory usage
int current_record = 0;
int max_recorded_record = 0;

void SetPriorityIRQ(uint32_t irq_id, uint32_t priority)
{
	NestedIC->IP[irq_id] = ((priority << (8 - __NVIC_PRIO_BITS)) & 0xff);
}

void DisableIRQ(uint32_t irq_id)
{
	NestedIC->ICER[(irq_id >> 5)] = (1 << (irq_id & 0x1F)); /* disable interrupt */
}

void ClearPendingIRQ(uint32_t irq_id)
{
	NestedIC->ICPR[(irq_id >> 5)] = (1 << (irq_id & 0x1F)); /* Clear pending interrupt */
}

void EnableIRQ(uint32_t irq_id)
{
	NestedIC->ISER[(irq_id >> 5)] = (1 << (irq_id & 0x1F)); /* enable interrupt */
}

uint32_t pin_to_mask(uint32_t pin)
{
	return 1U << (pin & 0x1F);
}


void set_master_framerate(int milliseconds) {
	TC2_CHANNEL1_RC = milliseconds * RC_PER_MILLISECOND;
}

void double_speed() {
	printf("Previous framerate was: %fms, framerate is now %ldms\r\n", TC2_CHANNEL1_RC / RC_PER_MILLISECOND, (TC2_CHANNEL1_RC/(RC_PER_MILLISECOND * 2)));
	TC2_CHANNEL1_RC = TC2_CHANNEL1_RC / 2;
}

void half_speed() {
	printf("Previous framerate was: %fms, framerate is now %ldms\r\n", TC2_CHANNEL1_RC / RC_PER_MILLISECOND, (TC2_CHANNEL1_RC/(RC_PER_MILLISECOND /2)));
	TC2_CHANNEL1_RC = TC2_CHANNEL1_RC * 2;
}

void start_record() {
	current_record = 0;
	mode = MODE_RECORD;
}

void stop_record() {
	mode = MODE_BYPASS;
	max_recorded_record = current_record;
}

void start_playback() {
	if (mode == MODE_RECORD) {
		stop_record();
	}
	looping = 0;
	current_record = 0;
	mode = MODE_PLAYBACK;
}

void loop_playback() {
	looping = 1;
	
}

ADC_inputs get_oversampled_adc_inputs(void) {
	
	AF_A0_f = SUM_AF_i_1 / af_count_i;							// Addition von 0,5 kann aber auch entfallen, da der Absolutwert nicht so wichtig ist,
	AF_A1_f = SUM_AF_i_2 / af_count_i;							// da ja alle Positionen beim Start des Sticks eingemessen werden
	AF_A0_i = AF_A0_f;										// Ganzzahliger Anteil wird �bergeben
	AF_A1_i = AF_A1_f;
	
	af_count_i = 0;											// Counter auf Anfang stellen
	
	SUM_AF_i_1 = 0;											// Summe wieder r�cksetzen
	SUM_AF_i_2 = 0;
	
	ADC_inputs inputs = { AF_A0_i, AF_A1_i };
	
	return inputs;
}


static void gpio_initialize(void)
{
	// init debug pins c.12 and c.14 for output
	REG_PIOC_PER	= REG_PIOC_PER		|		0x00005000u;
	REG_PIOC_OER	 = REG_PIOC_OER		|		0x00005000u;
}

void set_ppm_out_channel_value(int idx, int value) {
	ppm_out_channels[idx] = value;
}

void compute_space_vector_PWM(space_vector *sv) {
	if (sv->X >= 0) {
		if (sv->Y >= 0) {
			sv->PWM_w = (1 - sv->Y - sv->X)/2;
			sv->PWM_v = sv->PWM_w + sv->Y;
			sv->PWM_u = sv->PWM_v + sv->X;
		} else {
			if (sv->Z >= 0) {
				sv->PWM_v = (1 - sv->X - sv->Z)/2;
				sv->PWM_u = sv->PWM_v + sv->X;
				sv->PWM_w = sv->PWM_u + sv->Z;
			} else {
				sv->PWM_v = (1 + sv->Y + sv->Z)/2;
				sv->PWM_w = sv->PWM_v - sv->Y;
				sv->PWM_u = sv->PWM_w - sv->Z;
			}
		}
	} else {
		if (sv->Y >= 0) {
			if (sv->Z >= 0) {
				sv->PWM_u = (1 - sv->Z - sv->Y)/2;
				sv->PWM_w = sv->PWM_u + sv->Z;
				sv->PWM_v = sv->PWM_w + sv->Y;
			} else {
				sv->PWM_w = (1+sv->Z+sv->X)/2;
				sv->PWM_u = sv->PWM_w - sv->Z;
				sv->PWM_v = sv->PWM_u - sv->X;
			}
		} else {
			sv->PWM_u = (1+sv->X+sv->Y)/2;
			sv->PWM_v = sv->PWM_u - sv->X;
			sv->PWM_w = sv->PWM_v - sv->Y;
		}
	}
}

void update_pwm_duty_cycles(space_vector *sv_motor_X, space_vector *sv_motor_Y) {
		REG_PWM_CDTYUPD0 = (1 - sv_motor_X->PWM_u) * 2100;
		REG_PWM_CDTYUPD1 = (1 - sv_motor_X->PWM_v) * 2100;
		REG_PWM_CDTYUPD2 = (1 - sv_motor_X->PWM_w) * 2100;
		
		REG_PWM_CDTYUPD3 = (1 - sv_motor_Y->PWM_u) * 2100;
		REG_PWM_CDTYUPD4 = (1 - sv_motor_Y->PWM_v) * 2100;
		REG_PWM_CDTYUPD5 = (1 - sv_motor_Y->PWM_w) * 2100;		
		
		//	A Duty Cycle Update, �bernahme der Register Enable PWM channels (S.1016)
		//	- Register: PWM_SCUC (Sync Channel Update)
		//	- es gibt nur ein Bit in diesem Register:UPDULOCK (Unlock synchronous channels update)
		//	- wird es 1 gesetzt werden die Register f�r Duty Cycle ... �bernommen

		//	Ausgabe
		REG_PWM_SCUC = 0x00000001u;
}
	

void pwm_initialize(void)
{
	
/*A	
	Instanzen (S.38)
	- ID_PWM: 36 (Identifier f�r die Pulsweitenmodulation PWM)
	- Clock enable f�r PWM
		- S.563
		- Register: PMC_PCER1 (Power Management Controller Peripheral Clock Enable Register 1)
*/
	REG_PMC_PCER1 = REG_PMC_PCER1	|	0x00000010u;

/*A	
	Disable PWM channels (S.1008)
	- Register: PWM_DIS (PWM Disable)
	- Kanal 0: Motor1_X
	- Kanal 1: Motor1_Y	
	- Kanal 2: Motor1_Z
	
	- Kanal 3: Motor2_X
	- Kanal 4: Motor2_Y
	- Kanal 5: Motor2_Z	
	
	- Kanal 6: Referenzsignal
*/
	REG_PWM_DIS = REG_PWM_DIS		|	0x0000007Fu;

/*A 
	Clock einstellen
	- PWM_CLK (DIVA, PREA, DIVB, PREB)
	- PWM_CMRx 
	- entf�llt hier, da alle PWM-Kan�le mit MCK (84 MHz) getaktet werden
*/

/* 
	Synchronit�t, Kan�le 0,1,2,3,4,5 als synchrone Kan�le (S.1014) definieren:
	"They have the same source clock, the same period, the same alignment and are started together."
	
	- Register: PWM_SCM (Sync Channel Mode)

	- Synchronous Channels Update Mode2
	REG_PWM_SCM = REG_PWM_SCM		|	0x00020007u;
	- Synchronous Channels Update Mode0: "Manual write of double buffer registers and manual update of synchronous channels"
*/
	REG_PWM_SCM = REG_PWM_SCM		|	0x0000003Fu;

/* 
	Update Period Register (S.1017)
	Defines the time between each update of the synchronous channels if automatic trigger of the update is activated
*/
//	REG_PWM_SCUP = REG_PWM_SCUP		|	0x00000000u;

/*A 
	Kanal Mode, Aligment, Polarit�t, Dead Time (S.1044)
	- Register: PWM_CMRx (Channel Mode)
	- Dead Time:
		- �berlappungsfreiheit von komplement�ren Ausg�ngen, damit nicht beide MOSFETs kurz gleichzeitig eingeschalten
		- DTE=1 (Dead Time Generator Enable)
*/	
//	- Kanal 0:
//		- center aligned, Start mit high-Level, hier Dead Time enable f�r Ansteuerung Renesas-Board
//	REG_PWM_CMR0 = REG_PWM_CMR0		|	0x00010300u;	


//	- Kanal 0:
//		- center aligned, Start mit high-Level, hier Dead Time disable f�r Ansteuerung DS8313 (Leistungstreiber H-Br�cke)
	REG_PWM_CMR0 = REG_PWM_CMR0		|	0x00000300u;

//	- Kanal 1:
//		- center aligned, Start mit high-Level, Dead Time disabled
	REG_PWM_CMR1 = REG_PWM_CMR1		|	0x00000300u;

//	- Kanal 2:
//		- center aligned, Start mit high-Level, Dead Time disabled
	REG_PWM_CMR2 = REG_PWM_CMR2		|	0x00000300u;

//	- Kanal 3:
//		- center aligned, Start mit high-Level, Dead Time disabled
	REG_PWM_CMR3 = REG_PWM_CMR3		|	0x00000300u;

//	- Kanal 4:
//		- center aligned, Start mit high-Level, Dead Time disabled
	REG_PWM_CMR4 = REG_PWM_CMR4		|	0x00000300u;

//	- Kanal 5:
//		- center aligned, Start mit high-Level, Dead Time disabled
	REG_PWM_CMR5 = REG_PWM_CMR5		|	0x00000300u;

//	- Kanal 6:
//		- left   aligned, Start mit high-Level, Dead Time disabled
	REG_PWM_CMR6 = REG_PWM_CMR6		|	0x00000200u;

/*A 
	PWM Periode festlegen (S.1048)
	Erkl�rung: waveform period (S.978)
	- Register: PWM_CPRD
	- Periode = CPRD/84MHz = 8400/84MHz = 100us
	- Periode = CPRD/84MHz = 4200/84MHz =  50us		

*/

//	Werte f�r 50us:
	REG_PWM_CPRD0 = REG_PWM_CPRD0	|	0x00000834u;		// 2100
	REG_PWM_CPRD1 = REG_PWM_CPRD1	|	0x00000834u;		// 2100	
	REG_PWM_CPRD2 = REG_PWM_CPRD2	|	0x00000834u;		// 2100

	REG_PWM_CPRD3 = REG_PWM_CPRD3	|	0x00000834u;		// 2100
	REG_PWM_CPRD4 = REG_PWM_CPRD4	|	0x00000834u;		// 2100
	REG_PWM_CPRD5 = REG_PWM_CPRD5	|	0x00000834u;		// 2100
	
	REG_PWM_CPRD6 = REG_PWM_CPRD6	|	0x00001068u;		// 4200
	
		
/*
	PWM Duty Cycle (S.1046)
	- Register: PWM_CDTY (untere 16 Bit)
	- hier nur den initialen Wert einstellen
	- urspr�nglich:
		- PWM->PWM_CH_NUM[0].PWM_CDTY = (1 - DC_M1_X) * 4200;
		- da aber DC_M1_X = 0 kann man k�rzer schreiben:
	PWM->PWM_CH_NUM[0].PWM_CDTY = 4200;	
*/

	REG_PWM_CDTY0 = REG_PWM_CDTY0	|	0x00000834u;		// 2100
	REG_PWM_CDTY1 = REG_PWM_CDTY1	|	0x00000834u;		// 2100
	REG_PWM_CDTY2 = REG_PWM_CDTY2	|	0x00000834u;		// 2100	

	REG_PWM_CDTY3 = REG_PWM_CDTY3	|	0x00000834u;		// 2100
	REG_PWM_CDTY4 = REG_PWM_CDTY4	|	0x00000834u;		// 2100
	REG_PWM_CDTY5 = REG_PWM_CDTY5	|	0x00000834u;		// 2100
	
	REG_PWM_CDTY6 = REG_PWM_CDTY6	|	0x00000054u;		// 84	f�r 1us-Impuls (Referenzsignal f�r Oszi)

//	----------			
/* 
	Wir wollen nach dem Start eines SVPWM-Zyklus, 1us vergehen lassen und dann den ADC starten
	Das ist ein g�nstiger Zeitpunkt f�r die Strommessungen!!!
	
	PWM-Comparison Unit  (S.993)
		- Register: PWM Comparison x Mode Register (S.1042)
			- comparison x (x= 0...7, 8 PWM-Kan�le) erlauben, Bit CEN = 1
			- wir benutzen den Comparison 0
*/


//	Impuls auf der EventLine wird mit jedem 1. PWM-Referenzimpuls erzeugt (1  x 50us = 50us)
	REG_PWM_CMPM0 = REG_PWM_CMPM0	|	0x00000001u;

/*	
	PWM Comparison x Value Register (S.1040)
		- Register: PWM_CMPVx
		- comparison x value to be compared with the counter of the channel 0
		- wenn dieser Wert erreicht wird, wird "Comparison x" aktiv, was als Input f�r den Start des ADC genutzt wird
		- ACHTUNG:
			- der Wert wird auf 1us oder 12ns gesetzt 
				-> der optimale Abtastzeitpunkt muss mit dem Oszi ermittelt werden 
			- wenn also auf dem Oszi die fallende Flanke des Referenzsignals zu sehen ist, so wird in diesem Moment
			  der ADC gestartet -> ist hier noch anpassbar
		- Bit CVM: the comparison x between the counter of the channel 0 and the comparison x value is performed when this counter is
		  incrementing (bei center aligned, so ist ja Kanal 0 eingestellt, kann man hier auch "decrementing" w�hlen)	  
*/
	REG_PWM_CMPV0 = REG_PWM_CMPV0	|	0x00000001u;		// 0x00000054u -> 1us,  dann Start des ADC
															// 0x00000001u -> 12ns, dann Start des ADC (ist der kleinste Wert)	

/*A 	
	PWM Event Line x Register (S.995)
		- Register: PWM_ELMRx (S.1035)
		- a pulse is generated on the event line x when the comparison 0...7 match
		- wir benutzen den Comparison 0
*/
	REG_PWM_ELMR = REG_PWM_ELMR		|	0x00000001u;		// Comparison 0 ausgew�hlt
//	----------	

/*A 
	PWM-Leitungen (C.2, C.4, C.6, C.8, C.21, C.22, C.23) im Prozessor vom PIO-Controller trennen
	- Register: PIO_PDR (PIO Disable), S.634
	- zugeh�rige Bits auf 1 setzen
	- macht sich im Register: PIO_PSR (PIO Status) als 0 an den zugeh�renden Bits bemerkbar!
*/
//	REG_PIOC_PDR = REG_PIOC_PDR		|	0x000003FCu;		// -> Renesas
	REG_PIOC_PDR = REG_PIOC_PDR		|	0x00E00154u;		// -> DS8313

/*A 
	PWM-Leitungen (C.2, C.4, C.6, C.8, C.21, C.22, C.23) auf "peripheral B function" stellen
	- Register: PIO_ABSR (PIO AB Select), S.656
	- zugeh�rige Bits auf 1 setzen
*/
//	REG_PIOC_ABSR = REG_PIOC_ABSR	|	0x000003FCu;		// -> Renesas
	REG_PIOC_ABSR = REG_PIOC_ABSR	|	0x00E00154u;		// -> DS8313	

	
/*A 
	Enable PWM channels (S.1007)
	- gleichzeitige Start von Kanal 0 (steht stellvertretend f�r Kan�le 0...5 und Kanal 6 (Referenzimpuls), damit alle Kan�le 
	  den gleichen zeitlichen Nullpunkt haben.
	- initiale Ausgabe und Freischaltung der PWMs f�r Motor_1 und Motor_2:
*/
	REG_PWM_ENA = REG_PWM_ENA	|	0x00000041u; 

	//	Ausgabe an den PWM-VController

	REG_PWM_CDTYUPD0 = (1 - 0.5) * 2100;
	REG_PWM_CDTYUPD1 = (1 - 0.5) * 2100;
	REG_PWM_CDTYUPD2 = (1 - 0.5) * 2100;

	REG_PWM_CDTYUPD3 = (1 - 0.5) * 2100;
	REG_PWM_CDTYUPD4 = (1 - 0.5) * 2100;
	REG_PWM_CDTYUPD5 = (1 - 0.5) * 2100;
	//	A Duty Cycle Update, �bernahme der Register Enable PWM channels (S.1016)
	//	- Register: PWM_SCUC (Sync Channel Update)
	//	- es gibt nur ein Bit in diesem Register:UPDULOCK (Unlock synchronous channels update)
	//	- wird es 1 gesetzt werden die Register f�r Duty Cycle ... �bernommen

	//	Ausgabe
	REG_PWM_SCUC = 0x00000001u;
}


void copy_captured_channels_to_record() {
	for (int i = 0; i < NUMBER_OF_RC_CHANNELS; i++) {
		recorded_flight_records[current_record][i] = rc_channels[i].current_captured_ppm_value;
    
	}
	writeFile(rc_channels[1].current_captured_ppm_value);
}

void TC7_Handler(void) {
	// debug_pulse(1);
	if ((TC2_CHANNEL1_SR & TC_SR_CPCS) == TC_SR_CPCS) {
		if (mode == MODE_BYPASS || mode == MODE_RECORD) {
			for (int i = 0; i < NUMBER_OF_RC_CHANNELS; i++) {
				set_ppm_out_channel_value(i, rc_channels[i].current_captured_ppm_value);
			}
		}
		if (mode == MODE_RECORD) {
			copy_captured_channels_to_record();
			printf("Frame #: %d\r", current_record);
			current_record++;
			if (current_record >= NUMBER_OF_RECORDS) {
				stop_record();
				printf("REcording stopped as record limit is reached!\n\r");
			}
			} else if (mode == MODE_PLAYBACK) {
			for (int i = 0; i < NUMBER_OF_RC_CHANNELS; i++) {
				set_ppm_out_channel_value(i, recorded_flight_records[current_record][i]);
			}
			printf("Frame #: %d\r", current_record);
			current_record++;
			if (current_record >= max_recorded_record) {
				if (loop == 0) {
					mode = MODE_BYPASS;
					printf("Playback stopped as max recorded record %d is reached!\n\r", max_recorded_record);
					} else {
					// looped mode
					current_record = 0;
				}
				
			}
		}
	}
}





void record_playback_initialize(void)
{
	// enable clock for timer
	// TC2 channel 1 is peripheral ID = 34, second bit in PCER1
	REG_PMC_PCER1 |= (1 << 2);
	
	TC2_CHANNEL1_CCR |= 0x00000002u;
	TC2_CHANNEL1_CMR |= TC_CMR_TCCLKS_TIMER_CLOCK1	| TC_CMR_WAVE | TC_CMR_ACPA_CLEAR | TC_CMR_ACPC_SET | TC_CMR_CPCTRG;
	TC2_CHANNEL1_RC = DEFAULT_FRAMERATE_MILLIS * RC_PER_MILLISECOND;

	DisableIRQ(34);
	ClearPendingIRQ(34);
	SetPriorityIRQ(34, 5);
	EnableIRQ(34);
	
	// interrupt on rc compare
	TC2_CHANNEL1_IER |= 0x00000010u;
	// start tc2 channel 1
	TC2_CHANNEL1_CCR |= 0x00000005u;

}




void debug_pulse(int debug_pin) {
	#ifdef PULSE_DEBUG_LEVEL
	if (debug_pin == 0) {
		//	Debug_Pin C.12
		REG_PIOC_SODR	= REG_PIOC_SODR		|	0x00001000u; 		// Ausgang C12 =1 (DUE_Pin6)
		REG_PIOC_CODR	= REG_PIOC_CODR		|	0x00001000u; 		// Ausgang C12 =0 (DUE_Pin6)
		} else if (debug_pin == 1) {
		//	Debug_Pin C.24
		REG_PIOC_SODR	= REG_PIOC_SODR		|	0x00004000u; 		// Ausgang C14 =1 (DUE_Pin6)
		REG_PIOC_CODR	= REG_PIOC_CODR		|	0x00004000u; 		// Ausgang C14 =0 (DUE_Pin6)
	}
	#endif
}

void performance_trace_start(int debug_pin) {
	#ifdef PERFORMANCE_TRACE_LEVEL
	if (debug_pin == 0) {
		//	Debug_Pin C.12
		REG_PIOC_SODR	= REG_PIOC_SODR		|	0x00001000u; 		// Ausgang C12 =1 (DUE_Pin6)
		} else if (debug_pin == 1) {
		//	Debug_Pin C.24
		REG_PIOC_SODR	= REG_PIOC_SODR		|	0x00004000u; 		// Ausgang C14 =1 (DUE_Pin6)
	}
	#endif
}

void performance_trace_stop(int debug_pin) {
	#ifdef PERFORMANCE_TRACE_LEVEL
	if (debug_pin == 0) {
		//	Debug_Pin C.12
		REG_PIOC_CODR	= REG_PIOC_CODR		|	0x00001000u; 		// Ausgang C12 =0 (DUE_Pin6)
		} else if (debug_pin == 1) {
		//	Debug_Pin C.24
		REG_PIOC_CODR	= REG_PIOC_CODR		|	0x00004000u; 		// Ausgang C14 =0 (DUE_Pin6)
	}
	#endif
}


char * doubleToString(char *s, double n);

//	Double to ASCII
char * doubleToString(char *s, double n) {
	// handle special cases
	if (isnan(n)) {
		strcpy(s, "nan");
		} else if (isinf(n)) {
		strcpy(s, "inf");
		} else if (n == 0.0) {
		strcpy(s, "0");
		} else {
		int digit, m, m1;
		char *c = s;
		int neg = (n < 0);
		if (neg)
		n = -n;
		// calculate magnitude
		m = log10(n);
		int useExp = (m >= 14 || (neg && m >= 9) || m <= -9);
		if (neg)
		*(c++) = '-';
		// set up for scientific notation
		if (useExp) {
			if (m < 0)
			m -= 1.0;
			n = n / pow(10.0, m);
			m1 = m;
			m = 0;
		}
		if (m < 1.0) {
			m = 0;
		}
		// convert the number
		while (n > PRECISION || m >= 0) {
			double weight = pow(10.0, m);
			if (weight > 0 && !isinf(weight)) {
				digit = floor(n / weight);
				n -= (digit * weight);
				*(c++) = '0' + digit;
			}
			if (m == 0 && n > 0)
			*(c++) = '.';
			m--;
		}
		if (useExp) {
			// convert the exponent
			int i, j;
			*(c++) = 'e';
			if (m1 > 0) {
				*(c++) = '+';
				} else {
				*(c++) = '-';
				m1 = -m1;
			}
			m = 0;
			while (m1 > 0) {
				*(c++) = '0' + m1 % 10;
				m1 /= 10;
				m++;
			}
			c -= m;
			for (i = 0, j = m-1; i<j; i++, j--) {
				// swap without temporary
				c[i] ^= c[j];
				c[j] ^= c[i];
				c[i] ^= c[j];
			}
			c += m;
		}
		*(c) = '\0';
	}
	return s;
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    Float -> String   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    Float -> String   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//	Aus dem Internet: http://www.geeksforgeeks.org/convert-floating-point-number-string/

// reverses a string 'str' of length 'len'
void reverse(char *str, int len)
{
	int i=0, j=len-1, temp;
	while (i<j)
	{
		temp = str[i];
		str[i] = str[j];
		str[j] = temp;
		i++; j--;
	}
}

//	Converts a given integer x to string str[].  d is the number
//	of digits required in output. If d is more than the number
//	of digits in x, then 0s are added at the beginning.
int intToStr(int x, char str[], int d)
{
	int i = 0;
	while (x)
	{
		str[i++] = (x%10) + '0';
		x = x/10;
	}
	//	If number of digits required is more, then
	//	add 0s at the beginning
	while (i < d)
	str[i++] = '0';
	reverse(str, i);
	str[i] = '\0';
	return i;
}

void float_to_string(float n, char *Ergebnis, int afterpoint);

//	Converts a floating point number to string.
void float_to_string(float n, char *Ergebnis, int afterpoint)
{
	// Extract integer part
	int ipart = (int)n;
	// Extract floating part
	float fpart = n - (float)ipart;
	// convert integer part to string
	int i = intToStr(ipart, Ergebnis, 0);
	// check for display option after point
	if (afterpoint != 0)
	{
		Ergebnis[i] = ',';  // add dot
		// Get the value of fraction part upto given no.
		// of points after dot. The third parameter is needed
		// to handle cases like 233.007
		fpart = fpart * pow(10, afterpoint);
		
		intToStr((int)fpart, Ergebnis + i + 1, afterpoint);
	}
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    Float -> String   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx




void TC0_Handler(void) {
	// debug_pulse(1);
	uint32_t ra, rc;
	if ((TC0_CHANNEL0_SR & TC_SR_CPCS) == TC_SR_CPCS) {
		if (current_channel >= NUMBER_OF_RC_CHANNELS) {
			// calculate sync pulse pause
			uint32_t sync_pulse_micros = PPM_FRAME_LENGTH_TOTAL_MICROS - accumulated_frame_length;
			ra = sync_pulse_micros * TICKS_PER_uS;
			accumulated_frame_length = 0;
			current_channel = -1;
			} else {
			// pulse pause for single channel
			ra = ppm_out_channels[current_channel] * TICKS_PER_uS;
			accumulated_frame_length = accumulated_frame_length + ppm_out_channels[current_channel];
		}
		rc = ra + PPM_PULSE_MICROS * TICKS_PER_uS; // 300us Pulse
		accumulated_frame_length = accumulated_frame_length + PPM_PULSE_MICROS;
		TC0_CHANNEL0_RA = ra;
		TC0_CHANNEL0_RC = rc;
		current_channel++;
	}
}



void ppm_out_initialize(void)
{
	uint32_t rc;
	
	// initialize rc channels using PWM min value
	for (int i = 0; i < NUMBER_OF_RC_CHANNELS; i++) {
		ppm_out_channels[i] = MIN_PWM_MICROS;
	}
	
	// enable clock for TC0 channel 0 (ID 27)
	PMC_PCER0 |= (1 << 27);

	REG_PIOB_PUDR = pin_to_mask(25);
	REG_PIOB_MDDR  = pin_to_mask(25);
	REG_PIOB_IFDR = pin_to_mask(25);
	REG_PIOB_SCIFSR = pin_to_mask(25);
	REG_PIOB_ABSR |= pin_to_mask(25);
	
	REG_PIOB_PDR |= (1 << 25);
	
	TC0_CHANNEL0_CCR |= 0x00000002u;
	TC0_CHANNEL0_CMR |= TC_CMR_TCCLKS_TIMER_CLOCK1	| TC_CMR_WAVE | TC_CMR_ACPA_CLEAR | TC_CMR_ACPC_SET | TC_CMR_CPCTRG;
	
	TC0_CHANNEL0_RC = TICKS_PER_uS * PPM_FRAME_LENGTH_TOTAL_MICROS;
	
	DisableIRQ(27);
	ClearPendingIRQ(27);
	SetPriorityIRQ(27, 0);
	EnableIRQ(27);

	// interrupt on rc compare
	TC0_CHANNEL0_IER |= 0x00000010u;
	// start tc0 channel 0
	TC0_CHANNEL0_CCR |= 0x00000005u;
}



void TC6_Handler(void)
{
	// debug_pulse(1);
	if ((TC2_CHANNEL0_SR & TC_SR_LDRBS) == TC_SR_LDRBS) {
		uint16_t micros = 0;
		gs_ul_captured_rb = TC2_CHANNEL0_RB;
		micros = (gs_ul_captured_rb * 8) / 21; // clock is Master Clock with prescaler 32 at 84 MHz Master Clock
		if (micros > 3000) {
			// PPM sync pulse, recount channels starting with 0
			channel_id = 0;
			//	printf("Current: %d\r\n", rc_channels[1].current_captured_ppm_value);
			//	printf("Last: %d\r\n", rc_channels[1].last_captured_ppm_value);
			return;
		}
		rc_channels[channel_id].last_captured_ppm_value = rc_channels[channel_id].current_captured_ppm_value;
		rc_channels[channel_id].current_captured_ppm_value = micros - PPM_OFFSET;

		channel_id++;
	}
}

int get_interpolated_channel_ppm(int channel_id, int step) {
	int delta = (rc_channels[channel_id].current_captured_ppm_value - rc_channels[channel_id].last_captured_ppm_value) / 20;
	return rc_channels[channel_id].last_captured_ppm_value + (delta * step);
}

void ppm_capture_initialize(void)
{
	
	for (int i = 0; i < 8; i++)
	{
		rc_channels[i].current_captured_ppm_value = 0;
		rc_channels[i].last_captured_ppm_value = 0;
	}
	
	REG_PMC_PCER1 = REG_PMC_PCER1 | 0x00000002u;

	TC2_CHANNEL0_CCR = TC2_CHANNEL0_CCR | 0x00000002u;
	TC2_CHANNEL0_CMR = TC_CMR_TCCLKS_TIMER_CLOCK3 | TC_CMR_LDRA_FALLING | TC_CMR_LDRB_RISING 	| TC_CMR_ABETRG | TC_CMR_ETRGEDG_RISING;
	
	DisableIRQ(33);
	ClearPendingIRQ(33);
	
	SetPriorityIRQ(33, 15);
	EnableIRQ(33);
	
	// enable interrupt
	TC2_CHANNEL0_IER = TC2_CHANNEL0_IER | TC_IER_LDRBS;
	// start tc2 channel 0
	TC2_CHANNEL0_CCR = TC2_CHANNEL0_CCR | 0x00000005u;
	
	
}



double pid_compute(pid_controller *controller)
{
	float error = controller->setpoint - controller->input;
	controller->ITerm += (controller->ki * error);
	
	if(controller->ITerm > controller->outMax) {
		controller->ITerm = controller->outMax;
		} else if(controller->ITerm < controller->outMin) {
		controller->ITerm = controller->outMin;
	}
	float dInput = (controller->input - controller->lastInput);
	
	// Reglerausgang berechnen
	double output = controller->kp * error + controller->ITerm - controller->kd * dInput;

	// �berl�ufe kappen
	if(output > controller->outMax) {
		output = controller->outMax;
		} else if(output < controller->outMin) {
		output = controller->outMin;
	}
	//	letzten Wert speichern
	controller->lastInput = controller->input;
	return output;
}

void compute_space_vector_components(space_vector *sv, float rotation_angle, float power_factor) {
	sv->X = power_factor * cos(rotation_angle * WK1);
	sv->Y = power_factor * cos(rotation_angle * WK1-WK2);
	sv->Z = power_factor * cos(rotation_angle * WK1-WK3);
}


void compute_space_vector_motor_X(space_vector *sv, int input, float power_factor) {
	float rotation_angle = ELECTRICAL_MECHANICAL_GEAR_FACTOR * ((33 * (input - 2418) / 1677.0) + ANGLE_OFFSET_X);
	if (power_factor >= 0) {
		rotation_angle += 90;
		compute_space_vector_components(sv, rotation_angle, power_factor);
		} else {
		rotation_angle -= 90;
		compute_space_vector_components(sv, rotation_angle, (-1) * power_factor);
	}
}

void compute_space_vector_motor_Y(space_vector *sv, int input, float power_factor) {
	float rotation_angle = ELECTRICAL_MECHANICAL_GEAR_FACTOR * ((33 * (input - 2304) / 1641.0) + ANGLE_OFFSET_Y);
	if (power_factor >= 0) {
		rotation_angle += 90;
		compute_space_vector_components(sv, rotation_angle, power_factor);
		} else {
		rotation_angle -= 90;
		compute_space_vector_components(sv, rotation_angle, (-1) * power_factor);
	}
}


void compute_all_controllers(void) {
	ADC_inputs inputs = get_oversampled_adc_inputs();
	motor_X_position_controller.input = inputs.X;
	compute_space_vector_motor_X(&motor_X_space_vector, inputs.X, pid_compute(&motor_X_position_controller));

	motor_Y_position_controller.input = inputs.Y;
	compute_space_vector_motor_Y(&motor_Y_space_vector, inputs.Y, pid_compute(&motor_Y_position_controller));
	
	compute_space_vector_PWM(&motor_X_space_vector);
	compute_space_vector_PWM(&motor_Y_space_vector);
	
	update_pwm_duty_cycles(&motor_X_space_vector, &motor_Y_space_vector);
}

static void display_debug_output() {
	#ifdef CONSOLE_DEBUG_LEVEL
	char s1[32], s2[32], s3[32];
	printf("| X1      : %15s| Y1      : %15s| Z1  : %15s\r\n",
	doubleToString(s1, motor_X_space_vector.X), doubleToString(s2, motor_X_space_vector.Y), doubleToString(s3, motor_X_space_vector.Z));
	printf("| Sollwert      : %15s\r\n",
	doubleToString(s1, motor_X_position_controller.setpoint));
	printf("\r\n");
	printf("| X2      : %15s| Y2      : %15s| Z2  : %15s\r\n",
	doubleToString(s1, motor_Y_space_vector.X), doubleToString(s2, motor_Y_space_vector.Y), doubleToString(s3, motor_Y_space_vector.Z));
	printf("| Sollwert      : %15s\r\n",
	doubleToString(s1, motor_Y_position_controller.setpoint));
	printf("------------\r\n");
	#endif
}



void TC1_Handler(void) {
	if ((TC0_CHANNEL1_SR & TC_SR_CPCS) == TC_SR_CPCS) {
		debug_pulse(1);
		//	debug_pulse(1);
		cnt_1ms_poll++;

		motor_Y_position_controller.setpoint = get_interpolated_channel_ppm(1, cnt_1ms_poll % 20);
		motor_X_position_controller.setpoint = get_interpolated_channel_ppm(2, cnt_1ms_poll % 20);
		
		//		printf("IP: %d\r\n", get_interpolated_channel_ppm(1, cnt_1ms_poll % 20));

		
		performance_trace_start(0);
		compute_all_controllers();
		performance_trace_stop(0);
		
		if (cnt_1ms_poll % 1000 == 0) {
			display_debug_output();
		}
		
	}
}




void pid_initialize(void)
{
	uint32_t rc;
	// enable clock for timer
	PMC_PCER0 = PMC_PCER0 | 0x10000000u;
	
	TC0_CHANNEL1_CCR = TC0_CHANNEL1_CCR | 0x00000002u;
	TC0_CHANNEL1_CMR = TC0_CHANNEL1_CMR | TC_CMR_TCCLKS_TIMER_CLOCK1	| TC_CMR_WAVE | TC_CMR_ACPA_CLEAR | TC_CMR_ACPC_SET | TC_CMR_CPCTRG;
	
	TC0_CHANNEL1_RC = TICKS_PER_MILLISECOND * UPDATE_CONTROLLER_MILLIS;

	DisableIRQ(28);
	ClearPendingIRQ(28);
	SetPriorityIRQ(28, PID_INTERRUPT_PRIORITY);
	EnableIRQ(28);
	
	// interrupt on rc compare
	TC0_CHANNEL1_IER = TC0_CHANNEL1_IER | 0x00000010u;
	// start tc0 channel 1
	TC0_CHANNEL1_CCR = TC0_CHANNEL1_CCR | 0x00000005u;
	
}




void ADC_Handler(void)
{		
	debug_pulse(0);
	SUM_AF_i_1 += REG_ADC_CDR7;						// Summenbildung: aktueller ADC_A0-Wert wird dazu addiert
	SUM_AF_i_2 += REG_ADC_CDR6;						// Summenbildung: aktueller ADC_A1-Wert wird dazu addiert
	af_count_i ++;												// "af" -> Average-Filter
}

void adc_initialize(void)
{
/*A 
	ADC-Leitungen (C.2 - C.9) im Prozessor vom PIO-Controller trennen
	- Register: PIO_PDR (PIO Disable), S.634
	- zugeh�rige Bits auf 1 setzen
	- macht sich im Register: PIO_PSR (PIO Status) als 0 an den zugeh�renden Bits bemerkbar!
*/
	REG_PIOC_PDR = REG_PIOC_PDR		|	0x000003FCu;


/*	Clock enable f�r ADC
	Instanzen (S.38)
	- ID_ADC: 37 (Identifier f�r die ADC)
		- S.563
		- Register: PMC_PCER1 (Power Management Controller Peripheral Clock Enable Register 1)
*/
	REG_PMC_PCER1 = REG_PMC_PCER1	|	0x00000020u;

//	Reset ADC (S.1332)	
	REG_ADC_CR = 0x00000001;	


//	12 Bit / x4  -> ADC-Werte: 21 (2926 ... 2946)
//	Verteilung VW_MW = 2936;

	REG_ADC_MR = REG_ADC_MR			| 0x1E900209u;				// LOWRES von 1 auf 0 (10Bit -> 12Bit)
//	12 Bit / x4  -> ADC-Werte: 10 (2931 ... 2940) nach 1000 Messungen

	REG_ADC_CHER = REG_ADC_CHER		| 0x000000E0u;

	REG_ADC_CGR = REG_ADC_CGR		| 0x0000F000u;		// Gain = 1 f�r CH5 (A2) -> Poti zur Spollwertvorgabe
														// Gain = 4 f�r CH6 (A1)
														// Gain = 4 f�r CH7 (A0)
														// Gain = 1	f�r restliche Analogeing�nge 													


	REG_ADC_IER = REG_ADC_IER | 0x00000080u;
	DisableIRQ(37);
	ClearPendingIRQ(37);
 	SetPriorityIRQ(37, 0);
	EnableIRQ(37);



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



void setup()
{

  Serial.begin(115200);
	// init core HAPStik Modules
	gpio_initialize();
	pwm_initialize();
	adc_initialize();
	pid_initialize();

	// init peripheral modules to support rc Tx and simulator playback
	ppm_capture_initialize();
	ppm_out_initialize();
	record_playback_initialize();

  delay(400);  // catch Due reset problem
  
  // initialize the SD card at SPI_HALF_SPEED to avoid bus errors with
  // breadboards.  use SPI_FULL_SPEED for better performance
  if (!sd.begin(chipSelect, SPI_EIGHTH_SPEED)) {
    printf("Error init!\r\n");
  }


  
	display_menu();
}

void writeFile(int ppm_value) {

    // create or open and truncate output file
  ofstream sdout(fileName);
  char str[5];
  // write file from string stored in flash
  sdout << pstr(itoa(ppm_value, str, 10)) << flush;
  sdout << pstr("\r\n") << flush;

  // check for any errors
  if (!sdout) printf("writeFile Error!");
    sdout.close();

}

void loop()
{


      
		char key;
		if (Serial.available() > 0) {
        // read the incoming byte:
          key = Serial.read();
		

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
		
		
		

