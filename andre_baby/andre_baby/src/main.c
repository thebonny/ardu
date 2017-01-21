

/*	xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx     HAPSTIK FOC-Software für Arduino DUE      xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

DATUM:		23.12.2016 19:11 Uhr EIN KOMPLETTER STICK IST LAUFFÄHIG

AUTOR:		André Frank
NAME:		HS_POT_HS_ENC_Full_Power_zügig_kein_so_starkes_Überschwingen_*
Ziel:		2 Achsen mit einem DUE.
			Szenario:
			- ein DUE mit 3V AVREF (MAX6070AAUT30) für ADC Referenz-Eingang und Anschluss Poti
			- 3,3V AVDD für Versorgung des Analogteils vom SAM3X8E
			- Analogkanal A0 -> Poti1
			- Analogkanal A1 -> Poti2
			- Analogkanal A2 -> Poti Sollwertvorgabe


------------------------------------------------------------------------------------------------------------------------------
			
				
				
				
				
				
				
				
				
				
				
				
				
				
				
				
------------------------------------------------------------------------------------------------------------------------------

						
STAND:
---			14.10.2015
			+ Funktion zur Ausgabe von 3 PWM-Signalen
				- auf 1 gewichtet
				- mit einem flankierenden Referenz-PWM-Ausgang, damit center-aligned
				  beobachtet werden kann
---			09.11.2015	  
			+ Vorführung im Lab
				- Drehmoment fühlen
				- volle Umdrehung mit korrigierter SVPWM von Microchip

---			15.11.2015	  
			+ Einbau der Dead-Times für __-- und --__ Übergänge an den komplementären PWM-Ausgängen
				- PWM Channel Mode PWM_CMRx -> Bit DTE=1 setzen
				- 500ns		
			+ Loop mit Winkelveränderung und delay, so dass sich am Motor eine Rotation einstellt
				- Beispiel:
				- z.B. Winkelfortschritt 0,1° elektrisch
				- 7° elektrisch -> 1° mechanisch
				- 70    Loop-Durchläufe für 1° mechanisch
				- 25200 Loop-Durchläufe für 360° mechanisch (1 Umdrehung)
				- Annahme: 90s für 1 Umdrehung
					-> 4°  mechanisch in 1 Sekunde
					-> 28° elektrisch in 1 Sekunde
					-> 280 Loop-Durchläufe in 1 Sekunde (Winkelfortschritt 0,1° elektrisch)
					-> 1000ms/280 = 3,57ms/Loop-Durchlauf
					-> delay etwa auf 3ms einstellen!
			+ SVPWM-Wertegenerierung aus u,v,w (siehe Simulink-Modell)
			+ aus Winkelwert die Werte u,v,w bilden und dann in die SVPWM1 übergeben
			+ 10 ° Zeiger, der sich etwa im Sekundentakt 10° weiter bewegt	

---			16.11.2015								  
			+ 5 GPIO-Pins: C21, C22, C23, C24, C25 als Output definieren und 2 Befehle für Ein / Aus definieren
				- dient dem zeitlichen Überwachen von Programmabschnitten -> ext. Oszi an Pins
			+ Ausführung SVPWM1 mit float Variablen: 20,89us

---			17.11.2015, 18.11.2015
			+ ENC-Anregungen (PHA, PHB)
			
---			23.11.2015
			+ Sound	("Alle meine Endchen ...")			
				- besser die Halbperioden über einen Timer erzeugen
				- Töne lassen sich berechnen 440*2hoch((n-49)/?)

---			26.11.2015			
			+ ADC funktioniert (1 Kanal), sehr genau, bin zufrieden
				- float to character (Funktion aus dem Internet, damit wir float-Werte ins Terminal ausgeben können)

---			27.11.2015
			+ ADC funktio				
			+ 1. LP-Entwurf -> corel
			+ Gimbal Controller angucken -> Welcher 3-Phasen-Inverter wird verwendet?
				- BGC 32Bit 3-Achsen Gimbal Brushless Controller Board
				- http://www.amazon.de/Hubschrauber-3-Achsen-Gimbal-Brushless-Controller/dp/B016MJ5UFW/ref=sr_1_fkmr0_1?s=toys&ie=UTF8&qid=1448611487&sr=1-1-fkmr0&keywords=BGC+V3.3+DRV8313
			+ ADC-Werte für CH5, CH6, CH7 ausgeben
			+ auf Wandlungsende (End of Conversion) testen

---			08.12.2015
			+ neue #defines für ADC Channel Data Register
			+ nach n-maligem Impuls auf der Event_Line wird ADC angestoßen				 

---			07.01.2016
			+ Init-Funktionen erstellen -> Übersichtlichkeit verbessern

---			11.01.2016
			+ Druck-Funktionen erstellen -> Übersichtlichkeit verbessern
				- PRINT_STROM();
				- PRINT_CLARKE_PARK();
				- PRINT_INV_CLARKE_PARK();
				- PRINT_ENCODER();
				- PRINT_SVPWM();
				- PRINT_REGLER();		 

---			08.01.2016
			- Noch einmal elementare Funktionen testen
				- Encoder Zählwerte
				- Excel Clarke_Park und Inv_Clarke_Park
				- ...

---			18.04.2016
			- "Warming Up"
			- es sieht so aus, als kämen wir:
				- ohne Clarke_Parke aus, da die Drehgeschwindigkeit (Drehzahl) für unsere 
				  Anwendung unbedeutend ist
				- ohne Strommessung aus (die nur für Clarke_Park benötigt wird)
			- damit müsste der Regler wie folgt aussehen:
				- Positionsvorgabe des Stick mit:
					- TC0_CV0-Wert oder Winkel_Rotor oder Winkel_Motor -> ist alles das Gleiche, da miteinander verknüpft
				- positive Abweichung davon führt zu Minus Qpi (Drehmoment nach links)
				- negative Abweichung führt zu Plus Qpi (Drehmoment nach rechts)
				- EOL
				
---			25.04.2016
			- Positionsregler und Geschwindigkeitsregler implementiert

			Mögliche Video-Präsentationen [Nr. / Dauer]:
			1. [1 / 10s] Kalibrierung Stick -> Verstellwege messen und Stop in Mittenposition (MP)
			2. Stick in MP -> Pilot lenkt aus (45° links, 45° rechts, ...) und lässt los:
				- [2 / 10s] Retrofunke	-> Stick fährt zurück in MP und schwingt um den MP
				- [3 / 10s] HapStik		-> Stick fährt gedämpft zurück in MP
				- [4 / 10s]				-> Stick fährt stark gedämpft zurück in MP
			3. Lehrer-Pilot bewegt Stick um den MP pendelnd hin und her (Lehrer-Pilot ist "unsichtbar")
				- [5 / 10s] Schüler-Pilot legt kraftlos Finger auf:
										-> Stick bewegt Finger  								 
				- [6 / 10s] Schüler-Pilot legt kraftvoll Finger auf:
										-> Finger bewegt trotzdem Stick (Überblendung möglich, volle Kontrolle)
													
---			04.10.2016				
			- Referenzspannungsquelle 2,5V auf DUE am REF_ADC-Eingang des Prozessoers angeschlossen
			- ADC Kanal 5,6 -> Verstärkung im ADC_CGR (Channel Gain Register) auf x4 gestellt
			- Arbeitsbereich der Potis (+-35°) auf den HS-Achsen auf linken Rand des Poti-Bereiches mechanisch verstellt
				- Potis gedreht -> nun ist eine x4 Multiplikation möglich und damit eine ADC-Auflösung von 14Bit
			    - von den 333° des Potis werden vom HS nur ca. +- 35°, also 70° genutzt
				- 333° -> 4096 (12Bit)
				- 70°  ->  861 (gemessen ca. 750)
				- x4   ->  861 * 4 = 3444
				- 3444 / 70 = 49,2 Steps je Grad ... oder 0,0203° Auflösung
				- im Vergleich die Enc-Lösung mit 2000-er ENC:
					- 360° -> 8000
					- 70°  -> 1555
					- 1555 / 70 = 22,2  Steps je Grad ... oder 0,045° Auflösung
				- Damit ist die Encoder-Variante (2000-er) in der Auflösung nur "halb" so gut, 
				  wie die Variante Poti + ADC (x4)
				  
---			05.10.2016

			- Spannungen am HapStik Nr.1:
							|
						  0,082V
							|
							|
			 -- 0,068V -----|----- 0,633V --
							|
							|
						  0,648V
							|
			- Potis müssten noch etwas präziser eingebaut werden, sprich der Arbeitsbereich noch weiter nach links
			  verlagert werden
			- Warum? -> weil 0,648V x 4 (PGA im ADC) = 2,592V sind, aber nur 2,5V gewandelt werden können!
					 -> weil 0,633V x 4 (PGA im ADC) = 2,532V sind, aber nur 2,5V gewandelt werden können!				  

			- Testergebnisse: zunächst unbefriedigend!
			- Warum?
				- die ADC-Werte sind nicht ruhig: z.B. 2708 - 2718
				- obgleich die ADREF steht: 2,49547V ... 2,49548V           -> delta nur 0,00001V oder 0,01mV ... -> ist ok!
				- und obgleich am Poti      0,41774V ... 0,41776V anliegen  -> delta nur 0,00002V oder 0,02mV
				- und das bei einer Quantisierung von 2,5V / 4096 = 0,61mV!!!
				- und damit der Messwert offensichtlich nur ca. 1/30 der Quantisierung "wackelt"!
				-> Was ist dafür die Ursache? Der ADC selbst?
			- Abhilfemöglichkeiten:
				- Datenblatt studieren / prüfen
				- der ADC arbeitet mit 100us, liefert also innerhalb einer ms -> 10 Werte
					- ich könnte einen Mittelwert aus diesen 10 Werten bilden und den dann nach einer ms ausgeben
					-> am besten wäre eine Normalverteilung der ADC-Werte -> prüfen?
				- Pufferkondensatoren an die Analogeingänge, ca. 10nF nach GNDANA (Pin74), tau = 5kOhm * 10nF = 50us, sollte passen!		

---			06.10.2016
			Weiteres Vorgehen:
			Die ADC-Werte müssen ruhig stehen.
			- kleine Platine mit 3 kurzen Leitungen:
				- VREF, A1-Input, AGND
				- dort zunächst Spannungsteiler aus Festwiderständen: 2kOhm + 8kOhm -> 0,5V x 4 = 2,0V im ADC, nach PGA
				- abschliessend Poti, wenn Werte stehen

			- ADC-Parameter anpassen		
				- ADC clock
				  ADC clock range is between MCK/2, if PRESCAL is 0, and MCK/512, if PRESCAL is set to 255 (0xFF).
					- MCK ist 84MHz
					- aus Datenblatt:	- ZSOURCE = 22kOhm ADC clock 10,67MHz
										- ZSOURCE = 14kOhm ADC clock 16,00MHz
					- Annahme: ZSOURCE = 15kOhm (Poti + 5kOhm, zur Sicherheit)
					- wir nehmen 84MHz / 6 = 14MHz
					- ADCClock = MCK / ( (PRESCAL+1) * 2 ) = 84MHz / (2 + 1) * 2 = 84MHz / 6 = 14MHZ  -> period 71,43ns
					-> PRESCAL = 2 = 02H	
				- TRACKTIME: 
					- Berechnung: 12-bit mode: tTRACK = 0.054 × ZSOURCE + 205 = 0.054 x 15000 + 205 = 1015ns
					- Einstellwert: (TRACKTIM + 1) x ADC clock period = (14 + 1) x 71,43ns = 1071ns -> passt
					-> TRACKTIM = 14 = EH 
				- TRANSFER: = 1H (dazu ist nix im Datenblatt zu finden, also wofür, ..., warum, ...)
					- TRANSFER period = (TRANSFER * 2 + 3) * ADCClock period = (1 * 2 + 3) * 71,43ns = 357,15ns	
				- SETTLING: Settling time to change offset and gain laut Datenblatt (S.1408) min. 200ns
					- wir nehmen den Wert 1 (S.1334) -> entspricht 5 ADC clock periods = 5 x 71,43ns = 357,15ns
					-> SETTLING = 1 = 1H
				- ANACH = 1, damit SETTLING wirkt	 
				- STARTUP = 0, weil ADC immer aktiv ist
				- Zusammenfassung der Parameter:
					- 15kOhm	Quellwiderstand 
					- 14MHz		ADC clock
					- 71,43ns	ADCClock period
					- 1,071us	Tracktime 
					- 357,15ns	TRANSFER period
					- 357,15ns	SETTLING period (Einschwingen beim Übergang zum nächsten ADC-Kanal, z.B. andere Verstärkung, etc.
																
			->	- Referenz verbessern (höhere Spannung, weniger ppm -> Initialisierungswerte des Sticks bleiben über der Temperatur
			      erhalten, Genauigkeit höher)
				- ADCREF: min. 2,4V, max. 3,3V (VDD) 
					-> MAX6063A 3V Referenz, aber 20ppm, initiale Genauigkeit 0,4% bei 25°C, SOT23
					-> MAX6070AAUT30, 3V Referenz, 6ppm, initiale Genauigkeit 0,04% bei 25°C, SOT23/6				
					-> MAX6070AAUT33, 3,3V Referenz, 6ppm, initiale Genauigkeit 0,04% bei 25°C, SOT23/6	


	
---			05.12.2016 (2 Monate später)
			- Schaltung aufgebaut
				- 3V AVREF (MAX6070AAUT30) für ADC Referenz-Eingang und Anschluss Poti 
				- 3,3V AVDD für Versorgung des Analogteils vom SAM3X8E
				
			- Werte schwanken jetzt "nur noch" um 4, vorher über 10
			- das entspricht der ENOB (Effektive Anzahl von Bits) des ADCs (S.1407 Datenblatt) von 10,3 Bits
			- die ADC-Werte sind normalverteilt oder entsprechen der gaußschen Glockenkurve, so dass mit Oversampling und 
			  Mittelwertbildung die Genauigkeit erhöht werden kann.
			
			->	- mit 20kHz abtasten -> 50us
				- Mittelwertfilter bauen  

---			10.12.2016
			- alle 10ms einen gefilterten Wert
			- dann Reglerexperimente


---			13.12.2016
			- HS-Leistungsteil in Betrieb genommen -> eine Achse funktioniert jetzt einwandfrei
			
---			14.12.2016
			- PWM-Controller neu initialisiert, für 2 Motore
			- SVPWM für beide Motore erweitert
			-> funzt, Signale verlassen DUE korrekt

---			15.12.2016
			- Regler dubliziert
			- HAPSTIK FERTIG -> keine fundamentalen Fragen offen
			
---			16.12.2016 (Fr)
			- Thomas bei mir, war beeindruckt vom HSA
			- HSA macht Kreisbahn

---			17.12.2016 (Sbd)
			- SVPWM läuft mit 20kHz -> "PIEPSEN" ist weg!
			
---			18.12.2016 (So, 4.Advent)
			- HSA macht g-Spirale
			- HSA macht RECORDER (10s, 1ms Abtastung)
			- ADC läuft mit 20kHz, Oversampling -> jetzt werden 20 Werte im Average-Filter gemittelt




// ---------------------------------------------------------------------------------------------------------------------------			
FESTLEGUNGEN:
			- so werden gemessene Zeiten dokumentiert: 
				// TIME: 18,7us



FRAGEN:
---			Wie ist eine formatierte, spaltenorientierte Ausgabe in mit printf möglich?
			->	Mit der Angabe der Feldbreite z.B. %-20s bedeutet: 20 Zeichen breit, linksbündig

---			





ERLEDIGEN:	- Microchip Strommessroutine ansehen, ob dort eine Normierung stattfindet
			- Anzahl Aufrufe ADC-Interrupt: 
				- Anzeige -> ADC_ISR_CNT
				- evtl. einstellen  


			02.01.2016
			- nur d, q vorgegeben 
				-> astreines Drehmoment
			- jedoch dabei den Output von Clarke_Parke anzeigen lassen und dort ist ein wandern der d,q-Werte in Abhängigkeit
			  des ENC_WKL festzustellen
				-> ausgeben: ENC_WKL, ENC_WKL_MOT		 		
				-> dabei festgestellt, dass ENC_WKL_MOT Werte über 360° anzeigt -> FEHLER -> behoben	

			- d,q am Ausgang der ClarkePark müssen konstant sein?! Oder? Natürlich, ... vermute ich!





HILFE:		- Äquivalente:
				- PWM->PWM_CH_NUM[0].PWM_CMR = 0x1u << 8;
				- PWM->PWM_CH_NUM[0].PWM_CMR = PWM_CMR_CALG;

			- Bittest auf 80H:
				if (REG_ADC_ISR & (1 << 7))
				{
					...
				}
*/






/*	DEBUG_PINS
	Debug_Pin C.25= 1 -> C.25 = 0
				REG_PIOC_SODR	= REG_PIOC_SODR		|	0x02000000u; 		// Ausgang C25 =1 (DUE_Pin 5)	
				delay_us(10);
				REG_PIOC_CODR	= REG_PIOC_CODR		|	0x02000000u; 		// Ausgang C25 =0 (DUE_Pin 5)	

	Debug_Pin C.24= 1 -> C.24 = 0
				REG_PIOC_SODR	= REG_PIOC_SODR		|	0x01000000u; 		// Ausgang C24 =1 (DUE_Pin 6)
				delay_us(10);
				REG_PIOC_CODR	= REG_PIOC_CODR		|	0x01000000u; 		// Ausgang C24 =0 (DUE_Pin 6)
*/





//	ANFANG **********************************************     Includes       *************************************************
	#include "asf.h"
	#include "conf_board.h"
	#include "delay.h"					// z.B.	delay_us(564)	-> warte 564us
										//		delay_ms(20)	-> warte 20ms
										//		delay_s(2)		-> warte 2s
	#include "stdio_serial.h"			// UART		z.B. printf("MEC22_STEPS:   %i\r\n", ENC1_C);
	#include "conf_clock.h"
	#include "math.h"					//			z.B. cos(x)
	#include "string.h"
//	ENDE ************************************************     Includes       *************************************************





