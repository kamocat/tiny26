;
; uart.S
; A bitbang method to send UART
;
; Created: 3/25/2019 8:59:04 AM
; Author : hornm
;


#include <avr/io.h>

#define UART_BIT 0
#define UART_PORT _SFR_IO_ADDR(PORTA)

.global UART_Send_Byte
UART_Send_Byte:
	in r25, _SFR_IO_ADDR(SREG)
	; Each bit takes 8 cycles
	cbi UART_PORT, UART_BIT
	cli
	ldi	r23, 9	; 8 bits to send
NEXT_BIT:
	dec r23	; 1 cycle
	breq STOP_BIT ; 1 cycle (unless stop bit)
	ror r24 ; 1 cycle
	BRCS SET_HIGH	; 1 or 2 cycles
SET_LOW:
	nop	; 1 cycle
	cbi UART_PORT, UART_BIT	; 1 cycle
	rjmp NEXT_BIT ; 2 cycles
SET_HIGH:
	sbi UART_PORT, UART_BIT	; 1 cycle
	rjmp NEXT_BIT ; 2 cycles
STOP_BIT:
	nop
	sbi UART_PORT, UART_BIT
	out _SFR_IO_ADDR(SREG), r25
	ret
