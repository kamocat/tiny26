#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "7seg.h"

#define F_CPU 1000000UL // 1MHz
#include <util/delay.h>

ISR( BADISR_vect ) {
	//catch anything we accidentally enabled
}

uint16_t ms_elapsed(void){ 
    return( TCNT0L | (TCNT0H << 8));
}

void ms_reset(void){
    TCNT0H = 0;
    TCNT0L = 0;
    TCCR0A = 0x80;  // 16-bit counter
    TCCR0B = 0x0D;  // Count milliseconds, clear prescaler
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
    int8_t speed = 0;
    struct mag pulse = {.hunds=1,.exponent=-4};
    struct mag period = {.hunds=1,.exponent=-3};
    ms_reset();
	while(1) {
      uint16_t elapsed = ms_elapsed();
      if( elapsed < 80 )
          speed = 3;
      else if( elapsed < 150 )
          speed = 2;
      else
          speed = 1;

      enc <<= 4;
      enc |= PINA & 7; // Only look at pA0-2

      if(enc == 0x45){
          inc_mag(line?&period:&pulse, -speed);
          ms_reset();
      } else if(enc == 0x54){
          inc_mag(line?&period:&pulse, speed);
          ms_reset();
      } else if(enc == 0x73){
          line ^= 1;
      }
      write_mag(&period, 0);
      write_mag(&pulse, 1);

      /* TODO: Resolve magnitude with prescaler
      TC1H = pulse>>8;
      OCR1D = pulse;
      TC1H = period>>8;
      OCR1C = period;
      */
	}

	return 0;
}
