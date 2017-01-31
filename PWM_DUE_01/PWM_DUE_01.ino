#define TC_CAPTURE_TIMER_SELECTION TC_CMR_TCCLKS_TIMER_CLOCK3
/** Use TC Peripheral 2 **/
#define TC  TC2

/** Configure TC2 channel 0 as capture input. **/
#define TC_CHANNEL_CAPTURE 0
#define ID_TC_CAPTURE      ID_TC6  //TC6 is channel 0 of TC2 module!!

// #define PIN_TC_CAPTURE_MUX IOPORT_MODE_MUX_B // peripheral B // TODO warum ist das Anschalten des pins nicht erforderlich??

/** Use TC2_Handler for TC capture interrupt**/
#define TC_Handler  TC6_Handler
#define TC_IRQn     TC6_IRQn


void TC_Handler(void)
{

}


void ppm_capture_initialize(void)
{

  pmc_enable_periph_clk(ID_TC_CAPTURE);
  
  
  NVIC_DisableIRQ(TC_IRQn);
  NVIC_ClearPendingIRQ(TC_IRQn);
  NVIC_SetPriority(TC_IRQn, 2);
  NVIC_EnableIRQ(TC_IRQn);
  
 
  
  
}


void setup() {                
    ppm_capture_initialize();
}

void loop() {
//  Debug_Pin C.24= 1 -> C.24 = 0 -> DUE_Pin6
  REG_PIOC_SODR = REG_PIOC_SODR   | 0x01000000u;    // Ausgang C24 =1 (DUE_Pin6)
   // delay_us(1);
  REG_PIOC_CODR = REG_PIOC_CODR   | 0x01000000u;    // Ausgang C24 =0 (DUE_Pin6)

}
