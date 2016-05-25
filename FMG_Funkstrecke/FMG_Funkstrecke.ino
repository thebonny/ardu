#include <avr/interrupt.h>


// -------------------------------------- Settings ---------------------------------------------
// bei Binding Problemen mal mit den festen Werten der DX5e probieren, sonst auskommentieren mit "//"
#define sendDX5eBindData
// ---- bei kleinen PPM-Signalen den Comperator AIN1 (Pin7) Input benutzen, sonst auskommentieren mit "//" -----
//#define USE_COMPERATOR_INPUT              // mit 1,1V Schwelle f�r kleine Signale

// ---------- used Arduino Pins for LED and button switch ------------------------
#define BINDING_PIN        4                // Pin used to bin
#define STATUS_LED         3                // Pin used for LED


#define TICKS_PER_uS     (F_CPU/1000000L)

#define	CODE_NORMAL_SENDING	0x18		// DSMX all 2.4GHz channels
#define CODE_BINDING		0x98
#define CODE_LOW_POWER		0x38

// -------------------------------------------------------------------------------------------------
// --- Achtung diese Zeilen NICHT �ndern! ---
#define CENTER_DSM2    511		        // DSM2 HF-Modul Wert f�r Servo-Mitte

// ----- Sende Mode -----
typedef enum {
  NULL_MD = -1, SEND_DSM2, BINDING, LOW_PWR
} Mode_t;

#define DSM2_CHANNELS     6                 // Max number of DSM2 Channels transmitted 

// ------------------------------------------ Data --------------------------------------------------
static Mode_t  Mode;			    // TX-Mode null, send_dsm2, binding, low_power
struct DSM2_Data {		            // TX Daten struct
  uint8_t Header[2];
  uint8_t	Channel[DSM2_CHANNELS * 2];
} DSM2_Data;

// --- DX5e Daten im binding Mode ---
#ifdef sendDX5eBindData
// DX5e Bind Data
static byte DX5eBindData[(DSM2_CHANNELS * 2) + 2] = {CODE_BINDING, 0x00, 0x00, 0x00, 0x05, 0xFF, 0x09, 0xFF, 0x0D, 0xFF, 0x10, 0xAA, 0x14, 0xAA};
// Bind Data von einem Sketch
//static byte DX5eBindData[(DSM2_CHANNELS*2)+2]={CODE_BINDING, 0x00, 0x00,0xaa, 0x05,0xff, 0x09,0xff, 0x0d,0xff, 0x13,0x54, 0x14,0xaa};
#endif

static uint8_t	flash_count;		    // LED flashing counter
static int  flash_duration ;                // flashing LED duration calc
static byte DSM2_Sent;                      // true if time to send
static byte count;
int timer1_counter;

// poti
int potPin = 2;    // select the input pin for the potentiometer
int ledPin = 13;   // select the pin for the LED
int val = 0;       // variable to store the value coming from the sensor

void setup() {
  pinMode(ledPin, OUTPUT);  // declare the ledPin as an OUTPUT
  delay(100);
  Serial1.begin(125000);                     // closest speed for DSM2 module, otherwise it won't work
  // ------------- Init Ports ----------------
  pinMode(BINDING_PIN, INPUT);              // set binding pin
  digitalWrite(BINDING_PIN, HIGH);          // turn on internal pullup resistor on binding switch
  pinMode(STATUS_LED,   OUTPUT);             // define LED pin

  // LED blink once
  digitalWrite(STATUS_LED, HIGH);
  delay(100);
  digitalWrite(STATUS_LED, LOW);


 // initialize timer1 
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;

  // Set timer1_counter to the correct value for our interrupt interval
  //timer1_counter = 64886;   // preload timer 65536-16MHz/256/100Hz
  timer1_counter = 62786;   // preload timer 65536-8MHz/64/45,454545Hz 22ms
  // timer1_counter = 34286;   // preload timer 65536-16MHz/256/2Hz
  
  
  TCNT1 = timer1_counter;   // preload timer
  TCCR1B |= (1 << CS11) | (1 << CS10) ;    // 256 prescaler 
  TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
  interrupts();             // enable all interrupts
  
  if (digitalRead(BINDING_PIN) == LOW)	// Binding aktivieren, wenn Taster gedr�ckt ist
    Mode = BINDING;

  count = 100;
  DSM2_Sent = 0;

  
}

ISR(TIMER1_OVF_vect)        // interrupt service routine 
{
  
  TCNT1 = timer1_counter;   // preload timer
  DSM2_Sent = 1;
}

void loop() {
  
   val = analogRead(potPin);    // read the value from the sensor
               // stop the program for some time
  
  
  // ------ check Binding switch -----
  if (digitalRead(BINDING_PIN) == HIGH)              // if not pushed, always SEND_DSM2 Mode
    Mode = SEND_DSM2;
  // bei gedr�cktem Schalter nur von SEND_DSM2 auf Reichweiten-Test umschalten
  if ( (Mode == SEND_DSM2) && !(digitalRead(BINDING_PIN) == HIGH) )
    Mode = LOW_PWR;
  // ---------------------------


  if ((flash_count % flash_duration) < flash_duration / 2) { // flash LED routine
      digitalWrite(STATUS_LED, HIGH);
  } else
    digitalWrite(STATUS_LED, LOW);


  if (DSM2_Sent == 1) {                  	// if DSM2 frame is not sent yet
    sendDSM2();                         // send frame
    flash_count++;
  }

}// loop()

void sendDSM2() {

  
  if (Mode == SEND_DSM2) {
    DSM2_Data.Header[0] = CODE_NORMAL_SENDING;	// set normal ops header
    flash_duration = 200;						// ganz langsam blinken
  }
  else if (Mode == BINDING) {
    DSM2_Data.Header[0] = CODE_BINDING;			// set binding ops header
    flash_duration = 20;
  }
  else {
    DSM2_Data.Header[0] = CODE_LOW_POWER;       // set low power ops header
    flash_duration = 40;
  }
  // das k�nnte die Modellspeicher Nummer sein f�r Modellmatch, mal ausprobieren
  DSM2_Data.Header[1] = 0x00;              		// second header byte
  //DSM2_Data.Header[1] = ChannelCnt;              	// second header byte

  // alle 6 Channel Werte holen
  for (uint8_t i = 0; i < DSM2_CHANNELS; i++) {	// get receiver data
    uint16_t temp = val;
    DSM2_Data.Channel[i * 2]   = (uint8_t)(i << 2) | (temp >> 8);
    DSM2_Data.Channel[i * 2 + 1] = temp;
  }
  DSM2_Sent = 0;



  if (Mode == BINDING)
    // die 2 HeaderBytes und die Channel-Werte senden
    Serial1.write(DX5eBindData, sizeof(DX5eBindData));
  else
    // die 2 HeaderBytes und die Channel-Werte senden
    Serial1.write(DSM2_Data.Header, sizeof(DSM2_Data));



}


