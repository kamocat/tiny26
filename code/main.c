#include <stdint.h>
#include <avr/io.h>
#include "7seg.h"
#include "pwm.h"

#define F_CPU 1000000UL // 1MHz
#include <util/delay.h>

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
    /* Variable declaration */
    uint8_t line = 1;
    uint8_t enc = 0;
    int8_t speed = 0;
    struct mag period = {.hunds=1,.exponent=-3};
    uint8_t duty = 50; // Percent

    /* PORT SETUP */
    DDRA = 0x00;	// All of Port A is inputs
    PORTA = 0x07;   // Enable pull-ups on pA0-2
    DIDR0 = 0xF0;	// disable digital input buffers on pA5 through pA7
    DDRB = 0x7F;	// set all PORTB as outputs, except RESET.

    /* Peripheral Init */
    init_pwm();
    init_7seg();
    ms_reset();

    while(1) {
        uint16_t elapsed = ms_elapsed();
        if( elapsed < 50 )
            speed = 3;
        else if( elapsed < 150 )
            speed = 2;
        else
            speed = 1;

        enc <<= 4;
        enc |= PINA & 7; // Only look at pA0-2

        if(enc == 0x45){
            if(line){
              inc_mag(&period, -speed);
            } else if(duty > 0) {
                --duty;
            }
            ms_reset();
        } else if(enc == 0x54){
            if(line){
                inc_mag(&period, speed);
            } else if(duty < 100) {
                ++duty;
            }
            ms_reset();
        } else if(enc == 0x73){
            line ^= 1;
        }
        write_short(duty, 1);
        write_mag(&period, 1);
        update_duty(duty, &period);
    }
    return 0;
}