//	ANFANG **********************************************     defines        *************************************************
//	ADC, diese defines fehlen leider in der CMSIS
	#define REG_ADC_CDR             (*(__I  uint32_t*)0x400C0050U) // ADC Channel Data Register
	#define REG_ADC_CDR1			(*(__I  uint32_t*)0x400C0054U) // ADC Channel Data Register
	#define REG_ADC_CDR2			(*(__I  uint32_t*)0x400C0058U) // ADC Channel Data Register
	#define REG_ADC_CDR3			(*(__I  uint32_t*)0x400C005CU) // ADC Channel Data Register
	#define REG_ADC_CDR4			(*(__I  uint32_t*)0x400C0060U) // ADC Channel Data Register
	#define REG_ADC_CDR5			(*(__I  uint32_t*)0x400C0064U) // ADC Channel Data Register
	#define REG_ADC_CDR6			(*(__I  uint32_t*)0x400C0068U) // ADC Channel Data Register
	#define REG_ADC_CDR7			(*(__I  uint32_t*)0x400C006CU) // ADC Channel Data Register
	#define REG_ADC_CDR8			(*(__I  uint32_t*)0x400C0070U) // ADC Channel Data Register
	#define REG_ADC_CDR9			(*(__I  uint32_t*)0x400C0074U) // ADC Channel Data Register
	#define REG_ADC_CDR10			(*(__I  uint32_t*)0x400C0078U) // ADC Channel Data Register
	#define REG_ADC_CDR11			(*(__I  uint32_t*)0x400C007CU) // ADC Channel Data Register
	#define REG_ADC_CDR12			(*(__I  uint32_t*)0x400C0080U) // ADC Channel Data Register
	#define REG_ADC_CDR13			(*(__I  uint32_t*)0x400C0084U) // ADC Channel Data Register
	#define REG_ADC_CDR14			(*(__I  uint32_t*)0x400C0088U) // ADC Channel Data Register
	#define REG_ADC_CDR15			(*(__I  uint32_t*)0x400C008CU) // ADC Channel Data Register

	#define	PI		3.141592654f
	#define	WK1		(PI/180)
	#define	WK2		(PI*2/3)
	#define	WK3		(PI*4/3)
//	ENDE ************************************************     defines        *************************************************





//	ANFANG ********************************************     Global Variables       *******************************************



//	für RECORDER
	static		int		iREC1[10000];		// moveDaten MOTOR1
	static		int		iREC2[10000];		// moveDaten MOTOR2
		
	static		int		cnt_scan = 0;
	
	static		int		iPT = 1;			// ProgrammTeil


//	für TÄNZCHEN
	static		float	IST1 = 0.0;			// MOTOR1
	static		float	IST2 = 0.0;			// MOTOR2	
	
	static		float	TW_1 = 0.0;			// MOTOR1 -> TänzchenWinkel		
	static		float	TW_2 = 0.0;			// MOTOR2 -> TänzchenWinkel	

	static		int		VZ_1 = 1;			// VorZeichen
	static		int		VZ_2 = 1;

	static		float	SP_1 = 0.36;		// MOTOR1 -> Speed
	static		float	SP_2 = 0.0;	

	static		float	AM_1 = 0.0;			// MOTOR1 -> Speed
	static		float	AM_2 = 0.0;
					
	static		float	SA_1 = 0.0;			// MOTOR1 -> SoftAnlauf
	static		float	SA_2 = 0.0;			// MOTOR2 -> SoftAnlauf

	static		float	DW_1 = 0.0;			// MOTOR1 -> DeltaWinkel
	static		float	DW_2 = 0.0;			// MOTOR2 -> DeltaWinkel
	
	static		int		TAE_1ms = 0;		// 1ms zählen	
		
		
//	für Schleifen -> Zählvariablen
	volatile	int		a, b, c, d;


//	Zeitverzögerungen (delay)
	volatile	int		TD1;
	volatile	int		TD2;
	volatile	int		TD3;
	volatile	int		TD4;
	volatile	int		TD5;


//	für Anzeige der Verteilung von Werten

	volatile	int		PRINT_VERT_cnt_i = 0;			// Zählvariable für PRINT_WERT Ausgaben

	volatile	int		VW_i   =  0;

	volatile	int		VW_MW_i;						//  Mittelwert, der von Hand eingetragen werden muss
														//  und etwa die Mitte von VW_MIN bis VW_MAX betragen sollte
														
														
	volatile	int		VW_1 =  0;						//	Initialisierung der Vergleichswerte:
	volatile	int		VW_2 =  0;
	volatile	int		VW_3 =  0;
	volatile	int		VW_4 =  0;
	volatile	int		VW_5 =  0;
	volatile	int		VW_6 =  0;
	volatile	int		VW_7 =  0;
	volatile	int		VW_8 =  0;
	volatile	int		VW_9 =  0;

	
		
								
	volatile	int		VW_MIN_i =  1000000;			// Startwerte
	volatile	int 	VW_MAX_i = -1000000;	

	volatile	int		VW_cnt_1 =  0;					//	Zähler
	volatile	int		VW_cnt_2 =  0;
	volatile	int		VW_cnt_3 =  0;
	volatile	int		VW_cnt_4 =  0;
	volatile	int		VW_cnt_5 =  0;
	volatile	int		VW_cnt_6 =  0;
	volatile	int		VW_cnt_7 =  0;
	volatile	int		VW_cnt_8 =  0;
	volatile	int		VW_cnt_9 =  0;
	volatile	int		VW_cnt_10 = 0;	

	volatile	float	VW_SUM;							// Summe der VW_cnt_* - Werte
	
	
		
	volatile	float	P_ADC_A0_f;						// Snapshot von den ADC-Werten für Print
	volatile	float	P_ADC_A1_f;
	volatile	float	P_ADC_A2_f;

// Mittelwertfilter


/* nach oben
Die Speicherklasse static legt innerhalb einer Funktion fest, daß dauerhaft für
die Variable Speicherplatz reserviert wird. Normalerweise ist der Wert einer Variablen
innerhalb einer Funktion bei jeden Aufruf anfänglich undefiniert. Bei einer
statischen Variable ist das nicht der Fall.
*/

	static		int		af_count_i	= 0;						// Laufvariable im AF-Array
	static		int		SUM_AF_i_1	= 0;						// Summe MOTOR1
	static		int		SUM_AF_i_2	= 0;						// Summe MOTOR2
		
	static		int		AF_A0_i		= 0;						// Average_Filterwert für ADC-Kanal A0
	static		float	AF_A0_f		= 0;
	static		int		AF_A1_i		= 0;						// Average_Filterwert für ADC-Kanal A0
	static		float	AF_A1_f		= 0;
	
	
//	für SV_PWM()	
	volatile	float	PWMu1, PWMv1, PWMw1;
	volatile	float	PWMu2, PWMv2, PWMw2;
	
	
		
//	für Funktion: MOTOR_DREHT()
	volatile	int		MOTOR_DREHT_CNT = 0;							// Anzahl Aufrufe
	volatile	int		M_STEP_Z	=0;
	volatile	float	DWE1		= 0.0;
	volatile	float	DWE_a1		= 0.0;									// alter DWE-Wert

	volatile	float	DWE2		= 0.0;
	volatile	float	DWE_a2		= 0.0;									// alter DWE-Wert
	
		
//	für ADC_ISR()
//	Anzahl Aufrufe	
	volatile	int		ADC_ISR_CNT = 0;								// Anzahl Aufrufe des ADC-Interrupts
	volatile	int		ADC_ISR_CNT_1ms = 0;							// 1ms zählen
	
	volatile	float	U_A0 = 2.5, U_A1 = 2.5, U_A2 = 2.5;
	volatile	float	MAX_U_A0 = 0, MIN_U_A0 = 5.0, MIT_U_A0 = 0;
	volatile	float	MAX_U_A1 = 0, MIN_U_A1 = 5.0, MIT_U_A1 = 0;
	volatile	float	MAX_U_A2 = 0, MIN_U_A2 = 5.0, MIT_U_A2 = 0;
	volatile	float	U_GES;
	volatile	float	BAND_U_A0 = 0, BAND_U_A1 = 0, BAND_U_A2 = 0;

	volatile	int		svpwm_int = 0;									// Flag wird hier gesetzt und in der Endlosschleife
																		// der MAIN() zurück gesetzt
																		// -> für synchrone Vorgänge zu ADC_INT

//	für Poti am Analogeingng A0


	volatile	float	CH1_WERT1_1			= 0.0;						// Empfänger-Wert
	volatile	float	CH1_WERT1_1_alt		= 0.0;
	volatile	float	CH1_WERT1_1_li		= 0.0;						// linear interpoliert	
	volatile	float	CH1_WERT1_1_li_nor	= 0.0;						// linear interpoliert und normiert	
	volatile	float	CH1_DELTA			= 0.0;						// Delta-Wert für 1ms
		
	volatile	float	CH1_WERT2_1 = 0.0;		
	volatile	int		CH1_WERT3_1 = 0;


	volatile	float	CH2_WERT1_1			= 0.0;						// Empfänger-Wert
	volatile	float	CH2_WERT1_1_alt		= 0.0;
	volatile	float	CH2_WERT1_1_li		= 0.0;						// linear interpoliert	
	volatile	float	CH2_WERT1_1_li_nor	= 0.0;						// linear interpoliert und normiert	
	volatile	float	CH2_DELTA			= 0.0;						// Delta-Wert für 1ms
		
	volatile	float	CH2_WERT2_1 = 0.0;		
	volatile	int		CH2_WERT3_1 = 0;
	
	
	
	
	static		int		cnt_1ms_poll = 0;								// 1ms Counter, der in der Polling Main inkrementiert wird 



	volatile	float	ADC_POTI = 0.0f;							//

//	MOTOR1
	volatile	float	POT_CNT1_1		= 0, POT_CNT2_1 = 0;
//	MOTOR2
	volatile	float	POT_CNT1_2		= 0, POT_CNT2_2 = 0;	
	
	volatile	float	POT_CNT_3		= 0;
	volatile	float	POT_V_1			= 0.0;							// Geschwindigkeit MOTOR1
	volatile	float	POT_V_2			= 0.0;							// Geschwindigkeit MOTOR2


//	für Potis an den Analogeingängen A0, A1, A2
	volatile	int		ADC_A0_i = 0;									// Integer, Poti vom Stick für Sollpositionsvorgabe
	volatile	int		ADC_A1_i = 0;									// Integer, Poti HapStik vertikal
	volatile	int		ADC_A2_i = 0;									// Integer, Poti HapStik horizontal	
	
// SnapShot der ADC-Werte in der Print-Ausgabe erstellt	
	volatile	int		SS_ADC_A0_i;
	volatile	int		SS_ADC_A1_i;	
	volatile	int		SS_ADC_A2_i;	
	
	
	
	
	volatile	float	ADC_A0_f = 0.0f;								// Float, Poti vom Stick für Sollpositionsvorgabe
	volatile	float	ADC_A1_f = 0.0f;								// Float, Poti HapStik vertikal
	volatile	float	ADC_A2_f = 0.0f;								// Float, Poti HapStik horizontal

	volatile	float	POT_U_ver_NULL1 = 0.0f;							// ADC-Spannungswert in der vertikalen Sticknullposition 
	volatile	float	POT_U_ver_MAX1 = 0.0f;							// ADC-Spannungswert in der vertikalen Stickobenposition 
	volatile	float	POT_U_ver_MIN1 = 0.0f;							// ADC-Spannungswert in der vertikalen Stickuntenposition	

	volatile	float	POT_U_ver_NULL2 = 0.0f;							// ADC-Spannungswert in der vertikalen Sticknullposition
	volatile	float	POT_U_ver_MAX2 = 0.0f;							// ADC-Spannungswert in der vertikalen Stickobenposition
	volatile	float	POT_U_ver_MIN2 = 0.0f;							// ADC-Spannungswert in der vertikalen Stickuntenposition


	volatile	float	POT_PF_ver = 0.0f;								// Proportionalitätsfaktor zur akzeullen Winkelberechnung
	volatile	float	POT_ver_WKL = 0.0f;								// Winkel Stick vertikal

	volatile	float	POT_ver_WKL_n = 0.0f;							// neuer Winkel Rotor vertikal
	volatile	float	POT_ver_WKL_a = 0.0f;							// alter Winkel Rotor vertikal

	volatile	float	POT_ver_V = 0.0f;								// Drehgeschwindigkeit U/s vertikales StickPoti
	volatile	float	POT_hor_V = 0.0f;								// Drehgeschwindigkeit U/s horizontales StickPoti
	
	
	volatile	float	POT_U_hor_NULL1 = 0.0f;							// ADC-Spannungswert in der horizontalen Sticknullposition 
	volatile	float	POT_U_hor_MAX1 = 0.0f;							// ADC-Spannungswert in der horizontalen Stickobenposition 
	volatile	float	POT_U_hor_MIN1 = 0.0f;							// ADC-Spannungswert in der horizontalen Stickuntenposition	

	volatile	float	POT_U_hor_NULL2 = 0.0f;							// ADC-Spannungswert in der horizontalen Sticknullposition
	volatile	float	POT_U_hor_MAX2 = 0.0f;							// ADC-Spannungswert in der horizontalen Stickobenposition
	volatile	float	POT_U_hor_MIN2 = 0.0f;							// ADC-Spannungswert in der horizontalen Stickuntenposition

		
	volatile	float	WKL_OFF_1 =		5.5;							// Winkel_Offset für Poti_MOTOR1 Vertikal		(Nullposition)	-> "+" Stick wandert nach unten
	volatile	float	WKL_OFF_2 =		-16.0;							// Winkel_Offset für Poti_MOTOR2 Horizontal		(Nullposition)	-> "+" Stick wandert nach rechts

//	volatile	float	WKL_OFF_ver2 = -14.38;							// Winkel_Offset für POTI vertikal (Nullposition)
//	volatile	float	WKL_OFF_hor2 = 0.0;								// Winkel_Offset für POTI horizontal (Nullposition)
		
	
//	für STROM()
	volatile	int		STROM_CNT = 0;									// Anzahl Aufrufe	
	
	volatile	float	STROM_A0 = 0, STROM_A1 = 0, STROM_A2 = 0;
	volatile	float	S_A0 = 0, S_A1 = 0, S_A2 = 0;
		
	volatile	float	WINKEL_STROM = 0, WINKEL_STROM_ALT = 0;
	volatile	float	X_KOMP = 0, Y_KOMP = 0;
	volatile	float	MV_LAENGE = 0, MV_LAENGE_REZ = 0;
	volatile	float	ADC_PROP = 2.65;								// ADC-Proportionalitätsfaktor, so dass qpi-Werte zu
																		// gleichen bzw. ähnlichen qcp-Werten führen

//	für QDEC, Encoder
	volatile	int		ENCODER_CNT		= 0;							// Anzahl Aufrufe
	
	volatile	int		ENC_CNT			= 0, ENC_CNT_ALT = 0;
	volatile	int		ENC_CNT_MOT		= 0;
	
	volatile	int		ENC_UMD			= 0;
	volatile	int		ENC_UMD_MOT		= 0;
	
	volatile	float	ENC_CNT_1		= 0, ENC_CNT_2 = 0, ENC_CNT_3 = 0;
	volatile	float	ENC_V			= 0.0;	
			
	volatile	float	ENC_WKL			= 0;
	volatile	float	ENC_WKL_MOT		= 0;
//	ACHTUNG DEBUG
	volatile	int		ENC_WKL_MOT_INT		= 0;
		
	volatile	float	ENC_WKL_MOT60	= 0;

//	für CLARKE_PARK()
	volatile	float	ENC_WKL_RAD		= 0;
	volatile	float	ENC_WKL_MOT_RAD = 0;
	volatile	float	ENC_WKL_MOT60_RAD = 0;

	volatile	float	alpha = 0;
	volatile	float	beta =0;
	volatile	float	Dcp = 0;
	volatile	float	Qcp =0;
	
//	Array von Zeichen für Funktion: float_to_string(...)
	char	Ergebnis[20];

//	Leistungsfaktor	0 ... 1 Leistung | 0.5 -> 50% Leistung | 1.0 -> 100% Leistung
	volatile	float	LF1	= 1.0;
	volatile	float	LF2	= 1.0;
	
	
//	Duty cycle buffer for PDC transfer
//	uint16_t g_us_duty_buffer[3];

//	PDC transfer packet
//	pdc_packet_t g_pdc_tx_packet;

//	für SVPWM()
	float	X1, Y1, Z1;
	float	X2, Y2, Z2;
		
//	für PI-REGLER()	
	volatile	float	ST = 0.0005;									// 1ms -> SampleTime ADC

//	PI-Regler für d
	volatile	float	Df = 0.0;										// Führungsgröße d

//	ACHTUNG
	volatile	float	Kdp = 0.2;										// Proportionalitätsbeiwert PI-Regler für d
	volatile	float	Kdi = 5;										// Integrierbeiwert PI-Regler für d

	volatile	float	De = 0;											// Regeldifferenz für d		
	volatile	float	Dpy = 0;										// Anteil P-Regler für d
	volatile	float	Diy = 0;										// Anteil I-Regler für d
	
	volatile	float	Dpi = 0;										// Anteil P+I-Regler für d, hier 1 -> maximales
																		// direktes Moment
	volatile	float	Dpitemp = 0;									// Anteil P+I-Regler für d			

	volatile	float	Diyv = 0;										// vorhergehender Wert von Diy			


//	PI-Regler für q	

//	ACHTUNG
	volatile	float	Qf = 0.5;										// Führungsgröße q	

	volatile	float	ENC_WKLf = 90;

//	ACHTUNG
	volatile	float	Kqp = 0.0001;									// Proportionalitätsbeiwert PI-Regler für q
	volatile	float	Kqi = 0.02;										// Integrierbeiwert PI-Regler für q
	
	volatile	float	Qe = 0;											// Regeldifferenz für q	
	volatile	float	Qpy = 0;										// Anteil P-Regler für q
	volatile	float	Qiy = 0;										// Anteil I-Regler für q
	volatile	float	Qpi = 0;										// Anteil P+I-Regler für q
	volatile	float	Qpitemp = 0;									// Anteil P+I-Regler für q	
	volatile	float	Qiyv = 0;										// Vorhergehender Wert von Qiy	


//	für print float
	#define STRING_EOL    "\r"
	#define STRING_HEADER "Test double to ascii conversion"





