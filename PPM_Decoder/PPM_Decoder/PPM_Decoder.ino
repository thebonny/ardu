/* **************************************************************************
*
*	PPM to Spektrum DX5e HF-Modul sketch by ingo_s
*
*	V2.0	03.12.2012 - only 22ms asynchron Mode
*   V2.1    11.12.2012 - add DX5e Binding Values
*   V2.2    13.12.2012 - add LED PPM-Error codes, add alternate AIN1 Input
*	V2.3	04.01.2013 - Binding bugfix 
*
*   history @ whoopsie
*           16.02.2013 - changes for use with standalone ATmega328
*						 . converted to AtmelStudio project with Arduino 1.0.3
*						 . changed GREEN_LED to STATUS_LED (PD6, ATmega328 pin 12)
*						 . fuse settings for flashing with AVRDUDE:
*						   -U lfuse:w:0xFF:m	 (default is -U lfuse:w:0x62:m)
*						   -U hfuse:w:0xDA:m	 (default is -U hfuse:w:0xD9:m)
*						   -U efuse:w:0x05:m (default is -U efuse:w:0xFF:m)
*
****************************************************************************/

/*
  PPM to Spektrum - only for 16 or 8MHz CPU Clock!!!
  Sends DSM2 signal using DX5e Spektrum TX module
  Some parts based on the Arduino code by daniel_arg

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

/*
	--- Hinweise zur Schaltung: ---
	Impuls Input geht an Input capture Pin (ICP) mit Schutzwiderstand,
	alternativ kann der Comperator Input benutzt werden (#define USE_COMPERATOR_INPUT ).
	Über einen Taster wird Binding und Reichweiten Mode aktiviert.
	Taster beim Einschalten gedrückt --> Binding Mode
	Taster beim Betrieb gedrückt     --> Reichweiten Mode
	
	Es wird nur eine LED verwendet für die Zustände:
	normal Mode		--> blinken im Sekundentakt
        normnal Mode, Fehler vorgekommen --> blinken im Sekundentakt, aber flackernd
	Binding Mode		--> schnell blinken
	Reichweiten Mode	--> langsam blinken
	PPM Input Fehler 	--> hektisch blinken

	Hinweis zum Timing:
	Wegen Problemen mit dem Spektrum Sende-Zylus Timing nicht nur bei mehr als 23ms, sondern auch
	bei ca. 18ms wird ab V2.0 wird nur noch das asynchrone 22ms Spektrum Timing benutzt.

	Je nach Senderfabrikat unterscheidet sich die Varianz des Servo-Impulses gegenüber Spektrum
	Für MPX ist deshalb eine Umrechnung 80% MPX auf 100% Spektrum Servoweg integriert.
	Für eine Umrechnung bei anderen Herstellern liegen noch keine Werte vor.
	Über Tabellen ist ein Mapping und die Invertierung von Kanälen vorgesehen.
*/

/*
	--- Hinweise zur Software: ---
	Um möglichst geringen Jitter zu haben, wird hier das PPM Timing mit 125ns Auflösung gemessen,
	mit anschliessender Jitter Filterung. Ziel ist, den Ausgangs-Wert an Spektrum mit [1us] Einheiten
	möglichst Jitterfrei zu halten, da bei 100% Weg nur 682 Schritte Auflösung vorhanden sind.
	Das PPM Eingangs-Signal wird auf Fehler mittels Überprüfung der Anzahl Kanäle und deren 
	Impulsbreite überprüft. Wird die Anzahl der Kanäle geändert, so muss neu eingeschaltet werden!
	Im Fehlerfall werden die letzten gültigen Werte in Verbindung mit Motor-Aus gesendet.
	( da könnte man noch eine Hold-Zeit für den Motor vorsehen )
*/

/*
	---- Hinweis zu Daten der DX5e beim Binden ----
	Die Daten beim Binden sind unabhängig von der momentanen Knüppelposition
        Gas -140%   Quer Mitte    Höhe Mitte   Seite Mitte  K5 -100%     K6 -100% 
	0x00, 0x00;  0x05, 0xFF;  0x09, 0xFF;  0x0D, 0xFF;  0x10, 0xAA;  0x14, 0xAA 
	eigentlich, nach Spektrum Angaben sollten sie flexibel sein, da beim Binden die Failsave Position
	angegeben wird. Das macht aber bei einigen BNF Produkten (mCPX V1) scheinbar Probleme.
*/


