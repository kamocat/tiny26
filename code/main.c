#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "7seg.h"

#define F_CPU 1000000UL // 1MHz
#include <util/delay.h>

ISR( BADISR_vect ) {
	//catch anything we accidentally enabled
}


int main( void ) {
	
	/* PORT SETUP */
	DDRA = 0x00;	// All of Port A is inputs
	DIDR0 = 0xF0;	// disable digital input buffers on pA5 through pA7
	DDRB = 0x7F;	// set all PORTB as outputs, except RESET.
	
    init_7seg();
    int i = 0;
	while(1) {
      write_line(i++, 1, 1);
	  _delay_ms(1000);
	}

	return 0;
}
