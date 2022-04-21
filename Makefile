# This makefile is based on the one taken from
# http://mainloop.ru/avr-atmega/makefile-avr-gcc.html

TARG=timer

CC = avr-gcc
OBJCOPY = avr-objcopy

SRCS= sources/main.c    \
      sources/fsm.c     \
      sources/isr.c     \
      sources/printer.c \
      sources/button.c

OBJS = $(SRCS:.c=.o)
MCU=atmega8
CFLAGS = -mmcu=$(MCU) -Wall -g -Os -Werror -lm  -mcall-prologues -DF_CPU=1000000UL
LDFLAGS = -mmcu=$(MCU) -Wall -g -Os  -Werror

all: $(TARG)

$(TARG): $(OBJS)
	$(CC) $(LDFLAGS) -o $@.elf  $(OBJS) -lm
	$(OBJCOPY) -O binary -R .eeprom -R .nwram  $@.elf $@.bin
	$(OBJCOPY) -O ihex -R .eeprom -R .nwram  $@.elf $@.hex

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.elf *.bin *.hex  $(OBJS) *.map
