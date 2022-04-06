# This is my first makefile. Hopefully it works well.

MCU=attiny861
CFLAGS=-g -mmcu=$(MCU) --std=c99 -Wall -DF_CPU=1000000UL

all: compile run

compile: elffile
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex
	avr-size -C --mcu=$(MCU) main.elf

elffile: objectfile
	avr-gcc $(CFLAGS) -o main.elf main.o 7seg.o pwm.o decimal.o

objectfile:
	avr-gcc $(CFLAGS) -Os -c main.c -o main.o
	avr-gcc $(CFLAGS) -Os -c 7seg.c -o 7seg.o
	avr-gcc $(CFLAGS) -Os -c pwm.c -o pwm.o
	avr-gcc $(CFLAGS) -Os -c decimal.c -o decimal.o

inspect:
	avr-gcc $(CFLAGS) -Os -S 7seg.c -o inspect.asm
	vim inspect.asm

test:
	gcc decimal.c test_decimal.c -o run_test.out
	./run_test.out

clean:
	rm -f *.hex
	rm -f *.elf
	rm -f *.o
	rm -f *.out


AVRFLAGS= -pt861 -Pusb -cusbtiny

run: flash


flash:
	sudo avrdude $(AVRFLAGS)  -e -U flash:w:main.hex
