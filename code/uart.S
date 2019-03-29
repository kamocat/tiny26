;
; uart.S
; A bitbang method to send UART
;
; Created: 3/25/2019 8:59:04 AM
; Author : hornm
;


#include <avr/io.h>

#define UART_TX_BIT 0
#define UART_RX_BIT 1
#define UART_PORT _SFR_IO_ADDR(PORTA)
#define UART_PIN _SFR_IO_ADDR(PINA)

.global UART_Send_Byte
UART_Send_Byte:
	in r25, _SFR_IO_ADDR(SREG)
	; Each bit takes 8 cycles
	cbi UART_PORT, UART_TX_BIT
	cli
	ldi	r23, 9	; 8 bits to send
NEXT_BIT:
	dec r23	; 1 cycle
	breq STOP_BIT ; 1 cycle (unless stop bit)
	ror r24 ; 1 cycle
	BRCS SET_HIGH	; 1 or 2 cycles
SET_LOW:
	nop	; 1 cycle
	cbi UART_PORT, UART_TX_BIT	; 1 cycle
	rjmp NEXT_BIT ; 2 cycles
SET_HIGH:
	sbi UART_PORT, UART_TX_BIT	; 1 cycle
	rjmp NEXT_BIT ; 2 cycles
STOP_BIT:
	nop
	sbi UART_PORT, UART_TX_BIT
	out _SFR_IO_ADDR(SREG), r25
	ret
	

; We intend to call this in an interrupt, so we need to save SREG
; and all the other registers that are used
; Unfortunately, this interrupt will take about 80 clock cycles.

; We start off 8 clock cyles behind, due to the interrupt overhead
; This means we need to start sampling within the next 5 cycles.
; to hit the middle of the first bit.
.global UART_Receive_Byte
RX_WAIT:
	sbic UART_PIN, UART_RX_BIT ; Simulate the falling-edge interrupt
	rjmp RX_WAIT
	nop
	nop
	nop
	nop
	nop
UART_Receive_Byte:
	push r25	; 2 clock cycles
	in r25, UART_PIN ; 1 clock cycle, first bit sampled here!
	push r18 ; 2 clock cycles
	in r18, _SFR_IO_ADDR(SREG) ; 1 clock cycle, SREG is saved
	push r24 ; 2 clock cycles
	ldi	r24, 0x80 ; This bit is a marker so we know when we're done. 1 cycle
	push r19	; We'll use this for comparing. 2 cycles
RX_LOOP:
	sbic UART_PIN, UART_RX_BIT	; 1 cycle, bits sampled here
	sec	; If bit is 1, set carry flag ; 1 cycle
	ror r24	; rotate carry in, LSB first. 1 cycle
	ldi r19, 1	; 1 cycle
	and r19, r24 ; Is this our last bit?, 1 cycle
	nop; 1 cycle
	breq	RX_LOOP ; If not, keep looping. 2 cycles
	
	sbrs r25, UART_RX_BIT	; Check the first bit
	andi r24, 0xFE	; If it was cleared, fix the received byte
	; Check the stop bit
	sbrs UART_PIN, UART_RX_BIT ; If the stop bit isn't valid
	rjmp RX_CLEANUP ;don't receive the byte
	
	; Put the received byte into a buffer
	
	
RX_CLEANUP:
	; Put back the registers and return
	out _SFR_IO_ADDR(SREG), r18
	pop r19
	mov r18, r24
	pop r24
	mov r24, r18
	pop r18
	pop r25
	reti
	
	