#include <avr/interrupt.h>


//#define DEBUG                              // activate for DEBUG if needed


// -------------------------------------- Settings ---------------------------------------------
// bei Binding Problemen mal mit den festen Werten der DX5e probieren, sonst auskommentieren mit "//"
#define sendDX5eBindData
// ---- bei kleinen PPM-Signalen den Comperator AIN1 (Pin7) Input benutzen, sonst auskommentieren mit "//" -----
//#define USE_COMPERATOR_INPUT              // mit 1,1V Schwelle für kleine Signale

// ---------- used Arduino Pins for LED and button switch ------------------------
#define BINDING_PIN        4                // Pin used to bind
//#define GREEN_LED         13                // Pin used for board LED
#define STATUS_LED         6                // Pin used for LED

// ----------------------------- activate your Spektrum HF-Modul ------------------
// ------ DX5e X1TX0 REVB / BRWDAMTX11 HF-Modul Codes ------
// ------ DX5e DSM2 Codes ------
//#define CODE_NORMAL_SENDING             0x00		// DSM2 2.4GHz france chanels only
//#define CODE_BINDING	                0x80
//#define CODE_LOW_POWER                  0x20
// ------ DX5e DSM2 Codes ------
//#define CODE_NORMAL_SENDING	        0x10		// DSM2 all 2.4GHz channels
//#define CODE_BINDING			0x90
//#define CODE_LOW_POWER	        0x30
// ------ DX5e DSMX/DSM2 Codes ------
#define	CODE_NORMAL_SENDING	0x18		// DSMX all 2.4GHz channels
#define CODE_BINDING		0x98
#define CODE_LOW_POWER		0x38
// ------ not testet MLP4DSM Codes -------------------------
//#define	CODE_NORMAL_SENDING	0x00		// MLP4DSM Codes
//#define CODE_BINDING			0x80
//#define CODE_LOW_POWER		0x20		// ?low power code is not testet?

// -------------------------------------------------------------------------------------------------
// --- Achtung diese Zeilen NICHT ändern! ---
#define CENTER_DSM2    511		        // DSM2 HF-Modul Wert für Servo-Mitte
#ifdef USE_COMPERATOR_INPUT                     // because of inverted Comperator
#define FALLING_EDGE   (1<<ICES1)               // Edge select must be inverted
#define RISING_EDGE    (0<<ICES1)
#else
#define FALLING_EDGE   (0<<ICES1)
#define RISING_EDGE    (1<<ICES1)
#endif
// ============================= Anlagen spezifisch =========================================

// ----------------- allgemeines Timing für Futaba und andere Anlagen ------------------
#define EDGE_MODE		FALLING_EDGE
// ------------------ standard TX with 1,5ms +/- 0,4ms ----------------------
#define CENTER_PULSE	1500					
#define ABSMIN_PULSE   	(CENTER_PULSE - CENTER_DSM2)
#define ABSMAX_PULSE  	(CENTER_PULSE + CENTER_DSM2)
#define CONV_PPM2DSM	(tppm - ABSMIN_PULSE)
// Channel Mapping
volatile uint8_t 	ChanIndex[]   = {3,1,2,4,5,6};    // Futaba DSM2 to PPM Channel Mapping
volatile uint8_t	ChanReverse[] = {0,0,0,0,0,0};

// ==========================================================================================
/*
// -------------------    Aurora A9 Timing      --------------------
#define EDGE_MODE		FALLING_EDGE          // on Trainer connector
// A9 +/- 125% Servoweg [500us] ==> +/- 125% Spektrum Servoweg [426us]
#define CENTER_PULSE	1500
#define ABSMIN_PULSE   	(CENTER_PULSE - 591)		// 591 = ((511 * 52) + 22) / 45
#define ABSMAX_PULSE  	(CENTER_PULSE + 591)	
#define CONV_PPM2DSM	((tppm - ABSMIN_PULSE) * 45 / 52)
// Channel Mapping
volatile  uint8_t 	ChanIndex[]   = {6,1,2,3,4,5};	// A9 DSM2 to PPM Channel Mapping
volatile uint8_t	ChanReverse[] = {0,1,0,0,1,0};	// Channel Reverse (Quer, Seite)
*/
// ==========================================================================================
// Multiplex Royal/SX und MC30x0 (with -10% Servo Adjust) timing
/*
// -------------------    MPX Timing      --------------------
#define EDGE_MODE		FALLING_EDGE    
// MPX +/- 100% Servoweg [550us] ==> +/- 125% Spektrum Servoweg [426us]
#define CENTER_PULSE	1500
#define ABSMIN_PULSE   	(CENTER_PULSE - 659)		// 659 = ((511 * 58) + 22) / 45
#define ABSMAX_PULSE  	(CENTER_PULSE + 659)	
#define CONV_PPM2DSM	((tppm - ABSMIN_PULSE) * 45 / 58)
// Channel Mapping
// Achtung Motor liegt auf Kanal 4, MPX belegt bei Heli aber Kanal 5 vor, Anpassen!!!
volatile uint8_t 	ChanIndex[]   = {4,1,2,3,5,6};	// normal DSM2 to PPM Channel Mapping
//volatile uint8_t 	ChanIndex[]   = {5,1,2,3,4,6};	// Heli   DSM2 to PPM Channel Mapping
volatile uint8_t	ChanReverse[] = {0,1,0,1,0,0};	// Channel Reverse (Quer, Seite)
*/

