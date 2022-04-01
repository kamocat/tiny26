#include <stdint.h>

/* A structure to hold each digit
 */
struct mag{
    unsigned int hunds:4;
    unsigned int tens:4;
    unsigned int ones:4;
    int8_t exponent; // Had issues with specifying the field width
};

void inc_mag(struct mag * n, int8_t speed);

int8_t calc_prescaler(const struct mag * n, uint16_t * occr);
