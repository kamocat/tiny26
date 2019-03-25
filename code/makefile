# This is my first makefile. Hopefully it works well.

CFLAGS= -mmcu=attiny261a --std=c99 -Wall -Os
ASMFLAGS= -mmcu=attiny261a

all: compile run

compile: elffile
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex

elffile: objectfile
	avr-gcc $(ASMFLAGS) -o main.elf main.o uart.o

objectfile:
	avr-gcc $(CFLAGS) -c main.c -o main.o
	avr-gcc $(ASMFLAGS) -c uart.S -o uart.o


clean:
	rm -f *.hex
	rm -f *.elf
	rm -f *.o


AVRFLAGS= -pt861 -Pusb -cusbasp

run: flash


flash:
	sudo avrdude $(AVRFLAGS)  -e -U flash:w:main.hex