// ############################## don't change below this line ###############################################
// ===========================================================================================================
// ----- PPM Status ----- 
typedef enum {
    NULL_ST = -1, NOT_SYNCHED, ACQUIRING, READY, FAILSAFE
} State_t;

// ----- Sende Mode -----
typedef enum {
	NULL_MD = -1, SEND_DSM2, BINDING, LOW_PWR
} Mode_t;

#define TICKS_PER_uS     (F_CPU/1000000L)    // number of timer ticks per 1 microsecond without prescaler
#define DE_JITTER        ((TICKS_PER_uS/2)+1)
#define MAX_CHANNELS      9                 // maximum number of channels we can store
#define MIN_IN_PULSE  ( 750 * TICKS_PER_uS) // valid pulse must be at least   750us
#define MAX_IN_PULSE  (2250 * TICKS_PER_uS) // valid pulse must be less than 2250us
#define SYNC_GAP_LEN  (3500 * TICKS_PER_uS) // we assume a space at least 3500us is sync
#define VALID_FRAMES      8                 // must have this many consecutive valid frames to transition to the ready state.
#define DSM2_CHANNELS     6                 // Max number of DSM2 Channels transmitted 

// ------------------------------------------ Data --------------------------------------------------
static int rawImpVal[MAX_CHANNELS + 1];     // array Impuls Rohwerte mit  tics/us
static int Pulses[  MAX_CHANNELS + 1];      // array holding channel pulses width value in microseconds
static int Failsafe[MAX_CHANNELS + 1];      // array holding channel fail safe values in microseconds
static byte ChannelNum = 0;                 // number of channels detected so far in the frame (first channel is 1)
static byte ChannelCnt = 0;                 // the total number of channels detected in a complete frame
static State_t State;                       // this will be one of the following states: Null, Not_Synched, Acquiring, Ready, Failsafe
static Mode_t  Mode;			    // TX-Mode null, send_dsm2, binding, low_power
static byte stateCount = 0;                 // counts the number of times this state has been repeated

struct DSM2_Data {		            // TX Daten struct
	uint8_t Header[2];
	uint8_t	Channel[DSM2_CHANNELS*2];
} DSM2_Data;

// --- DX5e Daten im binding Mode ---
#ifdef sendDX5eBindData
// DX5e Bind Data
static byte DX5eBindData[(DSM2_CHANNELS*2)+2]={CODE_BINDING, 0x00, 0x00,0x00, 0x05,0xFF, 0x09,0xFF, 0x0D,0xFF, 0x10,0xAA, 0x14,0xAA}; 
// Bind Data von einem Sketch
//static byte DX5eBindData[(DSM2_CHANNELS*2)+2]={CODE_BINDING, 0x00, 0x00,0xaa, 0x05,0xff, 0x09,0xff, 0x0d,0xff, 0x13,0x54, 0x14,0xaa};
#endif

static uint8_t	flash_count;		    // LED flashing counter
static int  flash_duration ;                // flashing LED duration calc
static byte DSM2_Sent;                      // true if time to send
static byte ppmERROR;					// Error Status and Failsave Holdcounter
static byte count;
//static int  pulse;

// ------------------------------------------ ISR --------------------------------------------------

static volatile uint8_t	cntCompaOvf;	// CompA Overflow counter		

