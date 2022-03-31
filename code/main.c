#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 1000000UL // 1MHz
#include <util/delay.h>

ISR( BADISR_vect ) {
	//catch anything we accidentally enabled
}

/* Code for the MAX7219 7seg driver */
void spi_send(uint8_t * buf, uint8_t n){
    const uint8_t hi = 0x11; // SPI master, set clock high
    const uint8_t lo = 0x13; // SPI master, set clock low and shift data
    ++n;
    PORTB &= ~(1<<4); //Set PB4 low
    while(--n){
        USIDR = *buf++;
        USICR = hi;
        USICR = lo;
        USICR = hi;
        USICR = lo;
        USICR = hi;
        USICR = lo;
        USICR = hi;
        USICR = lo;
        USICR = hi;
        USICR = lo;
        USICR = hi;
        USICR = lo;
        USICR = hi;
        USICR = lo;
        USICR = hi;
        USICR = lo;
    }
    PORTB |= (1<<4); //Set PB4 hi
}

void spi_cmd(uint8_t cmd, uint8_t val){
    uint8_t buf[2];
    buf[0] = cmd;
    buf[1] = val;
    spi_send(buf, 2);
}

void init_7seg(void){
    spi_cmd(9, 0xFF); // Control digits not individual segments
    spi_cmd(10, 8);  // Intensity 8. (range is 0-15)
    spi_cmd(11, 7);  // Display all 8 digits
    spi_cmd(12, 1);  // Enable the display
}

int main( void ) {
	
	
	/* PORT SETUP */
	DDRA = 0x00;	// All of Port A is inputs
	DIDR0 = 0xF0;	// disable digital input buffers on pA5 through pA7
	DDRB = 0x7F;	// set all PORTB as outputs, except RESET.
	
    init_7seg();
	while(1) {
      spi_cmd(1, 0xA5);
	  _delay_ms(1);
	}

	return 0;
}
