#
# Makefile for blinky
#

MCU = atmega328

F_OSC = 8000000

CFLAGS =  -std=c99 -g -Os -mmcu=$(MCU) $(EXTRAOPT) -DF_CPU=$(F_OSC) -Wl,-u,vfprintf -lprintf_min
# no-optimization settings for interrupt code
CFLAGS_NOOPT = -g -O0 -mmcu=$(MCU) $(EXTRAOPT) -DF_CPU=$(F_OSC) -I../util -Wl,-u,vfprintf -lprintf_min

LDFLAGS = -Wl,-Map=$(<:.o=.map),--cref -mmcu=$(MCU)

FUSEOPT = -U hfuse:w:0xd9:m -U lfuse:w:0xe2:m -U efuse:w:0x07:m

# programmer configuration for avrdude programming utility
# DUDEPORT  = -c avrispv2 -P /dev/ttyACM1
DUDEPORT  = -c avrispv2 -P usb
DUDEMCU = m328p
AVRDUDE  = avrdude $(DUDEPORT) -p $(DUDEMCU) -y -u 

objects = main.o neopixel.o

all: $(objects)  main.hex list

clean:
	rm -f *.o *~ *.hex *.elf *.map *.lst

flash: main.hex
	$(AVRDUDE) -U flash:w:$^:i

list:
	avr-objdump -h -S main.elf > main.lst
	avr-size --common -d main.elf

$(objects): %.o: %.c
	avr-gcc -c $(CFLAGS) $< -o $@

main.elf: $(objects)
	avr-gcc $(LDFLAGS) $(objects) -o $@

main.hex: main.elf
	avr-objcopy -j .text -j .data -O ihex $< $@

fuse:
	$(AVRDUDE) $(FUSEOPT)

status:
	$(AVRDUDE) -v
