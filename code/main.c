#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "uart.h"

#define F_CPU 1000000UL // 1MHz
#include <util/delay.h>


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

/* This function converts a number into a string
 * and sends it over UART.
 */
const uint16_t decimal[] = {10000, 1000, 100, 10};
 void UART_Send_Num( uint16_t val ){
	char c = '0';
	for( uint8_t i = 0; i < 4; ++i ){
		while( val >= decimal[i] ){
			val -= decimal[i];
			++c;
		}
		UART_Send_Byte(c);
		c = '0';
	}
	c += val;
	UART_Send_Byte(c);
	UART_Send_Byte('\r');
	UART_Send_Byte('\n');
}

ISR( BADISR_vect ) {
	//catch anything we accidentally enabled
}

int main( void ) {
	uint16_t volts;
	
	/* Change clock cal to 7.3MHz */
	OSCCAL = 23;

	/* Change clock speed to 1MHz */
	CLKPR = 1<<CLKPCE;
	CLKPR = 2;	// 1Mhz
	
	/* PORT SETUP */
	DDRA = 0x05;	// SCL and SDA on PORTA are outputs
	DIDR0 = 0xF0;	// disable digital input buffers on pA5 through pA7
	DDRB = 0x7F;	// set all PORTB as outputs, except RESET.

	
	
	while(1) {
		volts = 0;
		for( int i = 0; i < 64; ++i ){
			volts += adc_sample10( 5 ); // pA6
		}
		UART_Send_Num( volts );
		_delay_ms(100);

	}

	return 0;
}
