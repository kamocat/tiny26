#include <stdint.h>
#include <avr/io.h>

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


#define F_CPU 1000000UL // 1MHz
#include <util/delay.h>



/* This function should return the result of (period * portion / 1024).
 * The point is to do that without using 32 bit integers, so it is more
 * effecient on an 8 bit microcontroller.
 * 
 * portion expects a value between 0 and 1023.
 * period expects a value between 0 and 65535.
 * Output should be between 0 and 65535.
 */
uint16_t scale( uint16_t period, uint16_t portion ) {
	uint16_t result;
	portion &= 0x3F;	//trim off top
	if( period < (1<<6))
		result = (period * portion) >> 10;
	else if( period < (1<<8) )
		result = ((period>>2) * portion) >> 8;
	else if( period < (1<<10) )
		result = ((period>>4) * portion) >> 6;
	else if( period < (1<<12) )
		result = ((period>>6) * portion) >> 4;
	else if( period < (1<<14) )
		result = ((period>>8) * portion) >> 2;
	else
		result = (period>>10) * portion;
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


int main( void ) {
	DDRA = 0x00;	// All of Port A is inputs
	DIDR0 = 0xC0;	// disable digital input buffers on pA6 and pA7
	DDRB = 0x7F;	// set all PORTB as outputs, except RESET.
	
	TCCR1A = 0x53; //PWM on OCR1A, and OCR1B with inverted and non-inverted output
	TCCR1B = 0x01;	// prescaler of 1
	TCCR1D = 0x01;	// Fast PWM
	PLLCSR = 0x02;	// Enable fast PLL (64 Mhz)
	
	while(1) {
		_delay_ms(0.1);
		PORTB = 0x10;
		OCR1A = adc_sample(5);	// read the voltage off pA6
		PORTB = 0x00;
		_delay_ms(0.1);
		PORTB = 0x10;
		OCR1B = adc_sample(6); // read the voltage off pA7
		PORTB = 0x00;
	}

	return 0;
}