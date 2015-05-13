
#include <avr/interrupt.h>


//#define DEBUG                              // activate for DEBUG if needed


// -------------------------------------------------------------------------------------------------
// --- Achtung diese Zeilen NICHT ändern! ---
#define CENTER_DSM2    511		        // DSM2 HF-Modul Wert für Servo-Mitte
#define FALLING_EDGE   (0<<ICES3)
#define RISING_EDGE    (1<<ICES3)
// ============================= Anlagen spezifisch =========================================

// ----------------- allgemeines Timing für Futaba und andere Anlagen ------------------
#define EDGE_MODE		FALLING_EDGE
// ------------------ standard TX with 1,5ms +/- 0,4ms ----------------------
#define CENTER_PULSE	1500					
#define ABSMIN_PULSE   	(CENTER_PULSE - CENTER_DSM2)
#define ABSMAX_PULSE  	(CENTER_PULSE + CENTER_DSM2)
#define CONV_PPM2DSM	(tppm - ABSMIN_PULSE)
#define DEBUG_PIN1 9
#define DEBUG_PIN2 10


// ############################## don't change below this line ###############################################
// ===========================================================================================================
// ----- PPM Status ----- 
typedef enum {
    NULL_ST = -1, NOT_SYNCHED, ACQUIRING, READY, FAILSAFE
} State_t;

#define TICKS_PER_uS     (F_CPU /1000000L)    // number of timer ticks per 1 microsecond without prescaler
#define DE_JITTER        ((TICKS_PER_uS/2)+1)
#define MAX_CHANNELS      9                 // maximum number of channels we can store
#define MIN_IN_PULSE  ( 750 * TICKS_PER_uS) // valid pulse must be at least   750us
#define MAX_IN_PULSE  (2250 * TICKS_PER_uS) // valid pulse must be less than 2250us
#define SYNC_GAP_LEN  (3500 * TICKS_PER_uS) // we assume a space at least 3500us is sync
#define VALID_FRAMES      8                 // must have this many consecutive valid frames to transition to the ready state.

// ------------------------------------------ Data --------------------------------------------------
static int rawImpVal[MAX_CHANNELS + 1];     // array Impuls Rohwerte mit  tics/us
static int Pulses[  MAX_CHANNELS + 1];      // array holding channel pulses width value in microseconds
static int Failsafe[MAX_CHANNELS + 1];      // array holding channel fail safe values in microseconds
static byte ChannelNum = 0;                 // number of channels detected so far in the frame (first channel is 1)
static byte ChannelCnt = 0;                 // the total number of channels detected in a complete frame
static State_t State;                       // this will be one of the following states: Null, Not_Synched, Acquiring, Ready, Failsafe
static byte stateCount = 0;                 // counts the number of times this state has been repeated


static byte ppmERROR;					// Error Status and Failsave Holdcounter
static byte count;

// ------------------------------------------ ISR --------------------------------------------------

static volatile uint8_t	cntCompaOvf;	// CompA Overflow counter	

static volatile byte compa_faster = 0;
static volatile byte icp_faster = 0;

int servo1 = 6;
int servo2 = 7;
	

// ---- PPM 65ms timeout check -----
ISR(TIMER3_COMPA_vect) {	
   
  	// on Frame-Start cntCompaOvf=0 and OCR1A=ICR1
	if (cntCompaOvf > TICKS_PER_uS){				
	  if(State == READY) {
	    State = FAILSAFE;       // use fail safe values if signal lost
		ppmERROR = 4;			// remember Failsave
	    ChannelNum = 0;         // reset the channel count
	  }   
	}
	else
	  cntCompaOvf++;                                        
}                                           

