/*************************/
/* Definitions           */
/*************************/



// Define Brushless PWM Mode, uncomment ONE setting
#define PWM_32KHZ_PHASE  // Resolution 8 bit for PWM
// #define PWM_8KHZ_FAST    // Resolution 8 bit for PWM
//#define PWM_4KHZ_PHASE   // Resolution 8 bit for PWM
//#define NO_PWM_LOOP

#define MOTORUPDATE_FREQ 500                 // in Hz, 1000 is default
#define LOOPUPDATE_FREQ MOTORUPDATE_FREQ     // loop control sample rate equals motor update rate
#define DT_FLOAT (1.0/LOOPUPDATE_FREQ*1.024) // loop controller sample period dT
#define DT_INT_MS (1000/MOTORUPDATE_FREQ)    // dT, integer, (ms)
#define DT_INT_INV (MOTORUPDATE_FREQ)        // dT, integer, inverse, (Hz)


// LP filter coefficient
#define LOWPASS_K_FLOAT(TAU) (DT_FLOAT/(TAU+DT_FLOAT))

// Number of sinus values for full 360 deg.
// NOW FIXED TO 256 !!!
// Reason: Fast Motor Routine using uint8_t overflow for stepping
#define N_SIN 256

#define SCALE_ACC 10000.0
#define SCALE_PID_PARAMS 1000.0f

// specifie input for VCC/Ubat measurement
#define ADC_VCC_PIN A3 

#define UBAT_ADC_SCALE (5.0 / 1023.0)
// voltage divider 
#define UBAT_R1 10000.0
#define UBAT_R2 2200.0
#define UBAT_SCALE ( (UBAT_R1 + UBAT_R2) / UBAT_R2 )




// Hardware Abstraction for Motor connectors,
// DO NOT CHANGE UNLES YOU KNOW WHAT YOU ARE DOING !!!
#define PWM_A_MOTOR1 OCR2A
#define PWM_B_MOTOR1 OCR1B
#define PWM_C_MOTOR1 OCR1A

#define PWM_A_MOTOR0 OCR0A
#define PWM_B_MOTOR0 OCR0B
#define PWM_C_MOTOR0 OCR2B


#ifdef DEBUG
    #define DEBUG_PRINT(x) Serial.print(x)
    #define DEBUG_PRINTF(x, y) Serial.print(x, y)
    #define DEBUG_PRINTLN(x) Serial.println(x)
    #define DEBUG_PRINTLNF(x, y) Serial.println(x, y)
#else
    #define DEBUG_PRINT(x)
    #define DEBUG_PRINTF(x, y)
    #define DEBUG_PRINTLN(x)
    #define DEBUG_PRINTLNF(x, y)
#endif


#ifdef PWM_32KHZ_PHASE
  #define CC_FACTOR 32
#endif
#ifdef PWM_4KHZ_PHASE
  #define CC_FACTOR 4
#endif
#ifdef PWM_8KHZ_FAST
  #define CC_FACTOR 8
#endif
#ifdef NO_PWM_LOOP
  #define CC_FACTOR 1
#endif


#define LEDPIN_PINMODE             pinMode (8, OUTPUT);
#define LEDPIN_SWITCH              digitalWrite(8,!bitRead(PORTB,0));
#define LEDPIN_OFF                 digitalWrite(8, LOW);
#define LEDPIN_ON                  digitalWrite(8, HIGH);

// note: execution time for CH2_ON/CH2_OFF = 4 us
#define CH2_PINMODE                pinMode (4, OUTPUT);
#define CH2_OFF                    digitalWrite(4, LOW);
#define CH2_ON                     digitalWrite(4, HIGH);

#define CH3_PINMODE                pinMode (7, OUTPUT);
#define CH3_OFF                    digitalWrite(7, LOW);
#define CH3_ON                     digitalWrite(7, HIGH);

// enable stack and heapsize check (use just for debugging)
//#define STACKHEAPCHECK_ENABLE

