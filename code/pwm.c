#include "decimal.h" // for struct mag
#include <avr/io.h>


void init_pwm(void){
    TCCR1B = 1;     // Start PWM clock at full speed
    TCCR1C = 0x09;  // Enable PWM on D
    TCCR1D = 0;     // Fast PWM
}

void update_pwm(const struct mag * pulse, const struct mag * period){
    const int8_t poffset = 21; //Setting the prescaler to 1 puts us at 1Mhz
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
        v1 = 1023;
    //If the pulse is less than half the period, reduce pulse resolution
    while(p1 < p2){
        v1 /= 2;
        ++p1;
    }

    TCCR1B = 0x0F & p2; // Set prescaler
    TC1H = v2>>8;// Set period
    OCR1C = v2;
    TC1H = v1>>8;// Set pulse width
    OCR1D = v1;
}