//	static double PRECISION = 0.00000000000001;
	static double PRECISION = 0.001;					//Anzahl Nachkommastellen, -> hier ohne Rundung!
	static int MAX_NUMBER_STRING_SIZE = 32;

	char s1[32];
	char s2[32];
	char s3[32];
	char s4[32];
	char s5[32];
		
//	* Double to ASCII prototype. Must be declared before usage in main function !!
	char * doubleToString(char *s, double n);
	

//	1.Regler MOTOR1	
//	PID1_1 Regler GESCHWINDIGKEIT
	float myInput1_1;
	
	float error1_1 = 0.0, input1_1 = 0.0, dInput1_1 = 0.0, lastInput1_1 = 0.0, mySetpoint1_1 = 0.0;
			
	double myOutput1_1;
	double ITerm1_1;
	double ki1_1 = 0.0;
	double kp1_1 = 0.0;
	double kd1_1 = 0.0;
	double outMax1_1;
	double outMin1_1;
	

//	2.Regler MOTOR1	
//	PID2_1 Regler POSITION
	int myInput2_1;
		
	float error2_1 = 0.0, input2_1 = 0.0, dInput2_1 = 0.0, lastInput2_1 = 0.0, mySetpoint2_1 = 0.0;

	double myOutput2_1;
	double ITerm2_1;
	double ki2_1 = 0.0;
	double kp2_1 = 0.0;
	double kd2_1 = 0.0;
	double outMax2_1;
	double outMin2_1;
		

//	1.Regler MOTOR2	
//	PID1_2 Regler GESCHWINDIGKEIT
	float myInput1_2;

	float error1_2 = 0.0, input1_2 = 0.0, dInput1_2 = 0.0, lastInput1_2 = 0.0, mySetpoint1_2 = 0.0;

	double myOutput1_2;
	double ITerm1_2;
	double ki1_2 = 0.0;
	double kp1_2 = 0.0;
	double kd1_2 = 0.0;
	double outMax1_2;
	double outMin1_2;

//	2.Regler MOTOR2	
//	PID2_2 Regler POSITION
	int myInput2_2;

	float error2_2 = 0.0, input2_2 = 0.0, dInput2_2 = 0.0, lastInput2_2 = 0.0, mySetpoint2_2 = 0.0;

	double myOutput2_2;
	double ITerm2_2;
	double ki2_2 = 0.0;
	double kp2_2 = 0.0;
	double kd2_2 = 0.0;
	double outMax2_2;
	double outMin2_2;

	
	
	

	volatile	int		SPRUNG_CNT		= 0;							// Anzahl Aufrufe
	volatile	int		FLAG_SWEEP 		= 0;							// Anzahl Aufrufe

//	* Ende PID Regler Global Variablen
	
	
//	ENDE **********************************************     Global Variables       *******************************************





//	ANFANG **********************************************     FUNKTIONEN     *************************************************


