#include "decimal.h"

void inc_mag(struct mag * n, int8_t speed){
    switch(speed){
        case 1:
            n->ones++;
            if(n->ones < 10)
                break;
            /* Else falls through */
        case 2:
            n->ones = 0;
            n->tens++;
            if(n->tens< 10)
                break;
            /* Else falls through */
        case 3:
            n->tens = 0;
            n->hunds++;
            if(n->hunds >= 10){
                n->hunds = 1;
                n->exponent++;
                if(n->exponent > 0){
                    n->exponent = 0;
                    n->hunds = 9;
                    n->tens = 9;
                    n->ones = 9;
                }
            }
            break;
        case -1:
            n->ones--;
            if(n->ones < 10)
                break;
            n->ones = 9;
            /* Else falls through */
        case -2:
            n->tens--;
            if(n->tens< 10)
                break;
            n->tens = 9;
            /* Else falls through */
        case -3:
            n->hunds--;
            if(n->hunds == 0){
                if(n->tens == 0){
                    n->hunds = 9;
                } else {
                    n->hunds = n->tens;
                    n->tens = n->ones;
                }
                n->exponent--;
                if(n->exponent < -7){
                    n->exponent = -7;
                    n->hunds = 1;
                    n->tens = 0;
                    n->ones = 0;
                }
            }
            break;
        default:
            break;
    }
}


int8_t calc_prescaler(const struct mag * n, uint16_t * occr){
    uint16_t x;
    // Convert base-10 exponent to base-2 exponent
    // Approximating 10^3 with 2^10
    int8_t exponent = n->exponent;
    int8_t p = 0;
    p = exponent/3*10;
    exponent %= 3;
    if(exponent < 0){
        p -= 10;
        exponent += 3;
    }
    switch(exponent){
        case 1:
            // I divided each multiplier by 4 to prevent overflow
            x = n->ones*25 + n->tens*250 + n->hunds*2500;
            p -=8;
            break;
        case 0:
            // I divided each multiplier by 2 because it was cool
            x = n->ones*5 + n->tens*50 + n->hunds*500;
            p -=9;
            break;
        case 2:
            x = n->ones + n->tens*10 + n->hunds*100;
            break;
        default:
            x = 1;
            break;
    }
    // Reduce to only 10 bits wide
    while(x > 1023){
        x = x/2;
        ++p;
    }
    while(x < 512){
        x *= 2;
        --p;
    }
    *occr = x;
    return p;
}