/* ---------- ---------- ---------- Sync ---------- ---------- ---------- */
// inline - Sync Function
static void processSync() {                 // sync pulse was detected so reset the channel to first and update the system state
  if(State == READY) {                      
    if( ChannelNum != ChannelCnt){           // if the number of channels is unstable, go into failsafe
      State = FAILSAFE;           
	  ppmERROR = 4;							// remember Failsave	  
	}
  }                                         
  else {                                    
    if(State == NOT_SYNCHED) {              
      State = ACQUIRING;                    // this is the first sync pulse, we need one more to fill the channel data array
      stateCount = 0;                       
    } else {                                
      if(State == ACQUIRING) {             
        if(++stateCount > VALID_FRAMES) {
          State = READY;                    // this is the second sync and all channel data is ok so flag that channel data is valid
          ChannelCnt = ChannelNum;          // save the number of channels detected
        }                                   
      } else                                
        if( State == FAILSAFE) {            
          if(ChannelNum == ChannelCnt)      // did we get good pulses on all channels?
            State = READY;                  
        }                                   
    }                                       
  }                                         
  ChannelNum = 0;                           // reset the channel counter
}

                                            
// der Timer1 läuft immer durch, für genauere Messungen mit 125ns/62.5ns Auflösung
ISR(TIMER3_CAPT_vect) {  


                      

     



          digitalWrite(DEBUG_PIN1, HIGH);
         

	static uint16_t	ppm_ltime;				// last capture time
	uint16_t tdiff, tppm=0;         
	uint16_t ppm_atime = ICR1;				// actual capture time

	// CompA Overflow oder Pause > SYNC_GAP_LEN -> Synchronisierung
  	if( cntCompaOvf || (ppm_atime - ppm_ltime) >= SYNC_GAP_LEN){                  // is the space between pulses big enough to be the SYNC
	    processSync();  
	}
  	else{                                      
	    if(ChannelNum < MAX_CHANNELS) {         // check if its a valid channel pulse and save it
		    tdiff = ppm_atime - ppm_ltime;
	      	if((tdiff >= MIN_IN_PULSE) && (tdiff <= MAX_IN_PULSE)){  // check for valid channel data
		    ChannelNum++;
		    // Impuls-Jitter etwas filtern
		    tppm =rawImpVal[ChannelNum];  
		    if (tdiff > tppm+DE_JITTER){
			tppm = tdiff-DE_JITTER;
			rawImpVal[ChannelNum] = tppm;
		    }
		    else if (tdiff < tppm-DE_JITTER){
			tppm = tdiff+DE_JITTER;
			rawImpVal[ChannelNum] = tppm;
		    }
		}
	      	else{
		    if(State == READY) {
		        State = FAILSAFE;                 // use fail safe values if input data invalid
				ppmERROR = 4;					  // remember Failsave
		        ChannelNum = 0;                   // reset the channel count
		    }
		}
		if(State == READY){
		    tppm = tppm / TICKS_PER_uS;		  // umrechnen auf [us] Einheiten
		    // numerischen min/max Bereich eingrenzen
		    if (tppm < ABSMIN_PULSE)
			  tppm = ABSMIN_PULSE;
		    else if (tppm > ABSMAX_PULSE)
		        tppm = ABSMAX_PULSE;
		    // aktuellen, begrenzten Wert umrechnen und speichern
		    tppm = CONV_PPM2DSM;		// Umrechnung per #define 
		    Pulses[ChannelNum] = tppm ;	
		}
	    }
	}
	// beim letzten Impuls haben wir Zeit alle Impulswerte für den PPM-Error zu übernehmen
	if((State == READY) && (ChannelCnt == ChannelNum)) {	// nur bei Frames ohne Fehler
	    for (uint8_t i=1; i<=ChannelCnt; i++) 		// loop only aktiv Channels 
		Failsafe[i] = Pulses[i];		
	}
	// immer letzten aktuellen ICP Zeitpunkt festhalten
	ppm_ltime = ppm_atime;
	// reset Synchronisierung Überwachung
	cntCompaOvf = 0;				// reset Overflow counter
	OCR3A = ppm_atime;				// update Compare Register	
	TIFR3 = (1<<OCF3A);				// clear CompA isr flag

        digitalWrite(DEBUG_PIN1, LOW);
}


                                 
                                          
   uint16_t getChannelData(uint8_t channel) // this is the access function for channel data
  {     
  	uint16_t result = 85;           		// Failsafe -125% MotorAus default value
    if(State == READY) {
      cli();                            			// disable interrupts
      result = Pulses[channel];  		// return the last valid pulse width for this channel
      sei();                            			// enable interrupts
    }
	else{	// (State == FAILSAVE)
		if (channel > 0 ){			// immer Data-Hold außer Motor
			cli();                            		// disable interrupts
		    result = Failsafe[channel];  // return the channels failsafe value if set and State is Failsafe
			sei();                            		// enable interrupts
		}
		else{
			cli();                            		// disable interrupts
			if (ppmERROR > 1){		// Motor Hold counter
				ppmERROR--;
			    result = Failsafe[channel];  // return the channels failsafe value if set and State is Failsafe
			}
			sei();                            		// enable interrupts
		}
	}
  	  
  	return result;
  }


void setup() {
  delay(100);
  pinMode(DEBUG_PIN1, OUTPUT);
   pinMode(DEBUG_PIN2, OUTPUT);
   
    pinMode(servo1, OUTPUT);
  pinMode(servo2, OUTPUT);
  

// ---------------------------- Init Data ------------------------------
    ChannelNum = 0;                         
    State   = NOT_SYNCHED;     
     
	ppmERROR = 0;
    // set midpoint as default values for pulses and failsafe
    for(uint8_t ch = 0; ch <= MAX_CHANNELS; ch++) {
    	Failsafe[ch] = Pulses[ch] = CENTER_DSM2;     
    }

    // -------------------------- Init Timer3 with ICP --------------------  
    pinMode(13, INPUT);                      // Timer1 interrupt handler uses pin 8 as input, do not change it
    digitalWrite(13, HIGH);                  // !!!!! turn on internal pullup resistor on PPM pin
    // 16bit Timer1 runs with F_CPU tics/us!!!
    TCCR3A	= 0;				   // Normal port operation, OC1A/OC1B disconnected
    TCCR3B	= (1<<ICNC3)|(1<<CS30)|EDGE_MODE;  // icp noise filter,falling edge, clk/1 = 8tics/us
    
    TIMSK3  = (1<<ICIE3)|(1<<OCIE3A);  // enable input capture and output compareA+B
  

 
   
  
  
}

void servoPulse (int servo, int pwm)
{
 digitalWrite(servo, HIGH);
 digitalWrite(DEBUG_PIN2, HIGH);
 delayMicroseconds(pwm);
 digitalWrite(servo, LOW);
  digitalWrite(DEBUG_PIN2, LOW);
                 
}


void loop() {

 servoPulse(servo1, getChannelData(0));
 delay(20);

 
}// loop()


