#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 16000000UL  // 16 MHz kellotaajuus

// LEDien ja liiketunnistimen pinnit
#define PIX_PIN  2
#define LED_R_PIN  0  
#define LED_G_PIN  1   
#define LED_B_PIN  2   


volatile uint16_t no_motion_timer = 0;  // Ajastin liikkeen puutteelle
volatile uint8_t motion_detected = 0;   // Liiketila

//LEDien ja liiketunnistimenw alustaminen
void init() {
	asm volatile (
	//Asetetaan LED-pinnien ulostuloiksi
	"ldi r16, (1 << %0) | (1 << %1) | (1 << %2) \n\t"
	"sts 0x0421, r16                       \n\t" // PORTB.DIRSET osoite on 0x0421

	//Sammutetaan LEDit
	"ldi r16, (1 << %0) | (1 << %1) | (1 << %2) \n\t"
	"sts 0x0426, r16                       \n\t" // PORTB.OUTCLR osoite on 0x0426

	//Liiketunnistimen pinni sisääntuloksi 
	"ldi r16, (1 << %3)                    \n\t"
	"sts 0x0462, r16                       \n\t" // PORTD.DIRCLR osoite on 0x0462
	:
	: "i" (LED_R_PIN), "i" (LED_G_PIN), "i" (LED_B_PIN), "i" (PIX_PIN)
	: "r16"
	);
}

// Ajastimen alustaminen
void init_timer() {
	TCA0.SINGLE.PER = 16000;  // Ajastimen jakso 
	TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV64_gc | TCA_SINGLE_ENABLE_bm;  // Käynnistä ajastin prescaler 64
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;  // Aktivoi keskeytys ylivuodosta
}

void init_adc() {
	ADC0.CTRLA = ADC_ENABLE_bm;            // Ota käyttöön ADC
	ADC0.CTRLB = ADC_PRESC_DIV16_gc;       	// Kellotaajuus 16 MHz / 16
	ADC0.MUXPOS = ADC_MUXPOS_AIN0_gc;      // Valitaan ADC0 (LIGHT_SENSOR_PIN)
	ADC0.CTRLC = ADC_REFSEL_VDDREF_gc;     // Viiteta jännite VDD
}

uint16_t read_light_sensor() {
	ADC0.COMMAND = ADC_STCONV_bm;          		// Käynnistä muunnos
	while (!(ADC0.INTFLAGS & ADC_RESRDY_bm));  	// Odota tulosta
	ADC0.INTFLAGS = ADC_RESRDY_bm;         		// Nollaa keskeytyslippu
	return ADC0.RES;                       		// Palauta ADC:n tulos
}

// LEDin värin asettaminen
void setLEDColor(uint8_t red, uint8_t green, uint8_t blue) 
{
	//punaisen kontrolli
	if (red)
		PORTB.OUTSET = (1 << LED_R_PIN);
	else
		PORTB.OUTCLR = (1 << LED_R_PIN);
	
	//vihreän kontrolli
	if (green)
		PORTB.OUTSET = (1 << LED_G_PIN);
	else 
		PORTB.OUTCLR = (1 << LED_G_PIN);

	//sinisen kontrolli
	if (blue)
		PORTB.OUTSET = (1 << LED_B_PIN);
	else 
		PORTB.OUTCLR = (1 << LED_B_PIN);
	
	//valot pois päältä
	if(!red && !blue && !green)
	{
		PORTB.OUTCLR = (1 << LED_R_PIN);
		PORTB.OUTCLR = (1 << LED_G_PIN);
		PORTB.OUTCLR = (1 << LED_B_PIN);
	}
}

// Ajastimen keskeytysrutiini
ISR(TCA0_OVF_vect) {
	if (!motion_detected)
		no_motion_timer++;   // Kasvatetaan ajastinta 
	TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;  // Nollataan keskeytyslippu
}


int main(void) {
	init();   			// Alustetaan LEDit ja liikkeentunnistin
	init_timer(); 		// Alustetaan ajastin
	init_adc();       	// Alustetaan ADC valosensorille
	sei();        		// Globaalin keskeytyksen mahdollistaminen

	while (1)
	{
		// Lue valosensorin arvo
		uint16_t current_light_level = read_light_sensor();
		if (current_light_level > 3500) 	// Jos ympärillä olevaa valoa on riittävästi suljetaan ledi.
			setLEDColor(0,0,0);  // LED pois		
		// Tarkista liiketunnistimen tila
		else if (PORTD.IN & (1 << PIX_PIN))
		{ 	// Jos liikettä havaitaan
			setLEDColor(1, 1, 1);  			// LED päälle
			motion_detected = 1;   			// Liike havaittu
			no_motion_timer = 0;   			// Nollataan ajastin
		} 
		else
		{
			if (no_motion_timer >= 27) 	// noin 8s ilman liikettä
				setLEDColor(0, 0, 0);  			// LED pois
			else
				motion_detected = 0;   			// Ei liikettä
		}
	}
}

