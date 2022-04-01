#include "7seg.h" // for struct mag
#include <avr/io.h>


void init_pwm(void){
    TCCR1B = 1;     // Start PWM clock at full speed
    TCCR1C = 0x09;  // Enable PWM on D
    TCCR1D = 0;     // Fast PWM
}


int8_t calc_prescaler(const struct mag * n, uint16_t * occr){
    uint16_t x;
    // Convert base-10 exponent to base-2 exponent
    // Approximating 10^3 with 2^10
    int8_t p = n->exponent/3*10;
    switch(n->exponent % 3){
        case 1:
            // I divided each multiplier by 4 to prevent overflow
            x = n->ones*25 + n->tens*250 + n->hunds*2500;
            p -=12;
            break;
        case 0:
            // I divided each multiplier by 2 because it was cool
            x = n->ones*5 + n->tens*50 + n->hunds*500;
            p -= 11;
            break;
        case 2:
        default:
            x = n->ones + n->tens*10 + n->hunds*100;
            p -= 10;
            break;
    }
    // Reduce to only 10 bits wide
    while(x > 1023){
        x = x/2;
        ++p;
    }
    *occr = x;
    return p;
}

void update_pwm(const struct mag * pulse, const struct mag * period){
    const int8_t poffset = 21; //Setting the prescaler to 1 puts us at 1Mhz
    uint16_t v1, v2;
    int8_t p1 = poffset + calc_prescaler(pulse, &v1);
    int8_t p2 = poffset + calc_prescaler(period, &v2);
    v1 = 300;

    //If the pulse is less than the minimum, correct it.
    while(p2 < 1){
        v2 /= 2;
        ++p2;
    }
    //If the pulse is longer than the period, trim it to max
    if(p1 > p2)
        v1 = 1023;
    //If the pulse is less than half the period, reduce pulse resolution
    while(p1 < p2){
        v1 /= 2;
        ++p1;
    }

    TCCR1B = p2; // Set prescaler
    TC1H = v2>>8;// Set period
    OCR1C = v2;
    TC1H = v1>>8;// Set pulse width
    OCR1D = v1;
}
