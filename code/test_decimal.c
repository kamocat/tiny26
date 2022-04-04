#include "decimal.h"
#include <stdio.h>

int8_t ref_calc_prescaler(const struct mag * n, uint16_t * value){
    float tmp = 0.01*n->ones + 0.1*n->tens + n->hunds;
    int ten_ex = n->exponent;
    int two_ex = 0;
    for(; ten_ex<0; ++ten_ex){
        tmp /= 10;
    }
    for(;ten_ex > 0; --ten_ex){
        tmp *= 10;
    }
    for(;tmp>1024;tmp/=2){
        two_ex++;
    }
    for(;tmp<512;tmp*=2){
        two_ex--;
    }
    *value = tmp;
    return two_ex;
}

void test_prescaler(struct mag *a){
    uint16_t v;
    int8_t p;
    p = ref_calc_prescaler(a, &v);
    printf("%d.%d%dE%d = %d*2^%d",a->hunds,a->tens,a->ones,a->exponent,v,p);
    p = calc_prescaler(a, &v);
    printf(" vs %d*2^%d\r\n",v,p);
}

int main( int argc, char ** argv){
#if 0
    struct mag a [] = {
        {.hunds=1,.tens=0,.ones=0,.exponent=0},
        {.hunds=0,.tens=1,.ones=0,.exponent=1},
        {.hunds=0,.tens=0,.ones=1,.exponent=2},
    };
    for(int i = 0; i < sizeof(a)/sizeof(struct mag); ++i){
        test_prescaler(a+i);
    }
#endif
    struct mag b = {.hunds=1,.tens=0,.ones=0,.exponent=-3};
    for(int i = -3; i< 4; ++i){
        b.exponent = i;
        test_prescaler(&b);
    }
    return 0;
}
        