// ---- PPM 65ms timeout check -----
ISR(TIMER1_COMPA_vect) {		// on Frame-Start cntCompaOvf=0 and OCR1A=ICR1
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
ISR(TIMER1_CAPT_vect) {  
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
	OCR1A = ppm_atime;				// update Compare Register	
	TIFR1 = (1<<OCF1A);				// clear CompA isr flag
}

#define COMB_LOOPS  ((22000UL*TICKS_PER_uS)/65536L)
#define COMP_ADJ    ((22000UL*TICKS_PER_uS) - COMB_LOOPS*65536L)
// OCR1B macht das 22ms Spektrum Timing
ISR(TIMER1_COMPB_vect) {	
    static uint8_t	cntCompbOvf=0;
	
    // für 22ms sind n volle + eine Teilrunde notwendig
    if (++cntCompbOvf == COMB_LOOPS)
	OCR1B += COMP_ADJ;
    else if (cntCompbOvf > COMB_LOOPS){
	cntCompbOvf = 0;
	DSM2_Sent = 1;				// transmit!
    }
}

/* ---------- ---------- ---------- Class ---------- ---------- ---------- */
class PPM_Decode {

public:
  PPM_Decode() {                            // Constructor
    // empty                                
  }                                         
                                            
  void begin() {                            
    // ---------------------------- Init Data ------------------------------
    ChannelNum = 0;                         
    State   = NOT_SYNCHED;     
    Mode    = SEND_DSM2;  
	ppmERROR = 0;
    // set midpoint as default values for pulses and failsafe
    for(uint8_t ch = 0; ch <= MAX_CHANNELS; ch++) {
    	Failsafe[ch] = Pulses[ch] = CENTER_DSM2;     
    }

    // -------------------------- Init Timer1 with ICP --------------------  
    pinMode(8, INPUT);                      // Timer1 interrupt handler uses pin 8 as input, do not change it
    digitalWrite(8, HIGH);                  // !!!!! turn on internal pullup resistor on PPM pin
#ifdef USE_COMPERATOR_INPUT
    pinMode(7, INPUT);
    ACSR = (1<<ACBG)|(1<<ACIC);
#endif
    // 16bit Timer1 runs with F_CPU tics/us!!!
    TCCR1A	= 0;				   // Normal port operation, OC1A/OC1B disconnected
    TCCR1B	= (1<<ICNC1)|(1<<CS10)|EDGE_MODE;  // icp noise filter,falling edge, clk/1 = 8tics/us
    TIMSK1  = (1<<ICIE1)|(1<<OCIE1A)|(1<<OCIE1B);  // enable input capture and output compareA+B
  } // begin()

// ---------------------------------------- Functions --------------------------------------------------

  // --- Spektrum channel bezogene Werte mit Mapping auf die Input Daten holen ---
  uint16_t getChannelData(uint8_t channel) // this is the access function for channel data
  {     
  	uint16_t result = 85;           		// Failsafe -125% MotorAus default value
    if(State == READY) {
      cli();                            			// disable interrupts
      result = Pulses[ChanIndex[channel]];  		// return the last valid pulse width for this channel
      sei();                            			// enable interrupts
    }
	else{	// (State == FAILSAVE)
		if (channel > 0 ){			// immer Data-Hold außer Motor
			cli();                            		// disable interrupts
		    result = Failsafe[ChanIndex[channel]];  // return the channels failsafe value if set and State is Failsafe
			sei();                            		// enable interrupts
		}
		else{
			cli();                            		// disable interrupts
			if (ppmERROR > 1){		// Motor Hold counter
				ppmERROR--;
			    result = Failsafe[ChanIndex[channel]];  // return the channels failsafe value if set and State is Failsafe
			}
			sei();                            		// enable interrupts
		}
	}
  	// Servo-Reserve, weil Spektrum andere default Richtungen hat
  	if (ChanReverse[channel])
  		result = 1023 - result;
  
  	return result;
  }
};

PPM_Decode Receiver = PPM_Decode();

void setup() {
  delay(100);
  #ifdef DEBUG
  Serial.begin(115200);                     // print values on the screen
  #else
  Serial.begin(125000);                     // closest speed for DSM2 module, otherwise it won't work
  #endif
  // ------------- Init Ports ----------------
  pinMode(BINDING_PIN, INPUT);              // set binding pin
  digitalWrite(BINDING_PIN, HIGH);          // turn on internal pullup resistor on binding switch
  pinMode(STATUS_LED,   OUTPUT);             // define LED pin
  
  // LED blink once
  digitalWrite(STATUS_LED, HIGH);
  delay(100);
  digitalWrite(STATUS_LED, LOW);

// ---------- Init Timer1 with ICP ---------
  Receiver.begin();

  if(digitalRead(BINDING_PIN) == LOW)	// Binding aktivieren, wenn Taster gedrückt ist
    Mode = BINDING;
  
  count = 100;
  while(State != READY && count-- > 0) // wait 5 sec or until PPM data is stable and ready
    delay(50);

  DSM2_Sent = 0;
}

void loop() {
  // ------ check Binding switch -----
  if(digitalRead(BINDING_PIN) == HIGH)               // if not pushed, always SEND_DSM2 Mode
      Mode = SEND_DSM2;						
  // bei gedrücktem Schalter nur von SEND_DSM2 auf Reichweiten-Test umschalten
  if ( (Mode == SEND_DSM2) && !(digitalRead(BINDING_PIN) == HIGH) )
      Mode = LOW_PWR;
  // ---------------------------
      
  if(State == FAILSAFE)		               // bei PPM Fehler hektisch blinken
    flash_duration = 4;

  if((flash_count%flash_duration) < flash_duration/2){  // flash LED routine
	if (ppmERROR){
		if ((flash_count%4) < 3)
			digitalWrite(STATUS_LED, HIGH); else digitalWrite(STATUS_LED, LOW);
	}else
  	  digitalWrite(STATUS_LED, HIGH);
  }else
    digitalWrite(STATUS_LED, LOW);

  if(State == READY || State == FAILSAFE) {
    if(DSM2_Sent == 1) {                  	// if DSM2 frame is not sent yet
      sendDSM2();                         // send frame
      flash_count++;
    }
  }
}// loop()

void sendDSM2() {

  	if (Mode == SEND_DSM2){
      	DSM2_Data.Header[0] = CODE_NORMAL_SENDING;	// set normal ops header
		flash_duration = 200;						// ganz langsam blinken
	}
	else if (Mode == BINDING){
  		DSM2_Data.Header[0] = CODE_BINDING;			// set binding ops header
		flash_duration = 20;  
	}
	else{
  		DSM2_Data.Header[0] = CODE_LOW_POWER;       // set low power ops header
		flash_duration = 40;
	}
	// das könnte die Modellspeicher Nummer sein für Modellmatch, mal ausprobieren
  	DSM2_Data.Header[1] = 0x00;              		// second header byte  
  	//DSM2_Data.Header[1] = ChannelCnt;              	// second header byte  
	
        // alle 6 Channel Werte holen
	for (uint8_t i=0; i<DSM2_CHANNELS; i++) {	// get receiver data
		uint16_t temp = Receiver.getChannelData(i);
		DSM2_Data.Channel[i*2]   = (uint8_t)(i<<2) | (temp>>8);
		DSM2_Data.Channel[i*2+1] = temp;
	}
	DSM2_Sent = 0; 

#ifndef DEBUG
#ifdef sendDX5eBindData
        if (Mode == BINDING)
	// die 2 HeaderBytes und die Channel-Werte senden
          Serial.write(DX5eBindData, sizeof(DX5eBindData));
        else
#endif
	// die 2 HeaderBytes und die Channel-Werte senden
        Serial.write(DSM2_Data.Header, sizeof(DSM2_Data));
        
}

#else
    Serial.print(DSM2_Data.Header[0], HEX);
    Serial.print("   ");
    Serial.print(DSM2_Data.Header[1], HEX);
    Serial.print("   ");
    for(byte i=0; i < DSM2_CHANNELS; i++) { // print channels 1 to 6 in Dec
      Serial.print("K");
      Serial.print((DSM2_Data.Channel[i*2]>>2)+1, DEC);
      Serial.print(": ");
      Serial.print((((DSM2_Data.Channel[i*2]&0x03)<<8 | DSM2_Data.Channel[i*2+1])-512)/5, DEC);
      Serial.print("    ");
    }
    Serial.println(" ");
    delay(50);
}

void serialPrintHex(byte b) {
    byte b1 = (b >> 4) & 0x0F;
    byte b2 = (b & 0x0F);
    char c1 = (b1 < 10) ? ('0' + b1) : 'A' + b1 - 10;
    char c2 = (b2 < 10) ? ('0' + b2) : 'A' + b2 - 10;
    Serial.print(c1);
    Serial.print(c2);
}
#endif
