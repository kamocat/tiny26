#include <stdint.h>
#include <avr/io.h>


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
	uint16_t i = 13<<7;
	DDRA = 0x00;
	DDRB = 0x02;
	
	while(1) {
		for( int j = 0; j < my_log2( i ); ++j ) {
			_delay_ms(  0.02 );
		}
		PINB = 0x02; // toggle pin 2 of port B.
		--i;
	}

	return 0;
}