//	MOTOR1
//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    REGLER1_1   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void PID1_1(void)
{
	  input1_1 = myInput1_1;
      error1_1 = mySetpoint1_1 - input1_1;
      ITerm1_1+= (ki1_1 * error1_1);
      if(ITerm1_1 > outMax1_1) ITerm1_1= outMax1_1;
      else if(ITerm1_1 < outMin1_1) ITerm1_1= outMin1_1;
      dInput1_1 = (input1_1 - lastInput1_1);
 
// Reglerausgang berechnen
      double output1_1 = kp1_1 * error1_1 + ITerm1_1 - kd1_1 * dInput1_1;

// Überläufe kappen      
	  if(output1_1 > outMax1_1) output1_1 = outMax1_1;
      else if(output1_1 < outMin1_1) output1_1 = outMin1_1;
	  myOutput1_1 = output1_1;
	  
//	letzten Wert speichern
      lastInput1_1 = input1_1;
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    REGLER1_1   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx





//	MOTOR2
//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    REGLER1_2   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void PID1_2(void)
{
	input1_2 = myInput1_2;
	error1_2 = mySetpoint1_2 - input1_2;
	ITerm1_2+= (ki1_2 * error1_2);
	if(ITerm1_2 > outMax1_2) ITerm1_2= outMax1_2;
	else if(ITerm1_2 < outMin1_2) ITerm1_2= outMin1_2;
	dInput1_2 = (input1_2 - lastInput1_2);
	
	// Reglerausgang berechnen
	double output1_2 = kp1_2 * error1_2 + ITerm1_2 - kd1_2 * dInput1_2;

	// Überläufe kappen
	if(output1_2 > outMax1_2) output1_2 = outMax1_2;
	else if(output1_2 < outMin1_2) output1_2 = outMin1_2;
	myOutput1_2 = output1_2;
	
	//	letzten Wert speichern
	lastInput1_2 = input1_2;
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    REGLER1_2   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx





//	MOTOR1
//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    REGLER2_1   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void PID2_1(void)
{
	input2_1 = myInput2_1;
    error2_1 = mySetpoint2_1 - input2_1;
    ITerm2_1+= (ki2_1 * error2_1);
    if(ITerm2_1 > outMax2_1) ITerm2_1= outMax2_1;
    else if(ITerm2_1 < outMin2_1) ITerm2_1= outMin2_1;
    dInput2_1 = (input2_1 - lastInput2_1);
 
// Reglerausgang berechnen
    double output2_1 = kp2_1 * error2_1 + ITerm2_1 - kd2_1 * dInput2_1;

// Überläufe kappen      
	if(output2_1 > outMax2_1) output2_1 = outMax2_1;
    else if(output2_1 < outMin2_1) output2_1 = outMin2_1;
	myOutput2_1 = output2_1;
	  
//	letzten Wert speichern
	lastInput2_1 = input2_1;
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    REGLER2_1   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx






//	MOTOR2
//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    REGLER2_2   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void PID2_2(void)
{
	input2_2 = myInput2_2;
	error2_2 = mySetpoint2_2 - input2_2;
	ITerm2_2+= (ki2_2 * error2_2);
	if(ITerm2_2 > outMax2_2) ITerm2_2= outMax2_2;
	else if(ITerm2_2 < outMin2_2) ITerm2_2= outMin2_2;
	dInput2_2 = (input2_2 - lastInput2_2);
	
// Reglerausgang berechnen
	double output2_2 = kp2_2 * error2_2 + ITerm2_2 - kd2_2 * dInput2_2;

// Überläufe kappen
	if(output2_2 > outMax2_2) output2_2 = outMax2_2;
	else if(output2_2 < outMin2_2) output2_2 = outMin2_2;
	myOutput2_2 = output2_2;
	
//	letzten Wert speichern
	lastInput2_2 = input2_2;
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    REGLER2_2   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx







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





//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    Float -> String   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
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





//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx       UART        xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
/*	Einstellungen sind:
		- 115kBaud
		- 8N1
	Output:
		- z.B. Integer Variable (INT_VAR) plotten
		printf("Dies ist eine IntegerVariable:   %i\r\n", INT_VAR);
		
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
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx       UART        xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx





//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx       SVPWM     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//	SpaceVektorPulsWeitenModulation
//	Gemessene maximale Ausführungszeiten:
//	1.:		20,89us		mit "void SVPWM(float uum, float uvm, float uwm)"
//	2.:		23,69us		mit "void SVPWM(double uum, double uvm, double uwm)"
//	Delta:	 2,80us
//	TIME:	17us

//	Input:
//		- X, Y, Z				Spannungen aus Funktion INV_CLARKE_PARK
//	Output:
//		- PWMu1, PWMv1, PWMw1	"Center aligned PWM-Signale" die direkt auf die Halbbrücken ausgegeben werden
//		- PWMu2, PWMv2, PWMw2



void SVPWM(float uum1, float uvm1, float uwm1, float uum2, float uvm2, float uwm2)
{
	//	Motor_1

	if (uum1 >= 0)
	{
		if (uvm1 >= 0)
		{
			PWMw1 = (1-uvm1-uum1)/2;
			PWMv1 = PWMw1 + uvm1;
			PWMu1 = PWMv1 + uum1;
		}
		else
		{
			if (uwm1 >= 0)
			{
				PWMv1 = (1-uum1-uwm1)/2;
				PWMu1 = PWMv1 + uum1;
				PWMw1 = PWMu1 + uwm1;
			}
			else
			{
				PWMv1 = (1+uvm1+uwm1)/2;
				PWMw1 = PWMv1 - uvm1;
				PWMu1 = PWMw1 - uwm1;
			}
		}
	}
	else
	{
		if (uvm1 >= 0)
		{
			if (uwm1 >= 0)
			{
				PWMu1 = (1-uwm1-uvm1)/2;
				PWMw1 = PWMu1 + uwm1;
				PWMv1 = PWMw1 + uvm1;
			}
			else
			{
				PWMw1 = (1+uwm1+uum1)/2;
				PWMu1 = PWMw1 - uwm1;
				PWMv1 = PWMu1 - uum1;
			}
		}
		else
		{
			PWMu1 = (1+uum1+uvm1)/2;
			PWMv1 = PWMu1 - uum1;
			PWMw1 = PWMv1 - uvm1;
		}
	}


	//	Motor_2

	if (uum2 >= 0)
	{
		if (uvm2 >= 0)
		{
			PWMw2 = (1-uvm2-uum2)/2;
			PWMv2 = PWMw2 + uvm2;
			PWMu2 = PWMv2 + uum2;
		}
		else
		{
			if (uwm2 >= 0)
			{
				PWMv2 = (1-uum2-uwm2)/2;
				PWMu2 = PWMv2 + uum2;
				PWMw2 = PWMu2 + uwm2;
			}
			else
			{
				PWMv2 = (1+uvm2+uwm2)/2;
				PWMw2 = PWMv2 - uvm2;
				PWMu2 = PWMw2 - uwm2;
			}
		}
	}
	else
	{
		if (uvm2 >= 0)
		{
			if (uwm2 >= 0)
			{
				PWMu2 = (1-uwm2-uvm2)/2;
				PWMw2 = PWMu2 + uwm2;
				PWMv2 = PWMw2 + uvm2;
			}
			else
			{
				PWMw2 = (1+uwm2+uum2)/2;
				PWMu2 = PWMw2 - uwm2;
				PWMv2 = PWMu2 - uum2;
			}
		}
		else
		{
			PWMu2 = (1+uum2+uvm2)/2;
			PWMv2 = PWMu2 - uum2;
			PWMw2 = PWMv2 - uvm2;
		}
	}
	
	
	

	//	Ausgabe an den PWM-VController

	REG_PWM_CDTYUPD0 = (1 - PWMu1) * 2100;
	REG_PWM_CDTYUPD1 = (1 - PWMv1) * 2100;
	REG_PWM_CDTYUPD2 = (1 - PWMw1) * 2100;

	REG_PWM_CDTYUPD3 = (1 - PWMu2) * 2100;
	REG_PWM_CDTYUPD4 = (1 - PWMv2) * 2100;
	REG_PWM_CDTYUPD5 = (1 - PWMw2) * 2100;




	//	A Duty Cycle Update, Übernahme der Register Enable PWM channels (S.1016)
	//	- Register: PWM_SCUC (Sync Channel Update)
	//	- es gibt nur ein Bit in diesem Register:UPDULOCK (Unlock synchronous channels update)
	//	- wird es 1 gesetzt werden die Register für Duty Cycle ... übernommen

	//	Ausgabe
	REG_PWM_SCUC = 0x00000001u;
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx      SVPWM       xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx








//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx       ADC_ISR        xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

void ADC_Handler(void)
{		





//	Kommt alle 50us
//	Wird von der PWM Event Line getriggert
//	Impuls auf der EventLine wird mit jedem 10. PWM-Referenzimpuls erzeugt (1 x 100us = 100us) -> siehe Init_PWM


//	LOKALE VARIABLEN hier deklarieren

		
//	Disable Interrupt (CH7 vom ADC)
//	REG_ADC_IDR = REG_ADC_IDR | 0x00000080u;
//	Disable ADC interrupt (im Register ISER1 vom NVIC, leider hier nicht im Angebot, ASF)
//	NVIC_DisableIRQ(ADC_IRQn);


//	ADC_ISR Zähler inkrementieren
	ADC_ISR_CNT++;


/*
//	DEBUG
//	Debug_Pin C.24= 1 -> C.24 = 0 -> DUE_Pin6
	REG_PIOC_SODR	= REG_PIOC_SODR		|	0x01000000u; 		// Ausgang C24 =1 (DUE_Pin6)
	delay_us(10);
	REG_PIOC_CODR	= REG_PIOC_CODR		|	0x01000000u; 		// Ausgang C24 =0 (DUE_Pin6)
*/	




//	Hier wird jetzt die Mittelwertbildung (10-fach) durchgeführt

//	Analogkanäle lesen 1x/100us -> Snapshot
	SS_ADC_A0_i = REG_ADC_CDR7;									// Poti HapStik vertikal
	SS_ADC_A1_i = REG_ADC_CDR6;									// Poti HapStik horizontal
	SS_ADC_A2_i = REG_ADC_CDR5;									// Poti, Sollwertvorgabe
	
/*
//	PRINT
//	Jeden ADC-Wert printen
	printf("| SS_ADC_A0_i   : %15d|\r\n",
	SS_ADC_A0_i);
*/


	SUM_AF_i_1 = SUM_AF_i_1 + SS_ADC_A0_i;						// Summenbildung: aktueller ADC_A0-Wert wird dazu addiert
	SUM_AF_i_2 = SUM_AF_i_2 + SS_ADC_A1_i;						// Summenbildung: aktueller ADC_A1-Wert wird dazu addiert

	af_count_i ++;												// "af" -> Average-Filter
																
	PRINT_VERT_cnt_i = PRINT_VERT_cnt_i + 1;					// Zähler der Print-Ausgaben

		


	if (af_count_i == 20)										// 20 ADC-Werte wurden addiert, 1ms ist vorbei
	{
		
//		printf("|WERTE-VERTEILUNG-------------------------------------------------------------------------------------------------------------------|\r\n");

//		AF_A0_f = 0.5 + SUM_AF_i / 10.0;						// hier 0,5 zur Rundung dazu addieren
		
		AF_A0_f = SUM_AF_i_1 / 20.0;							// Addition von 0,5 kann aber auch entfallen, da der Absolutwert nicht so wichtig ist,
		AF_A1_f = SUM_AF_i_2 / 20.0;							// da ja alle Positionen beim Start des Sticks eingemessen werden

		AF_A0_i = AF_A0_f;										// Ganzzahliger Anteil wird übergeben
		AF_A1_i = AF_A1_f;
		
		af_count_i = 0;											// Counter auf Anfang stellen
		SUM_AF_i_1 = 0;											// Summe wieder rücksetzen
		SUM_AF_i_2 = 0;
				
		svpwm_int = 1;											// Flag setzen, damit Regelschleife mit 1ms läuft

																// -> zur Übergabe an Endlosschleife in MAIN
/*
//	PRINT
		printf(" A0:  %10d\r\n",	AF_A0_i);					//	nur den Average-Filter-Wert von ADC-Kanal A0 printen
		printf(" A1:  %10d\r\n",	AF_A1_i);					//	nur den Average-Filter-Wert von ADC-Kanal A1 printen
		printf(" A2:  %20d|\r\n\n", SS_ADC_A2_i); 
		delay_ms(500);
*/
	}
	
	
	

	

//	Enable Interrupt (CH7 vom ADC)
//	REG_ADC_IER = REG_ADC_IER | 0x00000080u;
//	Enable ADC interrupt, schreibt das Register ISER im NVIC (Nested Vector Interrupt Controller)
//	NVIC_EnableIRQ(ADC_IRQn);		
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx       ADC_ISR        xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx





//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    STROM       xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//	Input:
//		- Spannungsabfälle über den 3 externen shunts zur Strommessung:
//		- U_A0, U_A1, U_A2
//	Output:
//		- S_A0, S_A1, S_A2					absolute Spulenströme
//		- STROM_A0, STROM_A1, STROM_A2		normierte Spulenströme
//		- WINKEL_STROM						Winkel des magnetischen Vektors
//		- X_KOMP							X-Komponente des magnetischen Vektors
//		- Y_KOMP							Y-Komponente des magnetischen Vektors
//		- MV_LAENGE							Länge des magnetischen Vektors

void STROM(void)
{
//	Zähler inkrementieren 	
	STROM_CNT = STROM_CNT + 1;
//	printf("STROM-Nr.:   %d\r\n"	,STROM_CNT);	
	
//	-----
//	Ströme bilden:
//	Mittenwerte für die 3 Kanäle werden wie folgt ermittelt:
//		- LF = 0 setzen -> keine Ansteuerung der Spulen
//		- jetzt liegen an den Kanälen die Mittenspannungen an


//	S_A0	= U_A0 - 2.501;
//	S_A1	= U_A1 - 2.500;
//	S_A2	= U_A2 - 2.478;

//	ACHTUNG DEBUG
//	Hier noch einen Proportionalitätsfaktor (ADC_PROP) eingeführt, so dass in etwas die q-Vorgabe
//	zu adäquatem qcp [q nach Clarke_Park()] führt
	STROM_A0	= ADC_PROP * (U_A0 - 2.501);
	STROM_A1	= ADC_PROP * (U_A1 - 2.500);
	STROM_A2	= ADC_PROP * (U_A2 - 2.478);

//	-----
//	Die Notwendigkeit der Vektorlängennormierung ist fraglich! (23.12.2015)
//	Vektor-Längennormierung, so dass qmax = 1
//	Die Ergebnisse sind aber hilfreich bei der Beurteilung der Ergebnisse der MatLab-Simulation
//	Koordiantensystem-Transformation:  stern -> kartesisch
//	Aus den 3 Teilvektoren (STROM_A0, STROM_A1, STROM_A2) wird der Winkel (WINKEL_STROM) des resultierenden magnetischen 
//	Vektors zur x-Achse berechnet
 
	X_KOMP			= (2 * STROM_A0 - STROM_A1 - STROM_A2) / 2;
	Y_KOMP			= (sqrt(3.0) / 2.0) * (STROM_A1 - STROM_A2);
	MV_LAENGE		= sqrt((X_KOMP * X_KOMP) + (Y_KOMP * Y_KOMP));
	MV_LAENGE_REZ	= 1.5 * 1/MV_LAENGE;

	S_A0		= MV_LAENGE_REZ * STROM_A0;
	S_A1		= MV_LAENGE_REZ * STROM_A1;
	S_A2		= MV_LAENGE_REZ * STROM_A2;
//	-----



//	-----
//	Den magnetischen Winkel berechnen, gegen Excel geprüft i.O. (11.12.2015)
//	Aus den Teilkomponenten X_KOMP und Y_KOMP den Richtungskosinus berechnen (Kleine Enzyklopädie Mathematik, S.541)
//	180 / pi = 57.29578
//	Da der Kosinus zu pi spiegelsymmetrisch ist, müssen wir auf Minus Y_KOMP prüfen und wenn Y_KOMP negativ ist den Winkel als
//	(360 - WINKEL_STROM) bilden


//	Voehergehenden Stromwert retten, um eine Delta-Ausgabe zwischen 2 aufeinanderfolgenden Winkelwerten machen zu können

	WINKEL_STROM_ALT = WINKEL_STROM;

//	Neuen Winkel berechnen
	if(Y_KOMP < 0)
	{
		WINKEL_STROM = 360 - (57.29578 * acos(X_KOMP / sqrt((X_KOMP * X_KOMP) + (Y_KOMP * Y_KOMP))));
	}
	else
	{
		WINKEL_STROM =        57.29578 * acos(X_KOMP / sqrt((X_KOMP * X_KOMP) + (Y_KOMP * Y_KOMP)));
	}	
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx      STROM       xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx





//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    ENCODER     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//	Encoder Winkelberechnung
//	Es wird das Register: REG_TC0_CV0 ausgelesen, in dem sich der Zählerstand des Encoders MEC22 befindet: +- 2147483648
//	Output:
//		- ENC_UMD			Umdrehunugen in int
//		- ENC_WKL			Winkel absolut, bezogen auf eine mechanische Umdrehung des Rotors in float
//		- ENC_WKL_MOT		Winkel relativ, bezogen auf eine magnetische Umdrehung des Rotors in float (7x größer)
//		- ENC_UMD_MOT		Anzahl elektrischer Umdrehungen des Motors
	
//	ACHTUNG:
//	Das bedeutet, dass nach 268435,456 Umdrehungen ein turn_arround stattfindet

void ENCODER(void)
{

//	Zähler inkrementieren 	
	ENCODER_CNT = ENCODER_CNT + 1;
//	printf("ENCODER-Nr.:   %d\r\n"	,ENCODER_CNT);
	
//	ACHTUNG DEBUG -> damit sich der Rotor erst einmal einschwingen kann
//	delay_ms(250);	
	
//	Winkel Rotor
	ENC_CNT =  REG_TC0_CV0 - ENC_UMD * 8000;
	
	if(ENC_CNT >= 8000)
	{
		ENC_UMD++;
	}
	if(ENC_CNT <= -8000)
	{
		ENC_UMD--;
	}

	ENC_WKL			= (360.0 / 8000.0) * ENC_CNT;						// z.B. ENC_CNT=8000 -> ENC_WKL=360
	ENC_WKL_RAD		= ENC_WKL * PI / 180;

//	Winkel Motor
	ENC_WKL_MOT_INT	= (ENC_WKL * 7) / 360;								// Ganzzahliger Anteil elektrischer Umdrehungen
																		// Anzahl magnetischer Umdrehungen auf einer mech.
																		// Umdrehung (0...6)
	ENC_WKL_MOT		= 360 * ((ENC_WKL * 7) / 360 - ENC_WKL_MOT_INT);	// Rest mal 360°
	ENC_WKL_MOT_RAD	= ENC_WKL_MOT * PI / 180;

//	Anzahl elektrischer Umdrehungen des Motors
	ENC_UMD_MOT		= (ENC_UMD * 7) + ENC_WKL_MOT_INT;
	
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    ENCODER     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx





//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    CLARKE_PARK       xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//	Clarke Park Transformation (MATLAB-Block "Clarke Park")
//	Input:
//		- STROM_A0, STROM_A1, STROM_A2 aus Funktion STROM()
//		- ENC_WKL_MOT_RAD
//	Output:
//		- alpha, beta
//		- Dcp, Qcp (d, q)
 
void CLARKE_PARK(void)
{
//	ACHTUNG DEBUG -> hier  + 60°, hat die MATLAB-Simu ergeben
	ENC_WKL_MOT60		= -ENC_WKL_MOT - 120;
	ENC_WKL_MOT60_RAD	= ENC_WKL_MOT60 * PI / 180.0;

//	alpha	= 0.666666 * (STROM_A0 - (STROM_A1 / 2) - (STROM_A2 / 2) );
//	beta	= 0.57735  * STROM_A1 - 0.57735 * STROM_A2;
	alpha	= (2.0 / 3.0) * (STROM_A0 - (STROM_A1 / 2) - (STROM_A2 / 2) );
	beta	= (2.0 / 3.0) * ((sqrt(3.0) / 2.0) * STROM_A1 - (sqrt(3.0) / 2.0) * STROM_A2);	

	Dcp = cos(ENC_WKL_MOT60_RAD)		* alpha + sin(ENC_WKL_MOT60_RAD) * beta;
	Qcp = cos(ENC_WKL_MOT60_RAD)		* beta  - sin(ENC_WKL_MOT60_RAD) * alpha;		 	
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    CLARKE PARK       xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx





//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    INV_CLARKE_PARK       xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//	Inverse Clarke Park Transformation
//	MATHLAB-Block "Modifizierte Inverse Clarke Park"
//	Input:
//		- Dpi, Qpi
//		- ENC_WKL_MOT_RAD
//	Output:
//		- X, Y, Z

void INV_CLARKE_PARK(void)
{
	alpha	= cos(ENC_WKL_MOT_RAD) * Dpi - sin(ENC_WKL_MOT_RAD) * Qpi;
	beta	= sin(ENC_WKL_MOT_RAD) * Dpi + cos(ENC_WKL_MOT_RAD) * Qpi;

//	X = beta								* LF/100;
//	Y = 0.5 * (sqrt(3.0) * alpha - beta)	* LF/100;
//	Z = (-0.5) * (beta + sqrt(3.0) * alpha)	* LF/100;

	X1 = beta;
	Y1 = 0.5 * (sqrt(3.0) * alpha - beta);
	Z1 = (-0.5) * (beta + sqrt(3.0) * alpha);
		
/*
//	Begrenzung
		if(X > 1)
		{
			X = 1;
		}

		if(X < -1)
		{
			X = -1;
		}

		if(Y > 1)
		{
			Y = 1;
		}

		if(Y < -1)
		{
			Y = -1;
		}

		if(Z > 1)
		{
			Z = 1;
		}

		if(Z < -1)
		{
			Z = -1;
		}
*/
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    INV_CLARKE PARK       xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx




 



//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    MOTOR DREHT       xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//	Motor dreht sich
//	Funktion muss in einer Loop stehen mit MOTOR_DREHT_CNT als Laufvariablen
//	z.B.
//	MOTOR_DREHT(1, -141, 7*360, 1);   -> Rechtslauf, -141° magnetischer Offset, 7*360 Schritte, 1° magnetisch
//		- DR		Drehrichtung, 1 -> rechts, 0 -> links
//		- M_OFFSET	Offset in magnetischen ° -> positiver Offset führt zu Drehung links
//		- M_STEPS	Anzahl von Motor-Ausgabezyklen
//		- EW_STEP	Schritthub (z.B. 1 -> ein elektrisches Grad)

void	MOTOR_DREHT(int DR, int M_OFFSET, int M_STEPS, float EW_STEP)
{	

//	Zähler inkrementieren 	
	MOTOR_DREHT_CNT = MOTOR_DREHT_CNT + 1;
		
//	Rechtslauf? Linkslauf?
	if (M_STEP_Z < M_STEPS) 
	{
//	Motor dreht nach links
		if (DR == 0)
		{
			DWE1 = DWE1 - EW_STEP;
			M_STEP_Z++;		
		}	
		else
		{
//	Motor dreht nach rechts
			DWE1 = DWE1 + EW_STEP;
			M_STEP_Z++;		
		}	
	}
	

	
//	Debug_Pin C.25 = 1
//	REG_PIOC_SODR	= REG_PIOC_SODR		|	0x02000000u;



//	Aus dem magnetischen Winkel die X, Y, Z - Anteile berechnen
	X1 = cos((DWE1 + M_OFFSET) * WK1)			* LF1/100;
	Y1 = cos((DWE1 + M_OFFSET) * WK1 - WK2)		* LF1/100;
	Z1 = cos((DWE1 + M_OFFSET) * WK1 - WK3)		* LF1/100;		// 60us -> der "cos" braucht die Zeit, Format des Argumentes ist egal
		
//	Debug_Pin 8 = 0
	REG_PIOC_CODR	= REG_PIOC_CODR		|	0x00400000u;

//	SpaceVektor-PWM-Ausgabe
//	TIME: 17us
	SVPWM(X1, Y1, Z1, 0, 0, 0);					

//	Debug_Pin 7 = 0
	REG_PIOC_CODR	= REG_PIOC_CODR		|	0x00800000u;		
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    MOTOR DREHT       xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx





//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx     Init PWM       xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
/*
	PWM (S.979)
			- Ausgabe eines Referenzsignals an PWMH3 (4. PWM-Kanal, Port: C.9):
				- 1us Impuls, Wiederholrate 100us:	
				- ..._______-___________________-___________________-___________________-______________
			- PWM-Kanäle sind auf low 		 
			  
			- PWM Kanäle:	- 0...2 -> center aligned
						  		- Periode 100us
							- 3		-> left	aligned
								- Länge 1us
						  
			|				|				|				|				|				|				|			
			FUNKTION		PWM-Channel		DUE-Pin			PORT			PERIPHERAL
			-------------------------------------------------------------------------------------------------
			X high			H0				35				C.3				B
			X low			L0				34				C.2				B
			Y high			H1				37				C.5				B
			Y low			L1				36				C.4				B
			Z high			H2				39				C.7				B
			Z low			L2				38				C.6				B
			Referenz high	H3				41				C.9				B
			Referenz low	L3				40				C.8				B	
			


Neu für Ansteuerung von 2 Motoren			
			|				|				|				|				|				|				|
			FUNKTION		PWM-Channel		DUE-Pin			PORT			PERIPHERAL
			-------------------------------------------------------------------------------------------------

			Motor_1			
			X low			L0				34				C.2				B
			Y low			L1				36				C.4				B
			Z low			L2				38				C.6				B
			
			Motor_2
			X low			L3				40				C.8				B
			Y low			L4				9				C.21			B
			Z low			L5				8				C.22			B			
							
			Referenz low	L6				7				C.23			B			
*/



void	INIT_PWM(void)
{
/*A	
	Instanzen (S.38)
	- ID_PWM: 36 (Identifier für die Pulsweitenmodulation PWM)
	- Clock enable für PWM
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
	- entfällt hier, da alle PWM-Kanäle mit MCK (84 MHz) getaktet werden
*/

/* 
	Synchronität, Kanäle 0,1,2,3,4,5 als synchrone Kanäle (S.1014) definieren:
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
	Kanal Mode, Aligment, Polarität, Dead Time (S.1044)
	- Register: PWM_CMRx (Channel Mode)
	- Dead Time:
		- Überlappungsfreiheit von komplementären Ausgängen, damit nicht beide MOSFETs kurz gleichzeitig eingeschalten
		- DTE=1 (Dead Time Generator Enable)
*/	
//	- Kanal 0:
//		- center aligned, Start mit high-Level, hier Dead Time enable für Ansteuerung Renesas-Board
//	REG_PWM_CMR0 = REG_PWM_CMR0		|	0x00010300u;	


//	- Kanal 0:
//		- center aligned, Start mit high-Level, hier Dead Time disable für Ansteuerung DS8313 (Leistungstreiber H-Brücke)
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
	Erklärung: waveform period (S.978)
	- Register: PWM_CPRD
	- Periode = CPRD/84MHz = 8400/84MHz = 100us
	- Periode = CPRD/84MHz = 4200/84MHz =  50us		


	Werte für 100us:
	REG_PWM_CPRD0 = REG_PWM_CPRD0	|	0x00001068u;		// 4200
	REG_PWM_CPRD1 = REG_PWM_CPRD1	|	0x00001068u;		// 4200	
	REG_PWM_CPRD2 = REG_PWM_CPRD2	|	0x00001068u;		// 4200

	REG_PWM_CPRD3 = REG_PWM_CPRD3	|	0x00001068u;		// 4200
	REG_PWM_CPRD4 = REG_PWM_CPRD4	|	0x00001068u;		// 4200
	REG_PWM_CPRD5 = REG_PWM_CPRD5	|	0x00001068u;		// 4200
	
	REG_PWM_CPRD6 = REG_PWM_CPRD6	|	0x000020D0u;		// 8400
*/

//	Werte für 50us:
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
	- ursprünglich:
		- PWM->PWM_CH_NUM[0].PWM_CDTY = (1 - DC_M1_X) * 4200;
		- da aber DC_M1_X = 0 kann man kürzer schreiben:
	PWM->PWM_CH_NUM[0].PWM_CDTY = 4200;	
*/

	REG_PWM_CDTY0 = REG_PWM_CDTY0	|	0x00000834u;		// 2100
	REG_PWM_CDTY1 = REG_PWM_CDTY1	|	0x00000834u;		// 2100
	REG_PWM_CDTY2 = REG_PWM_CDTY2	|	0x00000834u;		// 2100	

	REG_PWM_CDTY3 = REG_PWM_CDTY3	|	0x00000834u;		// 2100
	REG_PWM_CDTY4 = REG_PWM_CDTY4	|	0x00000834u;		// 2100
	REG_PWM_CDTY5 = REG_PWM_CDTY5	|	0x00000834u;		// 2100
	
	REG_PWM_CDTY6 = REG_PWM_CDTY6	|	0x00000054u;		// 84	für 1us-Impuls (Referenzsignal für Oszi)

//	----------			
/* 
	Wir wollen nach dem Start eines SVPWM-Zyklus, 1us vergehen lassen und dann den ADC starten
	Das ist ein günstiger Zeitpunkt für die Strommessungen!!!
	
	PWM-Comparison Unit  (S.993)
		- Register: PWM Comparison x Mode Register (S.1042)
			- comparison x (x= 0...7, 8 PWM-Kanäle) erlauben, Bit CEN = 1
			- wir benutzen den Comparison 0
*/


//	Impuls auf der EventLine wird mit jedem 1. PWM-Referenzimpuls erzeugt (1  x 50us = 50us)
	REG_PWM_CMPM0 = REG_PWM_CMPM0	|	0x00000001u;

//	Impuls auf der EventLine wird mit jedem 2. PWM-Referenzimpuls erzeugt	
//	REG_PWM_CMPM0 = REG_PWM_CMPM0	|	0x00000101u;

//	Impuls auf der EventLine wird mit jedem 3. PWM-Referenzimpuls erzeugt	
//	REG_PWM_CMPM0 = REG_PWM_CMPM0	|	0x00000201u;	

//	Impuls auf der EventLine wird mit jedem 4. PWM-Referenzimpuls erzeugt
//	REG_PWM_CMPM0 = REG_PWM_CMPM0	|	0x00000301u;

//	Impuls auf der EventLine wird mit jedem 5. PWM-Referenzimpuls erzeugt
//	REG_PWM_CMPM0 = REG_PWM_CMPM0	|	0x00000401u;	

//	Impuls auf der EventLine wird mit jedem 10. PWM-Referenzimpuls erzeugt (10 x 100us = 1ms)
//	REG_PWM_CMPM0 = REG_PWM_CMPM0	|	0x00000901u;

/*	
	PWM Comparison x Value Register (S.1040)
		- Register: PWM_CMPVx
		- comparison x value to be compared with the counter of the channel 0
		- wenn dieser Wert erreicht wird, wird "Comparison x" aktiv, was als Input für den Start des ADC genutzt wird
		- ACHTUNG:
			- der Wert wird auf 1us oder 12ns gesetzt 
				-> der optimale Abtastzeitpunkt muss mit dem Oszi ermittelt werden 
			- wenn also auf dem Oszi die fallende Flanke des Referenzsignals zu sehen ist, so wird in diesem Moment
			  der ADC gestartet -> ist hier noch anpassbar
		- Bit CVM: the comparison x between the counter of the channel 0 and the comparison x value is performed when this counter is
		  incrementing (bei center aligned, so ist ja Kanal 0 eingestellt, kann man hier auch "decrementing" wählen)	  
*/
	REG_PWM_CMPV0 = REG_PWM_CMPV0	|	0x00000001u;		// 0x00000054u -> 1us,  dann Start des ADC
															// 0x00000001u -> 12ns, dann Start des ADC (ist der kleinste Wert)	

/*A 	
	PWM Event Line x Register (S.995)
		- Register: PWM_ELMRx (S.1035)
		- a pulse is generated on the event line x when the comparison 0...7 match
		- wir benutzen den Comparison 0
*/
	REG_PWM_ELMR = REG_PWM_ELMR		|	0x00000001u;		// Comparison 0 ausgewählt
//	----------	

/*A 
	PWM-Leitungen (C.2, C.4, C.6, C.8, C.21, C.22, C.23) im Prozessor vom PIO-Controller trennen
	- Register: PIO_PDR (PIO Disable), S.634
	- zugehörige Bits auf 1 setzen
	- macht sich im Register: PIO_PSR (PIO Status) als 0 an den zugehörenden Bits bemerkbar!
*/
//	REG_PIOC_PDR = REG_PIOC_PDR		|	0x000003FCu;		// -> Renesas
	REG_PIOC_PDR = REG_PIOC_PDR		|	0x00E00154u;		// -> DS8313

/*A 
	PWM-Leitungen (C.2, C.4, C.6, C.8, C.21, C.22, C.23) auf "peripheral B function" stellen
	- Register: PIO_ABSR (PIO AB Select), S.656
	- zugehörige Bits auf 1 setzen
*/
//	REG_PIOC_ABSR = REG_PIOC_ABSR	|	0x000003FCu;		// -> Renesas
	REG_PIOC_ABSR = REG_PIOC_ABSR	|	0x00E00154u;		// -> DS8313	

/*A 
	Dead Time festlegen: 1us (S:1051)
	- Register: PWM_DT, die obere Registerhälfte enthält DTL und die untere Hälfte DTH
	- Dead Time = DTH/84MHz x 42 = 500ns -> 0x002A002Au;
	  oder:
	- Dead Time = DTH/84MHz x 84 = 1us -> 0x00540054u;

	REG_PWM_DT0 = 0x00540054u;
	REG_PWM_DT1 = 0x00540054u;		
	REG_PWM_DT2 = 0x00540054u;
	-> keine Dead time für DS8313 nötig, da keine komplementären Ausgänge	
*/

	
/*A 
	Enable PWM channels (S.1007)
	- gleichzeitige Start von Kanal 0 (steht stellvertretend für Kanäle 0...5 und Kanal 6 (Referenzimpuls), damit alle Kanäle 
	  den gleichen zeitlichen Nullpunkt haben.
	- initiale Ausgabe und Freischaltung der PWMs für Motor_1 und Motor_2:
*/
	REG_PWM_ENA = REG_PWM_ENA	|	0x00000041u; 


}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx     Init PWM       xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx





//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx     Init TC2 CH1 und CH2 xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//	REG_TC2_IER1
//	NVIC_DisableIRQ(TC2_IRQn);


/*	Decodierung eines PPM-Signals
	Blockbild S.864
	Es gibt 3 Timer/Counter Module.
	Jedes Modul hat 3 Channels, so dass:
				 | TC0 CH0 | TC0 CH1 |TC0 CH2 |
				 | TC1 CH0 | TC1 CH1 |TC1 CH2 |
				 | TC2 CH0 | TC2 CH1 |TC2 CH2 |				 		
	Wir benutzen: TC2 CH1, CH2
	
	Erste Variante:
		- Ausgangssignal eines Empfängerkanals (1,5ms +/- 0,5ms) als Input
		- Input an TIOA7, C.28 (TC2, CH1)
		- steigende Flanke	-> Trigger (Reset des Counters)
		- fallende Flanke	-> capture

		- TIOA7 soll den Counter mit der steigenden Flanke triggern (Reset)
			- ABETRG: TIOA or TIOB External Trigger Selection
				0: TIOB is used as an external trigger.
				1: TIOA is used as an external trigger		
*/


void	INIT_TC2(void)
{

/*	S.563
	PCM (Power Management Controller) konfigurieren -> enable Timer-Counter-Clock
	Register: PMC_PCER1 (Power Management Controller Peripheral Clock Enable Register 1)
		- Clock enable für TC2 CH1 -> TC
		- Instanz TC7: S.39
			- ID_TC7: 34 (Identifier für TC2 CH1 -> TC7)
			- ID_TC7: 35 (Identifier für TC2 CH2 -> TC8)			
*/
	REG_PMC_PCER1 = REG_PMC_PCER1	|	0x0000000Cu;


/*	S.880
	Channel Control Register
	Clock für TC2, CH1 erlauben
		- CLKEN = 1
	Clock für TC2, CH2 erlauben
		- CLKEN = 1		
*/
	REG_TC2_CCR1 = REG_TC2_CCR1		|	0x00000001u;
	REG_TC2_CCR2 = REG_TC2_CCR2		|	0x00000001u;	
	
/*	S.881
	Channel Mode
		- _ _ _ 1   0 5 0 2
		- TC_CMR (Timer / Counter Channel Mode Register)
		- clock selection:	 TIMER_CLOCK3 = MCK/8
		- clock nicht invertiert

		- ABETRG = 1: -> 1: TIOA7 is used as an external trigger
		- steigende Flanke	-> Trigger (Reset des Counters)
		- ETRGEDG = 1H
		- ETRGEDG = 2H (fallende Flanke)

		- fallende Flanke für (RA Loading Edge Selection) -> zum Laden des Capture-Registers RA
		- LDRA = 1H
		- LDRA = 2H (fallende Flanke)
		
	Was bedeutet den Counter mit MCK/32 zu takten?
		- 10500 Clocks je Milisekunde (84000/8)
		- 5250 Clocks je 0,5ms (Vollauslenkung des Servos)
		- also haben wir eine Winkelauflösung in der Abtastung von ca. 60°/5250 = 0,0114° -> geht in Ordnung, da der Regler
		  einen Input mit einer Auflösung von 0,02° bekommt und der Impuls am Empfängerausgang nur eine Auflösung von 1/1024 hat! 
		  Egal! Vielleicht wird sie ja mal besser! ;-)
		- Damit sollte der Wertebereich des Capture-Registers RA sein:	*** 5250 ... 10500 ... 15750 ***  	
		
*/
//	REG_TC2_CMR1 = REG_TC2_CMR1		|	0x00020503;			// MCK/128
//	REG_TC2_CMR1 = REG_TC2_CMR1		|	0x00020500;			// MCK/2
	REG_TC2_CMR1 = REG_TC2_CMR1		|	0x00020501;			// MCK/8

	REG_TC2_CMR2 = REG_TC2_CMR2		|	0x00020501;			// MCK/8
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx     Init TC        xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx





//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx     Init GPIO      xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//	Ports: PC24 und PC25 als Outputs definieren
//	--------------------------------------------------------------------------------------------------------------------------

void	INIT_GPIO(void)
{

/*
1.	Pull-up Resistor Control (Pullup Widerstand am Pin zuschalten)
		- PIO_PUER (Pull-up Enable Register) enable Pullup
		- PIO_PUDR (Pull-up Disable Resistor) disable Pullup
		- PIO_PUSR (Pull-up Status Register), hier steht das Ergebnis
		- nach Reset -> all of the pull-ups are enabled, i.e. PIO_PUSR resets at the value 0x0.
*/

/*				
2.	I/O Line or Peripheral Function Selection
		- PIO_PER (PIO Enable Register, S.633)
		- PIO_PDR (PIO Disable Register)
		- PIO_PSR (PIO Status Register), hier steht das Ergebnis
			- 0 -> pin is controlled by the corresponding on-chip peripheral selected in the PIO_ABSR (AB Select Register)
			- 1 -> pin is controlled by the PIO controller
		- nach Reset -> generally, the I/O lines are controlled by the PIO controller, i.e. PIO_PSR resets at 1.

		- PC21, PC22, PC23, PC24, PC25 sind I/O Lines -> 0x03E00000u
		-                   PC24, PC25 sind I/O Lines -> 0x03000000u -> die anderen werden für Ansteuerung Motor_2 benötigt
*/
	REG_PIOC_PER	= REG_PIOC_PER		|		0x03000000u;



	
/*
3.	Output Control (S.636)
		- wenn die Outputs vom PIO-Controller definiert werden (zugehörenden Bits in PIO_PSR sind 1)
		- PIO_OER (Output Enable Register) treibt den Pin
		- PIO_ODR (Output Disable Register) Pin wird nicht getrieben
		- PIO_OSR (Output Status Register), hier steht das Ergebnis
			- 0 -> I/O line is used as an input
			- 1 -> I/O line is driven by the PIO controller
		- PIO_SODR (Set Output Data Register)
		- PIO_CODR (Clear Output Data Register)
		- PIO_ODSR (Output Data Status Register), , hier steht das Ergebnis
			- 1 -> eine 1 am Ausgang
			- 0 -> eine 0 am Ausgang

		- Ports: PC21, PC22, PC23, PC24 und PC25 als Outputs definieren -> 0x03E00000u
*/
//	REG_PIOC_OER	 = REG_PIOC_OER		|		0x03E00000u;		
	REG_PIOC_OER	 = REG_PIOC_OER		|		0x03000000u;		// -> die anderen werden für Ansteuerung Motor_2 benötigt			
/*			
4.	Synchronous Data Output	(S.623)		
		- nach Reset -> the synchronous data output is disabled on all the I/O lines as PIO_OWSR resets at 0x0.
*/


/*
5.	Multi Drive Control (Open Drain)
		- nach Reset -> the Multi Drive feature is disabled on all pins, i.e. PIO_MDSR resets at value 0x0.
*/


/*
	Zum Debuggen -> 5 Pins ein- und ausschalten:
	
		- PIO Controller Set   Output Data Register (S.642)	
		- PIO Controller Clear Output Data Register (S.643)

	REG_PIOC_SODR	= REG_PIOC_SODR		|	0x03E00000u; 		// Ausgänge C21, C22, C23, C24, C25 auf 1 setzen	
	REG_PIOC_CODR	= REG_PIOC_CODR		|	0x03E00000u; 		// Ausgänge C21, C22, C23, C24, C25 auf 0 setzen	

	REG_PIOC_SODR	= REG_PIOC_SODR		|	0x0200 0000u; 		// Ausgang C25 =1 (DUE_Pin 5)	
	REG_PIOC_CODR	= REG_PIOC_CODR		|	0x0200 0000u; 		// Ausgang C25 =0 (DUE_Pin 5)	
	
	REG_PIOC_SODR	= REG_PIOC_SODR		|	0x0100 0000u; 		// Ausgang C24 =1 (DUE_Pin 6)
	REG_PIOC_CODR	= REG_PIOC_CODR		|	0x0100 0000u; 		// Ausgang C24 =0 (DUE_Pin 6)	


->  diese hier sind nicht mehr verfübar -> werden für Ansteuerung Motor_2 benötigt	
	REG_PIOC_SODR	= REG_PIOC_SODR		|	0x0080 0000u; 		// Ausgang C23 =1 (DUE_Pin 7)
	REG_PIOC_CODR	= REG_PIOC_CODR		|	0x0080 0000u; 		// Ausgang C23 =0 (DUE_Pin 7)
	
	REG_PIOC_SODR	= REG_PIOC_SODR		|	0x0040 0000u; 		// Ausgang C22 =1 (DUE_Pin 8)
	REG_PIOC_CODR	= REG_PIOC_CODR		|	0x0040 0000u; 		// Ausgang C22 =0 (DUE_Pin 8)
	
	REG_PIOC_SODR	= REG_PIOC_SODR		|	0x0020 0000u; 		// Ausgang C21 =1 (DUE_Pin 9)
	REG_PIOC_CODR	= REG_PIOC_CODR		|	0x0020 0000u; 		// Ausgang C21 =0 (DUE_Pin 9)
*/
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx     Init GPIO      xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx





//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx     Init ADC       xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
/*	
	Analoge Eingänge: (S.1320)
	- DUE: A0	SAM: A16		ADC: AD7
	- DUE: A1	SAM: A24		ADC: AD6		-> HS, vertikal ?
	- DUE: A2	SAM: A23		ADC: AD5		-> HS, horizontal ?
	Ergebnisse:
	- REG_ADC_CDR[5]
	- REG_ADC_CDR[6]
	- REG_ADC_CDR[7]
	Start (Software, zunächst):
	- REG_ADC_CR = 0x0000 0002;
	S&H ab Start-Kommando:
	- 476ns
	- 1666ns
	- 2856ns
*/	

void	INIT_ADC(void)
{
/*A 
	ADC-Leitungen (C.2 - C.9) im Prozessor vom PIO-Controller trennen
	- Register: PIO_PDR (PIO Disable), S.634
	- zugehörige Bits auf 1 setzen
	- macht sich im Register: PIO_PSR (PIO Status) als 0 an den zugehörenden Bits bemerkbar!
*/
	REG_PIOC_PDR = REG_PIOC_PDR		|	0x000003FCu;


/*	Clock enable für ADC
	Instanzen (S.38)
	- ID_ADC: 37 (Identifier für die ADC)
		- S.563
		- Register: PMC_PCER1 (Power Management Controller Peripheral Clock Enable Register 1)
*/
	REG_PMC_PCER1 = REG_PMC_PCER1	|	0x00000020u;

//	Reset ADC (S.1332)	
	REG_ADC_CR = 0x00000001;	

/*	ADC Mode (S.1333)
	- Register ADC_MR (Mode Register)
		- Hardware_Trigger enable -> PWM Event Line 0
			- ADC_TRIG4 (PWM Event Line 0)
		- 12Bit
		- Sleep aus (ADC zwischen den Wandlungen immer aktiv)
		- Freerun aus -> Normal Mode
		- elektrische Daten ADC auf S.1403
			- max. fADC =22MHz -> 84MHz/4=21MHz -> PRESCAL=1
			- StartUp Time 0, weil ADC immer aktiv -> STARTUP=0
			- Settling Time min. 200ns -> 4,2 ADC-Clockperioden -> SETTLING = 1 (5 periods)
		- kein Wechsel der analogen Einstellungen bei Kanalwechsel -> ANACH = 0
		- TrackTime (wahrscheinlich Einschwingen S&H) zunächst auf 9 -> 9+1 = 10 ... 10 * Periode ADC (47,6ns) = 476ns, -> TRACTIM=9
		- TransferTime (1*2+3)*Periode ADC (47,6ns) = 238ns
			-	1.S&H -> 476ns
				2.S&H -> 476ns  + 1190ns = 1666ns
				3.S&H -> 1666ns + 1190ns = 2856ns und im weiteren immer jeweils nach: ConvTime + TransferTime = 952ns + 238 = 1190ns 	
			-	Gemessen habe ich in Summe, vom Start bis Debug_Pin in ADC_Handler() =1 setzen -> 3660 ns
				-> passt recht ordentlich!
		- USEQ = 0, keine bestimmte Kanalsequenz, Normal Mode: The controller converts channels in a simple numeric order.	
		
		-> ADC_MR = 1E900209H
					   
*/


/*				- Zusammenfassung der Parameter: (neu definiert 06.10.2016)
				- 15kOhm	Quellwiderstand
				- 14MHz		ADC clock
				- 71,43ns	ADCClock period
				- 1,071us	Tracktime
				- 357,15ns	TRANSFER period
				- 357,15ns	SETTLING period (Einschwingen beim Übergang zum nächsten ADC-Kanal, z.B. andere Verstärkung, etc.
*/								
//	REG_ADC_MR = REG_ADC_MR			| 0x19100100u;				// Software_Trigger
//	REG_ADC_MR = REG_ADC_MR			| 0x19100109u;				// Hardware_Trigger -> PWM Event Line 0 
//	REG_ADC_MR = REG_ADC_MR			| 0x1E900209u;				// neu definiert 06.10.2016
	
	
	
	
	
	
	
	
//	TEST
//	2k + 10k als Spannungsteiler direkt über die Referenz gelötet und den Abgriff auf A0 in der Leiste eingespeist
//	Steckbrücke über A0 und A1 (beide sehen das selbe)
//	PGA: x1
	 
//	REG_ADC_MR = REG_ADC_MR			| 0x1E900209u;				// AUSGANGSSITUATION -> neu definiert 06.10.2016

//	REG_ADC_MR = REG_ADC_MR			| 0x1F900209u;				// TRACKTIM	(15 + 1) * 71,4 = 1142ns
																// -> keine Änderung ADC-Hub: 4 
//	REG_ADC_MR = REG_ADC_MR			| 0x10900209u;				// TRACKTIM	(0  + 1) * 71,4 = 71,4ns
																// -> keine Änderung ADC-Hub: 4 
//	REG_ADC_MR = REG_ADC_MR			| 0x3E900209u;				// TRANSFER von 1 auf 3 erhöht (max.) 
																// -> keine Änderung ADC-Hub: 4 
//	REG_ADC_MR = REG_ADC_MR			| 0x1EB00209u;				// SETTLING von 1 auf 3 erhöht (max.) 
																// -> keine Änderung ADC-Hub: 4 
//	REG_ADC_MR = REG_ADC_MR			| 0x1E9F0209u;				// STARTUP von 0 auf 15 erhöht (max.) 
																// -> keine Änderung ADC-Hub: 4 

//	REG_ADC_MR = REG_ADC_MR			| 0x1E90FF09u;				// PRESCAL von 2 auf 15 erhöht (max./2) 
																// -> keine Änderung ADC-Hub: 4 

//	REG_ADC_MR = REG_ADC_MR			| 0x3FBFFF09u;	



//	Einspeisung konstanter Wert: 2k + 10k als Spannungsteiler direkt über die Referenz gelötet und den Abgriff auf A0
//	in die DUE-Leiste eingespeist
//	2,2uF Tantal-Kondensator über AVREF (MAX 6066 -> 2,5V)
//	PRINT_VERT_cnt: > 2000


//	10 Bit / x1  -> ADC-Werte: 2
//	REG_ADC_MR = REG_ADC_MR			| 0x1E900219u;				// LOWRES von 0 auf 1 (12Bit -> 10Bit) 


//	10 Bit / x2  -> ADC-Werte: 3
//	REG_ADC_MR = REG_ADC_MR			| 0x1E900219u;				// LOWRES von 0 auf 1 (12Bit -> 10Bit) 


//	10 Bit / x4  -> ADC-Werte: 6 (731 ... 736)
//	REG_ADC_MR = REG_ADC_MR			| 0x1E900219u;				// LOWRES von 0 auf 1 (12Bit -> 10Bit) 


//	-> DAS BEDEUTET, dass die Änderung des Signals schon vor dem PGA stattfinden muss
//	   Vorausgesetzt natürlich, dass der PGA keinen zusätzlichen Fehler mit rein bringt!





//	12 Bit / x1  -> ADC-Werte: 8 (723 ... 730)
//	Verteilung VW_MW = 727;
//	REG_ADC_MR = REG_ADC_MR			| 0x1E900209u;				// LOWRES von 1 auf 0 (10Bit -> 12Bit)	
//	|	723		|	724		|	725		|	726		|	727		|	728		|	729		|	730		|	731		|	732		|
//	|	2  		|	27 		|	557		|	1307	|	522		|	310		|	42 		|	1  		|	0  		|	0  		|



//	12 Bit / x4  -> ADC-Werte: 21 (2926 ... 2946)
//	Verteilung VW_MW = 2936;

	REG_ADC_MR = REG_ADC_MR			| 0x1E900209u;				// LOWRES von 1 auf 0 (10Bit -> 12Bit)
//	12 Bit / x4  -> ADC-Werte: 10 (2931 ... 2940) nach 1000 Messungen


	
//	REG_ADC_MR = REG_ADC_MR			| 0x3FBFFF09u;				// allet max
//	12 Bit / x4  -> ADC-Werte: 13 (2930 ... 2942) nach 1000 Messungen

//				-> ADC-Werte: 21 (2926 ... 2946)		
//	|	2932	|	2933	|	2934	|	2935	|-	2936   -|	2937	|	2938	|	2939	|	2940	|	2941	|
//	|	38 		|	226		|	749		|	1434	|	1398	|	826		|	336		|	80 		|	18 		|	3  		|


//	-> DAS BEDEUTET auch hier:
//		- dass die Änderung des Signals schon vor dem PGA stattfinden muss
//		- UND, dass die Veränderung der ADC-Parameter, auf "allet max" (siehe oben) auch keine Verbesserung bringt
//		-> damit ist z.B. die Tracktime nicht zu knapp bemessen, etc. ... !!! und 
//		   REG_ADC_MR = REG_ADC_MR			| 0x1E900209u; vollkommen in Ordnung.	
















/*	Kanäle enable
		- Register: ADC_CHER (ADC Channel Enable Register), S.1338
		- Kanäle Ch5, CH6, CH7 erlauben
		-> ist im ADC_CHSR (ADC Channel Status Register) zu sehen		
*/
	REG_ADC_CHER = REG_ADC_CHER		| 0x000000E0u;



/*	Kanäle Verstärkung
		- Register: ADC_CGR (ADC Channel Gain Register), S.1349

	REG_ADC_CGR = REG_ADC_CGR		| 0xFFFFFFFFu;		//Gain = 4 für alle Kanäle
	REG_ADC_CGR = REG_ADC_CGR		| 0x00003C00u;		//Gain = 4 für Kanäle: Ch5, CH6	
	REG_ADC_CGR = REG_ADC_CGR		| 0x0000FC00u;		//Gain = 4 für Kanäle: Ch5, CH6, CH7

	ACHTUNG: bei neuem ADC-Kanal hier noch einmal genau die Zuordnung prüfen:
	Das hier:
	REG_ADC_CGR = REG_ADC_CGR		| 0x00003C00u;		//Gain = 4 für Kanäle: Ch5, CH6	
	... haut nicht hin
	Nur wenn alle Kanäle mit x4 eingestellt werden, erwische ich CH5 und CH6, also die HS-Potis, um die es hier geht
	Tschuldigung, habe jetzt hier keine Zeit (... eher keine Lust ;-) das zu prüfen
	06.10.2016
	Ist geklärt. Entscheidendist das Bit ANACH. Es muss 1 sein, wenn man zwischen den Kanälen z.B. die Verstärkung
	wechselt. Ansonsten werden die Parameter von CH0 für alle Kanäle ünernommen (S.1326). Das ist die Erklärung.
	Achtung, wenn ANACH = 1, dann 
	
*/




	REG_ADC_CGR = REG_ADC_CGR		| 0x0000F000u;		// Gain = 1 für CH5 (A2) -> Poti zur Spollwertvorgabe
														// Gain = 4 für CH6 (A1)
														// Gain = 4 für CH7 (A0)
														// Gain = 1	für restliche Analogeingänge 													

//	REG_ADC_CGR = REG_ADC_CGR		| 0xFFFFFFFFu;		// Gain = 4 für alle Kanäle
//	REG_ADC_CGR = REG_ADC_CGR		| 0x22222222u;		// Gain = 2 für alle Kanäle
//	REG_ADC_CGR = REG_ADC_CGR		| 0x00000000u;		// Gain = 1 für alle Kanäle






//	Kanäle Offset -> nicht benutzt
	
/*	Conversion Result der letzten Wandlung
		- Register: ADC_LCDR (ADC Last Converted Data Register), S.1341
	REG_ADC_LCDR	->   0x0000 | Kanalnummer 4Bit | DATA 12Bit |
*/	

/*	Start Conversion ADC -> nur bei Software_Trigger enable (S.1332)
	REG_ADC_CR = 0x00000002;
*/

/*	Conversion Result kanalbezogen (untere 12 Bit), S.1351
		- Register: ADC_CDRx (ADC Channel Data Register)
		- *CH_A0
		- *CH_A1
		- *CH_A2
*/

/*	Ablauf:
			
//	Start Conversion ADC (S.1332), nur bei Software_Trigger
	REG_ADC_CR = 0x00000002;

//	Auf ADC-End of conversion (EOC) testen (Hier wird nur auf CH7 fertig geprüft, weil der ADC fortlaufend, ab CH5 zu wandeln beginnt
//	und mit CH7 endet.
//	Alle 3 EOCs prüfen
//		if ( (REG_ADC_ISR && 0x0000080u) && (REG_ADC_ISR && 0x0000040u) && (REG_ADC_ISR && 0x0000020u) != 0) {
//	Nur das oberste EOC, vom Kanal 7, prüfen
		if (REG_ADC_ISR && 0x0000080u != 0) 
		{					
//	Zweipunkteform der Geraden
			U_A0 = (((3.2715 - 0.005) / (4095.0 - 0.005)) * (REG_ADC_CDR7 - 0.005)) + 0.005;
			U_A1 = (((3.2715 - 0.005) / (4095.0 - 0.005)) * (REG_ADC_CDR6 - 0.005)) + 0.005;
			U_A2 = (((3.2715 - 0.005) / (4095.0 - 0.005)) * (REG_ADC_CDR5 - 0.005)) + 0.005;
		}
*/
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx     Init ADC       xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx





//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx     INIT QDEC      xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
/*	Encodersignale des MEC22 werden mit Hilfe des QDEC dekodiert.
	Input (Encoder):
		- PHA -> PB25
		- PHB -> PB27
	Output:
		- REG_TC0_CV0
*/					


void	INIT_QDEC(void)
{

/*
1.	I/O Line or Peripheral Function Selection
		- PIO_PER (PIO Enable Register)
		- PIO_PDR (PIO Disable Register)
		- PIO_PSR (PIO Status Register), hier steht das Ergebnis
		- 0 -> pin is controlled by the corresponding on-chip peripheral selected in the PIO_ABSR (AB Select Register)
		- 1 -> pin is controlled by the PIO controller
		- nach Reset -> generally, the I/O lines are controlled by the PIO controller, i.e. PIO_PSR resets at 1

		- PB25 (TIOA0) und PB27 (TIOB0) werden als Peripheral Function selektiert
		- geht auch:
			PIOB -> PIO_PDR = PIOB -> PIO_PDR	|	0x0A000000u;
*/		
	REG_PIOB_PDR = REG_PIOB_PDR		|	0x0A000000u;

/*	
2.	Auswahl, ob Peripheral A oder B (S.858)
		- PIO_ABSR (PIO Peripheral A, B Select Register)
		- nach Reset -> 0x00000000 -> Peripheral A
		
		- PB25 (TIOA0) und PB27 (TIOB0) werden beide als Peripheral B selektiert	
		- geht auch:
	PIOB -> PIO_ABSR = PIOB -> PIO_ABSR |	0x0A000000u;
*/
	REG_PIOB_ABSR = REG_PIOB_ABSR	|	0x0A000000u;

/*
3.	Clock für Instance TC0 (Timer / Counter0) erlauben (S.859)
		- PMC_PCER0 und PMC_PCER1 (Power Management Controller Peripheral Clock Enable Register 0 und 1)
		- erlaubt den "peripheral clock" für den zugehörenden "peripheral identifier" (PID)

		- hier ist PID = 27 (Identifier für die TC0)
		- geht auch:
	PMC -> PMC_PCER0 = PMC -> PMC_PCER0	|	0x08000000u;
*/
	REG_PMC_PCER0 = REG_PMC_PCER0 	|	0x08000000u;

/*
4.	Channel Mode (S.881)
		- TC_CMR (Timer / Counter Channel Mode Register)
		- 0x00000505
		- Field TCCLKS of TC_CMRx must be configured to select XC0 input (i.e., 0x101)
		- Channel 0 and 1 must be configured in Capture mode
		- Capture mode is enabled
*/
	REG_TC0_CMR0 = REG_TC0_CMR0		|	0x00000005; 

/*
5.	Block Mode (S.882)
		- TC_BMR (Timer / Counter Block Mode Register)- MAXFILT (S.901)
		- 0x00001300, mit maximalem MAXFILT: 0x03F01300
		- POSEN bit =1, -> the motor axis position is processed on channel 0 (by means of the PHA, PHB edge detections)
		  and the number of motor revolutions are recorded on channel 1 if the IDX signal is provided on the TIOB1 input
*/
	REG_TC0_BMR = REG_TC0_BMR		|	0x03F11300u;		// 2 Flanken von PHA und PHB -> 4 MAXFILT = 65 und SWAP PHA mit PHB
															// -> entspricht einer Richtungsumkehr
//	REG_TC0_BMR = REG_TC0_BMR		|	0x00000300u;		// 2 Flanken von PHA -> 2
		
/*
6.	Channel Control (S.880), Starten des Counters
		- TC_CCRx (Timer / Counter Channel Control Register)
		- 0x00000003
		- A software trigger is performed: the counter is reset and the clock is started
		- nur einen Reset ausführen:
			- Counter setzt sich auf Wert: 4294967297 (2 hoch 32) und beim nächsten gültigen clock
			  steht er auf 0. Deshalb fehlt immer ein Count vom Encoder -> Korrektur vom Reset (+1) erforderlich
		- Softwaretrigger enable
*/
//	REG_TC0_CCR0 = REG_TC0_CCR0		|	0x00000004u;	// Software trigger is performed: the counter is reset and the clock is started.
	REG_TC0_CCR0 = REG_TC0_CCR0		|	0x00000001u;	// Enables the clock

	
/*
	Auslesen des ENC1-Wertes
	REG_TC0_CV0;
*/	
	
	
/*	Zusammenfassung, ohne Kommentare
	REG_PIOB_PDR = REG_PIOB_PDR		|	0x0A000000u;
	REG_PIOB_ABSR = REG_PIOB_ABSR	|	0x0A000000u;
	REG_PMC_PCER0 = REG_PMC_PCER0 	|	0x08000000u;
	REG_TC0_CMR0 = REG_TC0_CMR0		|	0x00000005; 
	REG_TC0_BMR = REG_TC0_BMR		|	0x00001300u;
	REG_TC0_CCR0 = REG_TC0_CCR0		|	0x00000005u;

	ENC1_P = REG_TC0_CV0;
*/
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx     INIT QDEC      xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
		




//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    MELODIE1    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void MELODIE1(void)
{
	int z;

	for (z=1; z<=300; z++)
	{
		SVPWM(0.0, 1.0, 0.0, 0, 0, 0);			// uh=1 vh=1 wh=1   ul=0 vl=0 wl=0	-> Motor_u an VBUS (Versorgung Renesas-Board)
		delay_us(900);
		SVPWM(0.0, 0.0, 0.0, 0, 0, 0);			// uh=1 vh=1 wh=1   ul=0 vl=0 wl=0	-> Motor an GND
		delay_us(1100);
	}

}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    MELODIE1    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx





//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    MELODIE2    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void MELODIE2(void)
{
	int z;

	for (z=1; z<=200; z++)
	{
		SVPWM(0.0, 1.0, 0.0, 0, 0, 0);			// uh=1 vh=1 wh=1   ul=0 vl=0 wl=0	-> Motor_u an VBUS (Versorgung Renesas-Board)
		delay_us(500);
		SVPWM(0.0, 0.0, 0.0, 0, 0, 0);			// uh=1 vh=1 wh=1   ul=0 vl=0 wl=0	-> Motor an GND
		delay_us(500);
	}
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx    MELODIE2    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx





//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx     PRINT_STROM    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void	PRINT_STROM(void)
{	
		printf("|STROM------------------------------------------------------------------------------------------------------------------------------|\r\n");
		printf("| STROM A0      : %15s| normiert      : %15s| MV - x_Achse  : %15s| Winkel x-Achse: %15s|\r\n",
		doubleToString(s1, STROM_A0), doubleToString(s2, S_A0), doubleToString(s3, X_KOMP), doubleToString(s4, WINKEL_STROM));
		printf("|       A1      : %15s|               : %15s|    - y_Achse  : %15s| Delta         : %15s|\r\n",
		doubleToString(s1, STROM_A1), doubleToString(s2, S_A1), doubleToString(s3, Y_KOMP), doubleToString(s4, WINKEL_STROM - WINKEL_STROM_ALT));
		printf("|       A2      : %15s|               : %15s|    - Länge    : %15s|\r\n",
		doubleToString(s1, STROM_A2), doubleToString(s2, S_A2), doubleToString(s3, MV_LAENGE));
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx     PRINT_STROM    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
		
		
		
		
		
//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx     PRINT_ADC      xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void	PRINT_ADC(void)
{	
//		printf("|ADC--------------------------------------------------------------------------------------------------------------------------------|\r\n");

		printf("| ADC_A0 (Geber_Stick): %15s| ADC_A1 (HS_ver): %15s| ADC_A2 (HS_hor): %15s|\r\n",
		doubleToString(s1, SS_ADC_A0_i), doubleToString(s2, SS_ADC_A1_i), doubleToString(s3, SS_ADC_A2_i));

/*
		printf("| ADC_A0 Geber  : %15s| ADC_A1 ver.   : %15s| ADC_A2 hor.   : %15s|\r\n",
		doubleToString(s1, ADC_A0_f), doubleToString(s2, ADC_A1_f), doubleToString(s3, ADC_A2_f));

		printf("| ADC_A0 Geber  : %15s| ADC_MWF_A1_ver: %15s| ADC_A2 hor.   : %15s|\r\n",
		doubleToString(s1, ADC_A0), doubleToString(s2, ADC_MWF_CDR6), doubleToString(s3, ADC_A2));
*/		

//		printf("| Reg. TC0_CV0  : %15d| Umdrehungen   : %15s| Winkel Rotor  : %15s| Winkel Motor  : %15s|\r\n",
//		REG_TC0_CV0, doubleToString(s2, ENC_UMD), doubleToString(s3, ENC_WKL), doubleToString(s4, ENC_WKL_MOT));
		
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx     PRINT_ADC      xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx





//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx     PRINT_TEST     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void	PRINT_TEST(void)
{
	printf("|TEST-------------------------------------------------------------------------------------------------------------------------------|\r\n");

	//		printf("| ADC_A0 (Geber_Stick): %15s| ADC_A1 (HS_ver): %15s| ADC_A2 (HS_hor): %15s|\r\n",
	//		doubleToString(s1, ADC_A0), doubleToString(s2, ADC_A1), doubleToString(s3, ADC_A2));


	printf("| ADC_A0_i      : %15d| ADC_A1_i      : %15d| ADC_A1_f      : %15s|\r\n",
	ADC_A0_i, ADC_A1_i, doubleToString(s1, ADC_A1_f)); //722  2928

//	printf("| ADC_A0 Geber  : %15s| ADC_MWF_A1_ver: %15s| ADC_A2 hor.   : %15s|\r\n",
//	doubleToString(s1, ADC_A0), doubleToString(s2, ADC_MWF_CDR6), doubleToString(s3, ADC_A2));
	

	//		printf("| Reg. TC0_CV0  : %15d| Umdrehungen   : %15s| Winkel Rotor  : %15s| Winkel Motor  : %15s|\r\n",
	//		REG_TC0_CV0, doubleToString(s2, ENC_UMD), doubleToString(s3, ENC_WKL), doubleToString(s4, ENC_WKL_MOT));
	
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx     PRINT_TEST     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx





//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx     PRINT_VERT     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//	VERTEILUNG von Werten, z.B. zur Untersuchung vom ADC-Verhalten
//	Gebildet werden MIN, MAX, 10 Verteilwerte


void	PRINT_VERT(void)
{
//	printf("|WERTE-VERTEILUNG-------------------------------------------------------------------------------------------------------------------|\r\n");


//	Snapshot vom ADC und dann Ruhe im weiteren!!!
	SS_ADC_A0_i = ADC_A0_i;									// Float, Poti vom Stick für Sollpositionsvorgabe
	SS_ADC_A1_i = ADC_A1_i;									// Float, Poti HapStik vertikal
	SS_ADC_A2_i = ADC_A2_i;	


/*
//	PRINT
//	Jeden ADC-Wert printen
	printf("| SS_ADC_A0_i   : %15d|\r\n",
	SS_ADC_A0_i);
*/



	SUM_AF_i_1 = SUM_AF_i_1 + SS_ADC_A0_i;						//aktueller ADC_A0-Wert wird dazu addiert
	af_count_i ++;


	

	
	
		PRINT_VERT_cnt_i = PRINT_VERT_cnt_i + 1;						// Zähler der Print-Ausgaben

		
//	Wertübergabe des Snapshot 
		VW_i		=  SS_ADC_A0_i;
	 
//	Initialisierung der Vergleichswerte:


//	Hier händisch den Mittelwert aus Swing VW_MIN und VW_MAX eintragen:
//		VW_MW	=   310;
//		VW_MW	=   727;
		VW_MW_i	=   1238;
	
	
		
		VW_1 =		VW_MW_i	- 4;
		VW_2 =		VW_MW_i	- 3;
		VW_3 =		VW_MW_i	- 2;
		VW_4 =		VW_MW_i	- 1;
	
		VW_5 =		VW_MW_i	+ 0;			// Mitte
	
		VW_6 =		VW_MW_i	+ 1;
		VW_7 =		VW_MW_i	+ 2;
		VW_8 =		VW_MW_i	+ 3;
		VW_9 =		VW_MW_i	+ 4;
		
	
	
// Prüfung: MIN
	if(VW_i < VW_MIN_i)
	{
		VW_MIN_i = VW_i;
	}			

// Prüfung: MAX
	if(VW_i > VW_MAX_i)
	{
		VW_MAX_i = VW_i;
	}			

// Prüfung: VW_1
	if(VW_i == VW_1)
	{
		VW_cnt_1 = VW_cnt_1 + 1;
	}

// Prüfung: VW_2
	if(VW_i == VW_2)
	{
		VW_cnt_2 = VW_cnt_2 + 1;
	}
	
// Prüfung: VW_3
	if(VW_i == VW_3)
	{
		VW_cnt_3 = VW_cnt_3 + 1;
	}
	
// Prüfung: VW_4
	if(VW_i == VW_4)
	{
		VW_cnt_4 = VW_cnt_4 + 1;
	}		

// Prüfung: VW_5
	if(VW_i == VW_5)
	{
		VW_cnt_5 = VW_cnt_5 + 1;
	}

// Prüfung: VW_6
	if(VW_i == VW_6)
	{
		VW_cnt_6 = VW_cnt_6 + 1;
	}
	
// Prüfung: VW_7
	if(VW_i == VW_7)
	{
		VW_cnt_7 = VW_cnt_7 + 1;
	}
	
// Prüfung: VW_8
	if(VW_i == VW_8)
	{
		VW_cnt_8 = VW_cnt_8 + 1;
	}
	
// Prüfung: VW_9
	if(VW_i == VW_9)
	{
		VW_cnt_9 = VW_cnt_9 + 1;
	}
	



	VW_SUM = (VW_cnt_1 + VW_cnt_2 + VW_cnt_3 + VW_cnt_4 + VW_cnt_5 + VW_cnt_6 + VW_cnt_7 + VW_cnt_8 + VW_cnt_9);
	



	if (af_count_i == 10)									// 20 ADC-Werte wurden addiert
	{
//		printf("|WERTE-VERTEILUNG-------------------------------------------------------------------------------------------------------------------|\r\n");

//		AF_A0_f = 0.5 + SUM_AF_i / 10.0;					// hier 0,5 zur Rundung dazu addieren
		
		AF_A0_f = SUM_AF_i_1 / 10.0;							// Addition von 0,5 kann aber auch entfallen, da der Absolutwert nicht so wichtig ist,
															// da ja alle Positionen beim Start des Sticks eingemessen werden

	
		AF_A0_i = AF_A0_f;									// Ganzzahlige Anteil wird übergeben

		af_count_i = 0;
		SUM_AF_i_1 = 0;
		//Hier noch Flag setzen, wenn INT alle 50us kommt, Regelschleife aber mit 1ms laufenb soll



/*						
//	Print
//	Alle Werte
	printf("| AF_A0_i       : %15d| PRINT_VERT_cnt: %15s| VW_SUM        : %15s| ADC_ISR_CNT   : %15s\r\n",
	AF_A0_i, doubleToString(s2,PRINT_VERT_cnt_i), doubleToString(s3,VW_SUM), doubleToString(s4,ADC_ISR_CNT));
	
	printf("| VW_MIN        : %15s| VW_MAX        : %15s| VW_1          : %15s| VW_2         : %15s\r\n",
	doubleToString(s1, VW_MIN_i), doubleToString(s2, VW_MAX_i), doubleToString(s3, VW_cnt_1), doubleToString(s4, VW_cnt_2));
	
	printf("| VW_3          : %15s| VW_4          : %15s| VW_5          : %15s| VW_6         : %15s\r\n",
	doubleToString(s1, VW_cnt_3), doubleToString(s2, VW_cnt_4), doubleToString(s3, VW_cnt_5), doubleToString(s4, VW_cnt_6));

	printf("| VW_7          : %15s| VW_8          : %15s| VW_9          : %15s\r\n",
	doubleToString(s1, VW_cnt_7), doubleToString(s2, VW_cnt_8), doubleToString(s3, VW_cnt_9));

*/

//	Nur den Average-Filter-Wert von ADC-Kanal A0 printen
//	Print
	printf(" ->  %10d\r\n", AF_A0_i);
	
	
	

//	printf("|-----------------------------------------------------------------------------------------------------------------------------------|\r\n");
	
		VW_MIN_i =  1000000;
		VW_MAX_i = -1000000;
		VW_cnt_1 = 0.0;
		VW_cnt_2 = 0.0;
		VW_cnt_3 = 0.0;
		VW_cnt_4 = 0.0;
		VW_cnt_5 = 0.0;
		VW_cnt_6 = 0.0;
		VW_cnt_7 = 0.0;
		VW_cnt_8 = 0.0;														
		VW_cnt_9 = 0.0;




	} 		
	
	
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx     PRINT_VERT     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx





//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx     PRINT_STICK    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void	PRINT_STICK(void)
{
	printf("|STICK------------------------------------------------------------------------------------------------------------------------------|\r\n");


	printf("| POT_U_ver_MIN1: %15s| POT_U_ver_NULL1: %15s| POT_U_ver_MAX1: %15s| POT_PF_ver    : %15s|\r\n",
	doubleToString(s1, POT_U_ver_MIN1), doubleToString(s2, POT_U_ver_NULL1), doubleToString(s3, POT_U_ver_MAX1), doubleToString(s4, POT_PF_ver));

	printf("| POT_ver_WKL   : %15s|\r\n",
	doubleToString(s1, POT_ver_WKL));



			

}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx     PRINT_STICK    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx





//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx  PRINT_CLARKE_PARK xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void	PRINT_CLARKE_PARK(void)
{	
		printf("|CLARKE_PARK------------------------------------------------------------------------------------------------------------------------|\r\n");
		printf("| alpha         : %15s| Dcp           : %15s|\r\n",
		doubleToString(s1, alpha), doubleToString(s2, Dcp));
		printf("| beta          : %15s| Qcp           : %15s|\r\n",
		doubleToString(s1, beta), doubleToString(s2, Qcp));
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx  PRINT_CLARKE_PARK xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
		
		
		
		

//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx  PRINT_INV_CLARKE_PARK  xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void	PRINT_INV_CLARKE_PARK(void)
{	
		printf("|INV_CLARKE_PARK--------------------------------------------------------------------------------------------------------------------|\r\n");
		printf("| X1            : %15s| Dpi           : %15s|\r\n",
		doubleToString(s1, X1), doubleToString(s2, Dpi));
		printf("| Y1            : %15s| Qpi           : %15s|\r\n",
		doubleToString(s1, Y1), doubleToString(s2, Qpi));
		printf("| Z1            : %15s|\r\n",
		doubleToString(s1, Z1), doubleToString(s2, Qpi));
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx  PRINT_INV_CLARKE_PARK  xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
		
			
	
	
	
//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx  PRINT_ENCODER     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void	PRINT_ENCODER(void)
{
		printf("|ENCODER----------------------------------------------------------------------------------------------------------------------------|\r\n");
		printf("| Reg. TC0_CV0  : %15d| Umdrehungen   : %15s| Winkel Rotor  : %15s| Winkel Motor  : %15s|\r\n",
		REG_TC0_CV0, doubleToString(s2, ENC_UMD), doubleToString(s3, ENC_WKL), doubleToString(s4, ENC_WKL_MOT));
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx  PRINT_ENCODER     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	
	
	
	
	
//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx      SVPWM       xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void	PRINT_SVPWM(void)
{	
		printf("|SVPWM------------------------------------------------------------------------------------------------------------------------------|\r\n");
		printf("| PWMu1         : %15s|\r\n",
		doubleToString(s1, PWMu1));
		printf("| PWMv1         : %15s|\r\n",
		doubleToString(s1, PWMv1));
		printf("| PWMw1         : %15s|\r\n",
		doubleToString(s1, PWMw1));
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx      SVPWM       xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
		
	



//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx  PRINT_ENCODER     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void	PRINT_REGLER(void)
{
		printf("|REGLER-----------------------------------------------------------------------------------------------------------------------------|\r\n");
		printf("| Df Führungsgr.: %15s| Dcp Rückführ. : %15s| De Regeldiff. : %15s| Dpi Reg.ausg. : %15s|\r\n",
		doubleToString(s1, Df), doubleToString(s2, Dcp), doubleToString(s3, De), doubleToString(s4, Dpi));
		printf("| Qf Führungsgr.: %15s| Qcp Rückführ. : %15s| Qe Regeldiff. : %15s| Qpi Reg.ausg. : %15s|\r\n",
		doubleToString(s1, Qf), doubleToString(s2, Qcp), doubleToString(s3, Qe), doubleToString(s4, Qpi));		
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx  PRINT_ENCODER     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	
	
	
//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx  PRINT_START       xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void	PRINT_START(void)
{
		printf("|HAPSTIK 02.10.2016     Programm auf Poti umstellen xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx|\r\n\n");
}
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx  PRINT_START       xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	
	
	
//	ENDE ************************************************     FUNKTIONEN     *************************************************










//	**************************************************************************************************************************
//	ANFANG *******************************************          MAIN           ***********************************************
//	**************************************************************************************************************************
int main(void)
{



//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx     Init SAM system     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx	
	sysclk_init();
	board_init();
	configure_console();
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx     Init SAM system     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	INIT_TC2();
	INIT_PWM();
	INIT_GPIO();
	INIT_ADC();
	INIT_QDEC();



//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx     Interrupt      xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
/*
	Wir wollen das End of Conversion des CH7 vom ADC (der 3. gewandelte Kanal für die Stromwerte) interruptfähig machen
	und in einer Interrupt Service Routine (ISR) alle weiteren Berechnungen durchführen.
		-  n	* 100us -> ISR
		- (n+1)	* 100us -> ISR
		- ...			
	- Instance ADC:	37
	- ADC Interrupt erlauben (S.1342)
		- Register: ADC_IER (ADC Interrupt Enable Register)
		- EOC7 erlauben (CH7), der letzte Kanal, der gewandelt wurde 
*/
	REG_ADC_IER = REG_ADC_IER | 0x00000080u;	



//	Aus dem main-init-Code zur korrekten De-/Aktivierung von Interrupts.
//	Hier aus dem Beispielprojekt "TC Capture Waveform" vom ASF Framework:

	NVIC_DisableIRQ(ADC_IRQn);
	NVIC_ClearPendingIRQ(ADC_IRQn);
	NVIC_SetPriority(ADC_IRQn, 0);
//	Enable ADC interrupt, schreibt das Register ISER im NVIC (Nested Vector Interrupt Controller)
	NVIC_EnableIRQ(ADC_IRQn);
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx     Interrupt      xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx



/*  Alle Debug-Printfunktionen:
	PRINT_STROM();	
	PRINT_CLARKE_PARK();
	PRINT_INV_CLARKE_PARK();	
	PRINT_ENCODER();	
	PRINT_SVPWM();
	PRINT_REGLER();
	PRINT_ADC();
	PRINT_STICK();
*/		


//	ANFANG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx   START                 xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	PRINT_START();


 

//	--------------------------------------------------------------------------------------- Kalibrierung der Winkel zueinander
//	Stick muss sich leicht drehen lassen

//	------------------------------------------------------------------------------------------------ Poti auf Winkel einlernen         
//	Analogwerte an 3 Stickpositionen messen und speichern

//	--------------------------------------------------------------------------------------------------- Stick in Position NULL
//	Leistungsfaktoren (0...1) MOTOR1, MOTOR2
	LF1 = 0.4;
	LF2 = 0.4;

	TD1 = 10;											// Zeitverzögerung1
	TD2 = 150;											// Zeitverzögerung2
	
	
	
//	Zunächst einmal den Winkeloffset für senkrechte Nullstellung des Sticks ermitteln
//	z.B. 13° -> WKL_OFF_1 = -13
//	DWE1 = -13 * 7;
	DWE1 = WKL_OFF_1 * 7;
	DWE2 = WKL_OFF_2 * 7;
	
	
		
//	Winkelanteile für Winkeloffset mit Berücksichtigung des Leistungsfaktors berechnen
	X1 = LF1 * cos(DWE1*WK1);
	Y1 = LF1 * cos(DWE1*WK1-WK2);
	Z1 = LF1 * cos(DWE1*WK1-WK3);
	
	X2 = LF2 * cos(DWE2*WK1);
	Y2 = LF2 * cos(DWE2*WK1-WK2);
	Z2 = LF2 * cos(DWE2*WK1-WK3);




	
//	Raumvektorausgabe -> Stick steht in Nullstellung	
	SVPWM(X1, Y1, Z1, X2, Y2, Z2);

//	Stick beruhigen lassen
	delay_ms(1000);

//	Raumvektorausgabe -> Stick steht in Nullstellung	
	SVPWM(0,0,0,0,0,0);





/*
//MOTOR1 Kalibrierlauf
//	------------------------------------------------------------------------------------- Stick in Position MAX in a Schritten
//	RUNTER
//	Schleifendurchläufe 33 (von 0° nach +33°)


	DWE_a1 = 7 * WKL_OFF_1;						// Offset des Sticks (installationsspezifisch)
	LF1 = 0.5;									// Leistungsfaktor (0...1)
	for (a=1; a<=33; a++) 
	{	if (a == 33)							// wenn Stick in Zielposition dann max. Power zum "Festhalten"
		{	LF1 = 1.0;
		}
		DWE1 = DWE_a1 + 7 * a * 1;				// Winkel pro Step = 1°  -> 10 * 1 = 1°
		X1 = LF1 * cos(DWE1*WK1);
		Y1 = LF1 * cos(DWE1*WK1-WK2);
		Z1 = LF1 * cos(DWE1*WK1-WK3);
		SVPWM(X1, Y1, Z1, 0, 0, 0);
		delay_ms(TD1);
	} // for (a=1; a<=33; a++) 
	DWE_a1 = DWE1;								// DWE merken
	delay_ms(TD2);								// Stick beruhigen lassen

	POT_U_ver_MAX1 = AF_A0_i;					// Poti1 lesen


//	------------------------------------------------------------------------------------- Stick in Position MIN in a Schritten
//	HOCH
//	Schleifendurchläufe 66 (von +33° nach -33°)

	LF1 = 0.5;									// Leistungsfaktor (0...1)
	for (a=1; a<=66; a++) 
	{	if (a == 66)							// wenn Stick in Zielposition dann max. Power zum "Festhalten"
		{	LF1 = 1.0;
		}
		DWE1 = DWE_a1 + 7 * a * -1;				//Winkel pro Step = 1°  -> 10 * 1 = 1°
		X1 = LF1 * cos(DWE1*WK1);
		Y1 = LF1 * cos(DWE1*WK1-WK2);
		Z1 = LF1 * cos(DWE1*WK1-WK3);
		SVPWM(X1, Y1, Z1, 0, 0, 0);
		delay_ms(TD1);
	} // for (a=1; a<=66; a++) 
	DWE_a1 = DWE1;
	delay_ms(TD2);								// Stick beruhigen lassen

	POT_U_ver_MIN1 = AF_A0_i;					// Poti1 lesen



//	------------------------------------------------------------------------------------- Stick in Position NULL in a Schritten
//	MITTE
// Schleifendurchläufe 33 (von -33° nach 0°)

	LF1 = 0.5;									// Leistungsfaktor (0...1)
	for (a=1; a<=33; a++) 
	{	if (a == 33)							// wenn Stick in Zielposition dann max. Power zum "Festhalten"
		{	LF1 = 1.0;
		}
		DWE1 = DWE_a1 + 7 * a * 1;				//Winkel pro Step = 1°  -> 10 * 1 = 1°
		X1 = LF1 * cos(DWE1*WK1);
		Y1 = LF1 * cos(DWE1*WK1-WK2);
		Z1 = LF1 * cos(DWE1*WK1-WK3);
		SVPWM(X1, Y1, Z1, 0, 0, 0);
		delay_ms(TD1);
	} // for (a=1; a<=33; a++) 
	DWE_a1 = DWE1;
	delay_ms(TD2);								// Stick beruhigen lassen

	POT_U_ver_NULL1 = AF_A0_i;					// Poti1 lesen


//	PRINT
	printf("|MAX1   : %10s|MIN1   : %10s|NULL1  : %10s|\r\n",
	doubleToString(s1, POT_U_ver_MAX1), doubleToString(s2, POT_U_ver_MIN1), doubleToString(s3, POT_U_ver_NULL1));	










//	MOTOR2 Kalibrierlauf
//	------------------------------------------------------------------------------------- Stick in Position MAX in a Schritten
//	RECHTS
//	Schleifendurchläufe 33 (von 0° nach +33°)

	DWE_a2 = 7 * WKL_OFF_2;						// Offset des Sticks (installationsspezifisch)

	LF2 = 0.5;									// Leistungsfaktor (0...1)
	for (a=1; a<=33; a++)
	{	if (a == 33)							// wenn Stick in Zielposition dann max. Power zum "Festhalten"
		{	LF2 = 1.0;
		}
		DWE2 = DWE_a2 + 7 * a * 1;				// Winkel pro Step = 1°  -> 10 * 1 = 1°
		X2 = LF2 * cos(DWE2*WK1);
		Y2 = LF2 * cos(DWE2*WK1-WK2);
		Z2 = LF2 * cos(DWE2*WK1-WK3);
		SVPWM(X1, Y1, Z1, X2, Y2, Z2);
		delay_ms(TD1);
	}
	DWE_a2 = DWE2;								// DWE merken
	delay_ms(TD2);								// Stick beruhigen lassen

	POT_U_ver_MAX2 = AF_A1_i;					// Poti2 lesen


//	------------------------------------------------------------------------------------- Stick in Position MIN in a Schritten
//	Schleifendurchläufe 66 (von +33° nach -33°)

	LF1 = 0.5;									// Leistungsfaktor (0...1)
	for (a=1; a<=66; a++)
	{	if (a == 66)							// wenn Stick in Zielposition dann max. Power zum "Festhalten"
		{	LF2 = 1.0;
		}
		DWE2 = DWE_a2 + 7 * a * -1;				// Winkel pro Step = 1°  -> 10 * 1 = 1°
		X2 = LF2 * cos(DWE2*WK1);
		Y2 = LF2 * cos(DWE2*WK1-WK2);
		Z2 = LF2 * cos(DWE2*WK1-WK3);
		SVPWM(X1, Y1, Z1, X2, Y2, Z2);
		delay_ms(TD1);
	}
	DWE_a2 = DWE2;
	delay_ms(TD2);								// Stick beruhigen lassen

	POT_U_ver_MIN2 = AF_A1_i;					// Poti2 lesen


//	------------------------------------------------------------------------------------- Stick in Position NULL in a Schritten
// Schleifendurchläufe 33 (von -33° nach 0°)

	LF1 = 0.5;									// Leistungsfaktor (0...1)
	for (a=1; a<=33; a++)
	{	if (a == 33)							// wenn Stick in Zielposition dann max. Power zum "Festhalten"
		{	LF2 = 1.0;
		}
		DWE2 = DWE_a2 + 7 * a * 1;				// Winkel pro Step = 1°  -> 10 * 1 = 1°
		X2 = LF2 * cos(DWE2*WK1);
		Y2 = LF2 * cos(DWE2*WK1-WK2);
		Z2 = LF2 * cos(DWE2*WK1-WK3);
		SVPWM(X1, Y1, Z1, X2, Y2, Z2);
		delay_ms(TD1);
	}
	DWE_a2 = DWE2;
	delay_ms(TD2);								// Stick beruhigen lassen

	POT_U_ver_NULL2 = AF_A1_i;					// Poti2 lesen




//	PRINT
	printf("|MAX2   : %10s|MIN2   : %10s|NULL2  : %10s|\r\n\n\n",
	doubleToString(s1, POT_U_ver_MAX2), doubleToString(s2, POT_U_ver_MIN2), doubleToString(s3, POT_U_ver_NULL2));	


*/	
	



	
	



//	----------------------------------------------------------------------------------------------------------- Endlosschleife
	while (1)
	{
		
//	Für synchrone Vorgänge passend zum ADC-Interrupt -> svpwm_int wird in der Funktion ADC_ISR -> 1 gesetzt (1ms Zyklus)
		if (svpwm_int == 1)
		{
//	Flag zurück setzen
			svpwm_int = 0;		


	cnt_1ms_poll++;



/*
	if(iPT == 1)										// ProgrammTeil1
	{
		


	
	
//	1ms ----------------------------------------------------- REGLELUNG -------------------------------------------------------
//	---------------------------------------------------------------------------------------------------------------------------


//	Debug_Pin C.25= 1 -> C.25 = 0
	REG_PIOC_SODR	= REG_PIOC_SODR		|	0x02000000u; 		// Ausgang C25 =1 (DUE_Pin 5)	
	delay_us(10);
	REG_PIOC_CODR	= REG_PIOC_CODR		|	0x02000000u; 		// Ausgang C25 =0 (DUE_Pin 5)	







//	---------- TÄNZCHEN: g-SPIRALE
	TAE_1ms++;

//	Input Regler		Nullpunkt			Amplitude		 Wert für 33°						Winkel in Grad	
	mySetpoint2_1	=	2418		+		AM_1		*	(1641			*	sin(( (VZ_1) *	TW_1 +  90)			* WK1));

	mySetpoint2_2	=	2304		+		AM_1		*	(1641			*	sin(( (VZ_1) *	TW_1)				* WK1));

	TW_1 = TW_1 + SP_1;									// jede ms wird der Winkel um SP_1° größer -> 1000 x 0,36 = 360°/s		

	if (TAE_1ms % 50 == 0)								// jede Sekunde
	{	
		SP_1 = SP_1 + VZ_2 * 0.005;
		if (SP_1 == 0.5)
		{	
			VZ_2 = -VZ_2;
		}
		AM_1 = AM_1 + VZ_2 * 0.005;
	}
	if (TAE_1ms % 7000 == 0)							// jede Sekunde
	{
		VZ_2 = -VZ_2;
	}	
		
//	----------
*/





/*
//	---------- RECORDER
	TAE_1ms++;

	if (TAE_1ms % 1 == 0)									// alle 1ms -> 200 Werte/s werden aufgezeichnet
	{	
		iREC1[cnt_scan] = AF_A0_i;
		iREC2[cnt_scan] = AF_A1_i;
		cnt_scan++;
	}
	
	

//	PRINT
	if (TAE_1ms % 10000 == 0)								// nach 10s wird gedruckt und zum ProgrammTeil2 übergegangen 
	{	

*/
/*
		printf(" 1: %10d\r\n",		iREC1[0]); 
		printf(" 2: %10d\r\n",		iREC1[1]); 		
		printf(" 3: %10d\r\n",		iREC1[2]); 		
		printf(" 4: %10d\r\n",		iREC1[3]); 		
		printf(" 5: %10d\r\n\n",	iREC1[4]); 

		printf(" 1: %10d\r\n",		iREC2[0]); 
		printf(" 2: %10d\r\n",		iREC2[1]); 		
		printf(" 3: %10d\r\n",		iREC2[2]); 		
		printf(" 4: %10d\r\n",		iREC2[3]); 		
		printf(" 5: %10d\r\n\n",	iREC2[4]); 
*/		
		
/*
		iPT = 2;											// 2. ProgrammTeil
		TAE_1ms = 0;									
		cnt_scan = 0;
		mySetpoint2_1	= 2418;								// mit NULLpunkt starten
		mySetpoint2_2	= 2304;		
	}	
	
	
	} // if(iPT == 1)
//	----------




	if(iPT == 2)
	{
//	RECORDER abspielen
	TAE_1ms++;
	
	if (TAE_1ms % 1 == 0)									// alle 1ms einen neuen Wert aus dem Array holen
	{	
		mySetpoint2_1	= iREC1[cnt_scan];
		mySetpoint2_2	= iREC2[cnt_scan];		
		cnt_scan++;	
		if(cnt_scan >= 10000)
		{
			mySetpoint2_1	= 2418;							// nach 10000 Werten NULLpunkt ausgeben 
			mySetpoint2_2	= 2304;

			iPT = 3;										// 3. ProgrammTeil

		}
	}
	
*/	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	

	
	
	
	
	
//	Wenn man mal die Reglerschleife in seiner Zeit verändern möchte:	
//	REGLER laufen mit 1ms oder 2ms oder 10ms ...
//	cnt_1ms_poll++;
			
//	if (cnt_1ms_poll % 2 == 0)										// alle 2ms Regler aufrufen
//	{
	

	
	
	
//	Debug_Pin C.25= 1 -> C.25 = 0
//	REG_PIOC_SODR	= REG_PIOC_SODR		|	0x02000000u; 		// Ausgang C25 =1 (DUE_Pin 5)
//	delay_us(1000);
//	REG_PIOC_CODR	= REG_PIOC_CODR		|	0x02000000u; 		// Ausgang C25 =0 (DUE_Pin 5)
//	delay_ms(2);
	
	
	
	
//	1ms ----------------------------------------------------- CAPTURE WERT LESEN ----------------------------------------------

//	PRINT
//	Wert eines Empfängerkanals (DUE: C.28) lesen -> *** 5250 ... 10500 ... 15750 *** 
// 	ACHTUNG: Beim Anschluss des Empfängerausgangs auf den Pegel achten. Der DUE verträgt nur 3,3V! -> Spannungsteiler!
	 



// Lineare Interpolation, um 1ms Werte vom Master zu bekommen, der nur alle 20ms einen aktuellen Wert versendet
//	alle 20ms
		if (cnt_1ms_poll % 20 == 0)																// 20ms
		{
//	CH1 (TIOA7)
			CH1_WERT1_1_alt = CH1_WERT1_1;													// alten CH0-Wert retten
			CH1_WERT1_1 = REG_TC2_RA1;														// alle 20ms neuen CH0-Wert übernehmen
			CH1_DELTA = (CH1_WERT1_1 - CH1_WERT1_1_alt)/20;

			CH1_WERT1_1_li = CH1_WERT1_1_li - CH1_DELTA;									// weil gleich danach in "jede ms" wieder CH0_DELTA dazu addiert wird

//	CH2 (TIOA8)

			CH2_WERT1_1_alt = CH2_WERT1_1;													// alten CH0-Wert retten
			CH2_WERT1_1 = REG_TC2_RA2;														// alle 20ms neuen CH0-Wert übernehmen
			CH2_DELTA = (CH2_WERT1_1 - CH2_WERT1_1_alt)/20;

			CH2_WERT1_1_li = CH2_WERT1_1_li - CH2_DELTA;									// weil gleich danach in "jede ms" wieder CH0_DELTA dazu addiert wird
		}
	
	
	
					
//	jede ms																							
//	CH1 (TIOA7)
			CH1_WERT1_1_li = CH1_WERT1_1_li + CH1_DELTA;									// CH0-Wert ist alter CH0-Wert + Delta

																							
			CH1_WERT1_1_li_nor = ((CH1_WERT1_1_li / 6.6336 - 2418) * 2.6) + 2700;			// Normierung auf Laufwege HS
			mySetpoint1_1 = CH1_WERT1_1_li_nor;												//int wert Übergabe

//	CH2 (TIOA8)
			CH2_WERT1_1_li = CH2_WERT1_1_li + CH2_DELTA;									// CH0-Wert ist alter CH0-Wert + Delta

																							
			CH2_WERT1_1_li_nor = ((CH2_WERT1_1_li / 6.6336 - 2418) * 2.6) + 2600;			// Normierung auf Laufwege HS
			mySetpoint1_2 = CH2_WERT1_1_li_nor;												//int wert Übergabe









/*
//	Debug_Pin C.25= 1 -> C.25 = 0
	REG_PIOC_SODR	= REG_PIOC_SODR		|	0x02000000u; 		// Ausgang C25 =1 (DUE_Pin 5)
	delay_us(10);
	REG_PIOC_CODR	= REG_PIOC_CODR		|	0x02000000u; 		// Ausgang C25 =0 (DUE_Pin 5)
//	delay_ms(2);
*/

						
/*			printf(" 1Kanal Empfänger: %20s\r\n",
			doubleToString(s1, CH0_WERT1_1));

			printf(" 1Kanal Empfänger: %20s\r\n",
			doubleToString(s1, CH0_WERT2_1));
*/		

			

	
	
	
	
	
	
	
	
//	1ms ----------------------------------------------------- REGLELUNG -------------------------------------------------------
//	---------------------------------------------------------------------------------------------------------------------------

	
//	Geschwindigkweit MOTOR1 (in Steps/ms -> 1Step/ms = ca. 0,02°/ms -> 20°/s)
	POT_CNT2_1	= POT_CNT1_1;
	POT_CNT1_1	= AF_A0_i;
	POT_V_1		= POT_CNT1_1 - POT_CNT2_1;					// 1ms Intervall

//	Geschwindigkweit MOTOR2 (in Steps/ms -> 1Step/ms = ca. 0,02°/ms -> 20°/s)
	POT_CNT2_2	= POT_CNT1_2;
	POT_CNT1_2	= AF_A1_i;
	POT_V_2		= POT_CNT1_2 - POT_CNT2_2;					// 1ms Intervall
		
	


//	Reglerkaskade PID1 (Position) -> PID2 (Geschwindigkeit)

//	MOTOR1 -------------------------------------------------------------------------------------------------------------------

//	REGLER POSITION PID1_1
//	mySetpoint1_1	= 800 + 0.6 * SS_ADC_A2_i;			// Führungsgröße MOTOR1 durch Poti "Masterstick" festlegen
//	mySetpoint1_1	=  2418;							// Führungsgröße manuell festlegen: 2418 -> 0° (mech.)
	outMax1_1		=	1.0;							// Max Geschwindigkeit =  1,0 -> ca. 20°/s  oder 0,2 -> ca. 4°/s
	outMin1_1		=  -1.0;							// Min

	kp1_1			=   0.0015;							// ?
//	ki1_1			=   0.0;	
	kd1_1			=   0.0035;

//	kp1_1			=   0.0015;							// SUPER gedämpft
//	ki1_1			=   0.0;	
//	kd1_1			=   0.004;

	
//  REGLER GESCHWINDIGKEIT PID2_1
//	mySetpoint2_1	=  -0.2;							// Wenn nur der Geschwindigkeitsregler arbeitet
														// 1.0 -> ca. 20°/s
														// 0.2 -> ca. 4°/s													
	outMax2_1		=  1.0;								// Max Drehmoment MOTOR1
	outMin2_1		= -1.0;

//	kp2_1			= 0.0005;							// HAMMER
//	ki2_1			= 0.00125;	
//	kd2_1			= 0.0;



//	REGLER rechnen

//	REGLER POSITION
	myInput1_1 = AF_A0_i;								// Input: Positionsdaten:
	PID1_1();											//		-> Kanal0 ADC (Average-Filter, 20-fach Oversampling) 
//	mySetpoint2_1 = myOutput1_1;						// Ausgang ist Führungsgröße für REGLER GESCHWINDIGKEIT
	LF1 = myOutput1_1;									// Wenn nur POSITIONS-REGLER
	
//	REGLER GESCHWINDIGKEIT (REG_TC0_CV0)
//	myInput2_1 = POT_V_1;
//	PID2_1();
//	LF1 = myOutput2_1;


/*
//	PRINT
//	Poti lesen, erfolgt in der ADC-Interrupt Routine
			if (TAE_1ms % 500 == 0)			// 500 x 1ms = 500ms
			{
				printf("|POT_V_1: %10s|LF1    : %10s|mSp2_1 : %10s|mI1_1  : %10s|mO1_1  : %10s|     \r\n",
				doubleToString(s1, POT_V_1), doubleToString(s2, LF1), doubleToString(s3, mySetpoint2_1), doubleToString(s4, myInput1_1), doubleToString(s5, myOutput1_1)  );
			}
*/






//	Ausgabe an MOTOR1
//	Zum Verständnis:
//	Wenn der Leistungsfaktor	-> positiv, dann +90° (elektrisch) Vektor raus geben
//								-> negativ, dann -90° (elektrisch) Vektor raus geben
			if (LF1 >= 0)
			{	DWE1 = 7 * ((33 * (AF_A0_i - 2418) / 1677.0) + WKL_OFF_1) + 90;	// "-" bei [2]


//	Winkelanteile mit Berücksichtigung des Leistungsfaktors berechnen
				X1 = LF1 * cos(DWE1*WK1);
				Y1 = LF1 * cos(DWE1*WK1-WK2);
				Z1 = LF1 * cos(DWE1*WK1-WK3);
			}
			if (LF1 < 0)
			{
				DWE1 = 7 * ((33 * (AF_A0_i - 2418) / 1677.0) + WKL_OFF_1) - 90;	// "+" bei [2]						
//	Winkelanteile mit Berücksichtigung des Leistungsfaktors berechnen
				X1 = -LF1 * cos(DWE1*WK1);
				Y1 = -LF1 * cos(DWE1*WK1-WK2);
				Z1 = -LF1 * cos(DWE1*WK1-WK3);
			}

if (cnt_1ms_poll % 200 == 0)			// 500 x 1ms = 500ms
			{
	printf("| X1      : %15s| Y1      : %15s| Z1  : %15s\r\n",
		doubleToString(s1, X1), doubleToString(s2, Y1), doubleToString(s3, Z1));
	printf("| Sollwert      : %15s\r\n",
		doubleToString(s1, mySetpoint1_1));
		printf("\r\n");
	printf("| X2      : %15s| Y2      : %15s| Z2  : %15s\r\n",
	doubleToString(s1, X2), doubleToString(s2, Y2), doubleToString(s3, Z2));
	printf("| Sollwert      : %15s\r\n",
	doubleToString(s1, mySetpoint1_2));	
	printf("------------\r\n");
			}
	








//	MOTOR2 -------------------------------------------------------------------------------------------------------------------

//	REGLER POSITION PID1_2
//	mySetpoint1_2	= 800 + 0.6 * SS_ADC_A2_i;			// Führungsgröße MOTOR1 durch Poti "Masterstick" festlegen
//	mySetpoint1_2	=  2304;							// Führungsgröße manuell festlegen: 2304 -> 0° (mech.)
	outMax1_2		=	1.0;							// Max Geschwindigkeit =  1,0 -> ca. 20°/s  oder 0,2 -> ca. 4°/s
	outMin1_2		=  -1.0;							// Min

	kp1_2			=   0.0015;							// ?
//	ki1_2			=   0.0;	
	kd1_2			=   0.0035;



	
//  REGLER GESCHWINDIGKEIT PID2_2
//	mySetpoint2_2	=  -0.2;							// Wenn nur der Geschwindigkeitsregler arbeitet
														// 1.0 -> ca. 20°/s
														// 0.2 -> ca. 4°/s													
	outMax2_2		=  1.0;								// Max Drehmoment MOTOR1
	outMin2_2		= -1.0;

//	kp2_2			= 0.0005;							// HAMMER
//	ki2_2			= 0.00125;	
//	kd2_2			= 0.0;



//	REGLER rechnen

//	REGLER POSITION
	myInput1_2 = AF_A1_i;								// Input: Positionsdaten:
	PID1_2();											//		-> Kanal0 ADC (Average-Filter, 20-fach Oversampling) 
//	mySetpoint2_2 = myOutput1_2;						// Ausgang ist Führungsgröße für REGLER GESCHWINDIGKEIT
	LF2 = myOutput1_2;									// Wenn nur POSITIONS-REGLER
	
//	REGLER GESCHWINDIGKEIT
//	myInput2_2 = POT_V_2;
//	PID2_2();
//	LF2 = myOutput2_2;


/*
//	PRINT
//	Poti lesen, erfolgt in der ADC-Interrupt Routine
			if (TAE_1ms % 500 == 0)			// 500 x 1ms = 500ms
			{
				printf("|POT_V_2: %10s|LF2    : %10s|mSp2_2 : %10s|mI1_2  : %10s|mO1_2  : %10s|     \r\n\n",
				doubleToString(s1, POT_V_2), doubleToString(s2, LF2), doubleToString(s3, mySetpoint2_2), doubleToString(s4, myInput1_2), doubleToString(s5, myOutput1_2)  );						
			}
*/






//	Ausgabe an MOTOR2
//	Zum Verständnis:
//	Wenn der Leistungsfaktor	-> positiv (Stick links),	dann -90° (elektrisch) Vektor raus geben
//								-> negativ (Stick rechts),	dann +90° (elektrisch) Vektor raus geben
			if (LF2 >= 0)
			{
				DWE2 = 7 * ((33 * (AF_A1_i - 2304) / 1641.0) + WKL_OFF_2) + 90;	
//	Winkelanteile mit Berücksichtigung des Leistungsfaktors berechnen
				X2 = LF2 * cos(DWE2*WK1);
				Y2 = LF2 * cos(DWE2*WK1-WK2);
				Z2 = LF2 * cos(DWE2*WK1-WK3);
			}

			if (LF2 < 0)
			{
				DWE2 = 7 * ((33 * (AF_A1_i - 2304) / 1641.0) + WKL_OFF_2) - 90;					

//	Winkelanteile mit Berücksichtigung des Leistungsfaktors berechnen
				X2 = -LF2 * cos(DWE2*WK1);
				Y2 = -LF2 * cos(DWE2*WK1-WK2);
				Z2 = -LF2 * cos(DWE2*WK1-WK3);
			}
	

		
//	Gemeinsame Raumvektorausgabe ---------------------------------------------------------------------------------------------	
		SVPWM(X1, Y1, Z1, X2, Y2, Z2);
			
			
	

























//	}	// if (TAE_1ms % 1 == 0)

//	}	// if(iPT == 2)

	}	// if (svpwm_int == 1)

	}	// while (1)

	}	// main(void)
//	ENDE xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx          MAIN           xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx