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
    PORTA = 0x07;   // Enable pull-ups on pA0-2
	DIDR0 = 0xF0;	// disable digital input buffers on pA5 through pA7
	DDRB = 0x7F;	// set all PORTB as outputs, except RESET.
	
    init_7seg();
    int i = 0;

    uint8_t line = 0;
    uint8_t enc;
	while(1) {
      enc <<= 4;
      enc |= PINA & 7; // Only look at pA0-2
      if(enc == 0x45)
          --i;
      if(enc == 0x54)
          ++i;
      if(enc == 0x73)
          line = !line;
      write_line(i, 1, line);
	}

	return 0;
}
