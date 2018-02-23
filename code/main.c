#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 1000000UL // 1MHz
#include <util/delay.h>


/* PWM for BLUE ESC goes from 1.1ms to 1.9ms, centered at 1.5ms */
#define PWM_CENTER 384


/* FUNCTIONS */

/* This function does not require the ADC to be initialized beforehand.
 * It takes about 75us to run the first time, and 50uS every time after that.
 */
uint8_t adc_sample( uint8_t channel ) {
	channel = (channel & 0x1F) | 0x20; // sanitize the input
	if( channel != ADMUX ) { 		// if it's not already the right channel
		while( ADCSRA & 0x40 );	// wait for the current conversion to complete
		ADMUX = channel;			// set the right channel
	}
	ADCSRA = 0xC0;			// start the conversion!
	while( ADCSRA & 0x40 );	// wait for the conversion
	return ADCH;
}

/* This is the same as adc_sample, but it returns a 10-bit result instead.
 * Because of the higher resolution, it runs a at a slower clock speed
 * It takes about 225uS the first time, and 130uS every time after that.
 */
uint16_t adc_sample10( uint8_t channel ) {
	uint16_t result = 0;
	channel = (channel & 0x1F); // sanitize the input
	if( channel != ADMUX ) { 		// if it's not already the right channel
		while( ADCSRA & 0x40 );	// wait for the current conversion to complete
		ADMUX = channel;			// set the right channel
	}
	ADCSRA = 0xC3;			// start the conversion!
	while( ADCSRA & 0x40 );	// wait for the conversion
	result = ADCL;
	result |= ADCH<<8;
	return result;
}


ISR( BADISR_vect ) {
	//catch anything we accidentally enabled
}

#define DEBUG 1
int main( void ) {
	
	int16_t volts = 0;
	uint16_t pwm = PWM_CENTER;
	
	/* PORT SETUP */
	DDRA = 0x00;	// All of Port A is inputs
	DIDR0 = 0xF0;	// disable digital input buffers on pA5 through pA7
	DDRB = 0x7F;	// set all PORTB as outputs, except RESET.
	
	#if DEBUG == 2
	/* PWM SETUP */
	TCCR1A = 0x11; //PWM on OCR1B, with inverted and non-inverted output
	TCCR1B = 0x01; 	// prescaler of 1
	TCCR1D = 0x00; 	// Fast PWM
	PLLCSR = 0x02; 	// Enable fast PLL (64 Mhz)
	#elif DEBUG == 1
	
	/* PWM SETUP */
	TCCR1A = 0x82; 	//PWM on OCR1A, with non-inverted output
	TCCR1B = 0x03; 	// prescaler of 4
	TCCR1D = 0x00; 	// Fast PWM
	TC1H = 0x03;
	OCR1C = 0xFF;		// Set TOP to max value
	#endif
	
	while(1) {

		#if DEBUG == 1
		volts = adc_sample(0);
		pwm = PWM_CENTER + (128 - volts);
		TC1H = pwm>>8;
		OCR1A = pwm;
		#else
		/* Simple blinking light */
		PINB = 0x08;
		_delay_ms( 500 );
		#endif
		
	}

	return 0;
}