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
    TCCR1B = 1;     // Start PWM clock at full speed
    TCCR1C = 0x09;  // Enable PWM on D
    TCCR1D = 0;     // Fast PWM
	
    init_7seg();
    uint8_t line = 0;
    uint8_t enc = 0;
    int16_t num[2];
    num[0] = 100;
    num[1] = 200;
	while(1) {
      enc <<= 4;
      enc |= PINA & 7; // Only look at pA0-2
      if(enc == 0x45)
          --num[line];
      if(enc == 0x54)
          ++num[line];
      if(enc == 0x73)
          line ^= 1;
      write_line(num[0], 3, 0);
      write_line(num[1], 3, 1);
      TC1H = num[0]>>8;
      OCR1D = num[0];
      TC1H = num[1]>>8;
      OCR1C = num[1];

	}

	return 0;
}
