#include <stdio.h>
#include <string.h>
#include <math.h>

#define uint16 unsigned short
#define uint8 unsigned char

/* This function should return the result of (period * portion / 1024).
 * The point is to do that without using 32 bit integers, so it is more
 * effecient on an 8 bit microcontroller.
 * 
 * portion expects a value between 0 and 1023.
 * period expects a value between 0 and 65535.
 * Output should be between 0 and 65535.
 */
uint16 scale( uint16 period, uint16 portion ) {
	uint16 result;
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
uint16 my_log2( uint16 x ) {
	uint16 result;
	uint8 segment = x >> 7;
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


/* Right now we're testing the accuracy of our scaling algorithm.
 * I'm measuring standard deviation, but as I got a result of 2, I'm
 * trying to get some more statistics and figure out where it isn't right.
 */
int main( int argc, char **argv ) {
	int diff = 0;
	long long variance = 0;
	double std_dev;
	for( int i = 0; i < (1<<10); ++i ) {
		for( int j = 0; j < (1<<16); ++j ) {
			int compare = (i * j) >> 10;
			diff = compare - scale( j, i );
			variance += (diff * diff);
		}
	}
	std_dev = sqrt((double)variance) / (1<<26);
	
	printf("Total variance was %lld and standard deviation was %f.\r\n", variance, std_dev );
	
	return 0;
}