#include <stdint.h>

/* A structure to hold each digit
 */
struct mag{
    unsigned int hunds:4;
    unsigned int tens:4;
    unsigned int ones:4;
    int8_t exponent; // Had issues with specifying the field width
};

// Initializes the display
void init_7seg(void);

/* Writes to the upper or lower line
 * Due to space constraint, the range is -999 to 9999.
 * The decimal can be placed at any of the four positions,
 * but default is to not show the decimal */
void write_line(int16_t val, uint8_t decimal, uint8_t line);

/* Writes in magnitude form
 * Only supports positive numbers
 * Currently supports magnitudes of -7 to 3
 */
void write_mag(const struct mag * n, uint8_t line);

/* Increments or decrements the magnitude
 * Step size is determined by the speed
 */
void inc_mag(struct mag * n, int8_t speed);
