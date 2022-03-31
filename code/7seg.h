#include <stdint.h>
// Initializes the display
void init_7seg(void);

// Writes to the upper or lower line
void write_line(int16_t val, uint8_t decimal, uint8_t line);
