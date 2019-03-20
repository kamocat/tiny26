# This is my first makefile. Hopefully it works well.

CFLAGS=-g -mmcu=attiny861 --std=c99 -Wall

all: compile run

compile: elffile
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex

elffile: objectfile
	avr-gcc $(CFLAGS) -o main.elf main.o

objectfile:
	avr-gcc $(CFLAGS) -Os -c main.c -o main.o


clean:
	rm -f *.hex
	rm -f *.elf
	rm -f *.o


AVRFLAGS= -pt861 -Pusb -cusbasp

run: flash


flash:
	sudo avrdude $(AVRFLAGS)  -e -U flash:w:main.hex
