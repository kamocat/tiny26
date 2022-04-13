#include "decimal.h" // for struct mag
#include <util/delay.h>
#include <avr/io.h>


void init_pwm(void){
    PLLCSR = 0x02;  // Enable 64Mhz PLL
    _delay_us(100); // Wait for PLL to stabilize
    while(~PLLCSR & 1);
    PLLCSR |= 0x04; // Use 64Mhz clock as timing source
    TCCR1D = 0;     // Fast PWM
    DT1 = 0x00;     // No dead-time
}

void update_duty(uint8_t duty, const struct mag * period){
    const int8_t poffset = 27; //Setting the prescaler to 1 puts us at 64Mhz
    uint16_t v1, v2;
    int8_t p2 = poffset + calc_prescaler(period, &v2);
    //If the period is less than the minimum, correct it.
    while(p2 < 1){
        v2 /= 2;
        ++p2;
    }
    v1 = v2;
    if(v1 > (UINT16_MAX / 100) ){
        // Prevent overflow
        v1 /= 4;
        v1 *= duty;
        v1 /= 25;
    } else {
        v1 *= duty;
        v1 /= 100;
    }
        
    TCCR1B = 0x0F & p2; // Set prescaler
    TC1H = v2>>8;// Set period
    OCR1C = v2;
    TC1H = v1>>8;// Set pulse width
    OCR1D = v1;
    TCCR1C = 0x05;  // Enable PWM on D (also enable inverted output)
}

void update_pwm(const struct mag * pulse, const struct mag * period){
    const int8_t poffset = 27; //Setting the prescaler to 1 puts us at 64Mhz
    uint16_t v1, v2;
    int8_t p1 = poffset + calc_prescaler(pulse, &v1);
    int8_t p2 = poffset + calc_prescaler(period, &v2);

    //If the period is less than the minimum, correct it.
    while(p2 < 1){
        v2 /= 2;
        ++p2;
    }
    //If the pulse is longer than the period, trim it to max
    if(p1 > p2)
        v1 = v2 - 1;
    //If the pulse is less than half the period, reduce pulse resolution
    while(p1 < p2){
        v1 /= 2;
        ++p1;
    }
    // Pulses less than 2 clock cycles don't appear at all
    if(v1 < 2)
        v1 = 2;

    TCCR1B = 0x80 | (0x0F & p2); // Set prescaler. PWM is inverted, dead-time is x1
    TC1H = v2>>8;// Set period
    OCR1C = v2;
    TC1H = v1>>8;// Set pulse width
    OCR1D = v1;
    TCCR1C = 0x05;  // Enable PWM on D (also enable inverted output)
}
