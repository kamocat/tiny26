#include "7seg.h"
#include <avr/io.h>
/* Code for the MAX7219 7seg driver */
void spi_send(uint8_t byte){
    // Bit-banging because the USI wasn't outputting data
    for(uint8_t i = 8; i; --i){
        uint8_t tmp = PORTB & ~0x05;
        if(byte & 0x80){
            tmp |= 1;
        }
        PORTB = tmp;
        byte <<= 1;
        PORTB |= 4;
    }
    PORTB &= ~4; // Clock rest low
}

void spi_cmd(uint8_t cmd, uint8_t val){
    PORTB &= ~(1<<4); //Set PB4 low
    spi_send(cmd);
    spi_send(val);
    PORTB |= (1<<4); //Set PB4 hi
}

void init_7seg(void){
    spi_cmd(9, 0xFF); // Control digits not individual segments
    spi_cmd(10, 8);  // Intensity 8. (range is 0-15)
    spi_cmd(11, 7);  // Display all 8 digits
    spi_cmd(12, 1);  // Enable the display
}

/* Calculate dividend and remainder at the same time */
uint8_t div_remainder(int16_t * val, int16_t increment){
    uint8_t digit;
    for(digit = 0; *val>=increment; *val -= increment){
        ++digit;
    }
    return digit;
}

void write_line(int16_t val, uint8_t decimal, uint8_t line){
    /* Due to space constraint, the range is -999 to 9999.
     * The decimal can be placed at any of the four positions,
     * but default is to not show the decimal */
    uint8_t digit[4];
    if(val > 9999 )
        val = 9999;
    if(val < -999)
        val = -999;
    if(val < 0){
        val = -val;
        digit[3] = 10; // negative sign
    } else {
        digit[3] = div_remainder(&val, 1000);
    }
    digit[2] = div_remainder(&val, 100);
    digit[1] = div_remainder(&val, 10);
    digit[0] = val;
    if(decimal < 3)
        digit[decimal] |= 0x80; // Set decimal point
    if(line == 1){
        spi_cmd(1, digit[0]);
        spi_cmd(2, digit[1]);
        spi_cmd(3, digit[2]);
        spi_cmd(4, digit[3]);
    } else {
        spi_cmd(5, digit[0]);
        spi_cmd(6, digit[1]);
        spi_cmd(7, digit[2]);
        spi_cmd(8, digit[3]);
    }
}
