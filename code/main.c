#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 1000000UL // 1MHz
#include <util/delay.h>

#define PRETEND_INPUTS

/* GLOBAL VARIABLES */
uint16_t period1= 0x4000;
uint16_t period2 = 0x4000;
char period_state = 0;
uint8_t period_counter = 0;

#define FOFFSET 132	// This calibrates the frequency generator

uint8_t background_level = 0;
uint8_t pedal_level = 0;


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



/* This function approximates logarithmic decay using linear programming.
 * The idea is to have a reasonable model without intensive computation.
 * Floating-point math would be a bad idea here, but I even went to the extent
 * of avoiding the divide operator in favor of bit shifting.
 * The function will accept any value of x, but the model ends at 1664, and x
 * loses precision at values higher than 1152.
 */
uint16_t my_log2( uint16_t x ) {
	uint16_t result;
	uint8_t segment = x >> 7;
	switch( segment ) {
		case 0:
			result = 65535 - (x<<8);
		break;
		case 1:
			result = 49152 - (x<<7);
		break;
		case 2:
			result = 32768 - (x<<6);
		break;
		case 3:
			result = 20480 - (x<<5);
		break;
		case 4:
			result = 12288 - (x<<4);
		break;
		case 5:
			result = 7168 - (x<<3);
		break;
		case 6:
			result = 4096 - (x<<2);
		break;
		case 7:
			result = 2304 - (x<<1);
		break;
		case 8:
			result = 1280 - x;
		break;
		case 9:
			result = 704 - (x>>1);
		break;
		case 10:
			result = 384 - (x>>2);
		break;
		case 11:
			result = 208 - (x>>3);
		break;
		case 12:
			result = 112 - (x>>4);
		break;
		default:
			result = 8;
		break;
	}
	return result;
}

ISR( TIMER0_COMPA_vect ) {
	++period_counter;
	if( period_counter & 64 ){
		period_counter = 0;
		/* Toggle the output */
		period_state = !period_state;
		if( period_state ) {
			PORTB = 0;
			OCR1A = pedal_level;
			OCR0B = period1 >> 8;
			OCR0A = period1;
		} else {
			PORTB = 0x10;
			OCR1A = background_level;
			OCR0B = period2 >> 8;
			OCR0A = period2;
		}
	}
	
	/* Reset the timer */
	TCNT0H = 0;
	TCNT0L = 0;
}

ISR( BADISR_vect ) {
	//catch anything we accidentally enabled
}


int main( void ) {
	uint16_t fknob = 0;
	uint16_t dknob = 0;
	uint32_t total_period = 0;
	
	/* PORT SETUP */
	DDRA = 0x00;	// All of Port A is inputs
	DIDR0 = 0xC0;	// disable digital input buffers on pA6 and pA7
	DDRB = 0x7F;	// set all PORTB as outputs, except RESET.
	
	/* PWM SETUP */
	TCCR1A = 0x42; //PWM on OCR1A, with inverted and non-inverted output
	TCCR1B = 0x01; 	// prescaler of 1
	TCCR1D = 0x01; 	// Fast PWM
	PLLCSR = 0x02; 	// Enable fast PLL (64 Mhz)
	
	
	/* FREQUENCY SETUP */
	TCCR0A = 0x80; 	// 16 bit timer, normal mode
	TCCR0B = 0x01;	// precaler of 1x system clock
	TIMSK = 0x10;		// enable interrupt for OCR0A
	OCR0B = 10;
	OCR0A = 0;
	sei();
	
	while(1) {
		dknob = adc_sample10(6);
		fknob = adc_sample10(5) + FOFFSET;
		
		total_period = my_log2( fknob );
		period1 = (total_period * dknob) >> 10;
		if( period1 == 0 ) period1 = 1; // special case
		period2 = total_period - period1;
		
		#ifdef PRETEND_INPUTS
		background_level = 10;
		pedal_level = 90;
		#else
		background_level = adc_sample(0);
		pedal_level = adc_sample(1);
		#endif
		
		
	}

	return 0;
}