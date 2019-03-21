#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 1000000UL // 1MHz
#include <util/delay.h>

/* Global Variables */
#define ADDRESS_MASK 0xFE	// lsb is R/W
#define I2C_ADDRESS (0xCA & ADDRESS_MASK)	// Already shifted

volatile enum i2c_state{
	idle,
	rx_address,
	rx_command,
	ack_command,	// Receive a command next
	tx_data,
	ack_data	// Send data next
}i2c_state;

int8_t farenheight;
volatile uint8_t blink_period = 50;

/* Interrupts for I2C */
ISR( USI_START_vect ){
	PORTB &= ~1;	// Set pin B0 low
	i2c_state = rx_address;	// Next we're receiving an address
	while( PINA & 4 );	// Wait for SCL input to fall
	USIDR = 0xFF;	// Clear the send buffer
	USISR = 1<<USISIF | 1<<USIOIF;	// Release the bus and clear the timer
	PORTB |= 1;	// Set pin B0 high again
}

ISR( USI_OVF_vect ){
	PORTB &= ~1;	// Set pin B0 low
	switch( i2c_state ){
		case rx_address:
			// Compare to our address
			if( (USIDR & ADDRESS_MASK) == I2C_ADDRESS ){
				USIDR = 0;	// Send ACK
				// 1 means read, 0 means write
				i2c_state = USIBR&1 ? ack_data: ack_command;
				USISR = 14;	// Set the clock to overflow after the ACK
			} else {
				// It's not our address
				i2c_state = idle;
				USIDR = 0xFF;	// Don't send data
			}
			break;
		case ack_data:
			if( USIDR & 1 ){
				// Master NACK'd. Release data line.
				USIDR = 0xFF;
				i2c_state = idle;
			} else {
				// Master ACK'd. Send more data.
				USIDR = farenheight;
				i2c_state = tx_data;
			}
			break;
		case ack_command:
			/* Get ready to receive data */
			USIDR = 0xFF;
			i2c_state = rx_command;
			break;
		case tx_data:
			/* Finished sending data. Wait for an ACK */
			USIDR = 0xFF;
			USISR = 14;	// Set to overflow after ack
			i2c_state = ack_data;
			break;
		case rx_command:
			/* We received a command. Always ACK. */
			OCR1C = USIDR;	// Copy data to PWM period
			USIDR = 0;	// Send ACK
			USISR = 14;
			i2c_state = ack_command;
			break;
		default:
		case idle:
			USIDR = 0xFF;	// Clear the send buffer
			break;

	}
	USISR |= 1<<USIOIF;	// Release the bus
	PORTB |= 1;	// Set pin B0 high again
}


/* FUNCTIONS */

void init_i2c( void ){
	/* Set up USI for i2c slave:
	 * Enable start-bit interrupt
	 * Hold line low on start condition
	 * Hold line low on timer overflow
	 * Clock output on rising edge
	 */
	USICR = 1<<USISIE | 1<<USIOIE | 1<<USIWM1 | 1<<USIWM0 |
		1<<USICS1 | 0<<USICS0;
	USISR = 1<<USISIF | 1<<USIOIF;	// Clear interrupts
	USIDR = 0xFF;	// Clear buffer

	// Enable outputs for SCL and SDA
	PORTA |= 1<<0 | 1<<2;
	DDRA |= 1<<0 | 1<<2;
	USIPP = 1;	// Use PORTA, not PORTB
	PORTB |= 1;	// Set bit high for diagnostics

	i2c_state = idle;

	sei();	// Enable interrupts
}

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

int main( void ) {
	
	/* PORT SETUP */
	DDRA = 0x05;	// SCL and SDA on PORTA are outputs
	DIDR0 = 0xF0;	// disable digital input buffers on pA5 through pA7
	DDRB = 0x7F;	// set all PORTB as outputs, except RESET.

	/* Set up PWM to show we're receiving commands */
	TCCR1B = 1;	// Prescaler of 1
	TCCR1C = 1<<COM1D0;	// Toggle output on compare match
	TCCR1D = 0;	// Normal mode, use OCR1C as TOP
	OCR1D = 0;	// Always toggle
	OCR1C = 50;	// Good starting point

	init_i2c();
	
	
	while(1) {
		farenheight = adc_sample( 6 );	// Pin A7
		_delay_ms( 500 );
		PORTB ^= 1<<4;	// Toggle pin 4 on PORTB
		
	}

	return 0;
}
