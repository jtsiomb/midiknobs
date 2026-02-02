src = $(wildcard src/*.c)
obj = $(src:.c=.o)
bin = midiknobs
hex = $(bin).hex
eep = $(bin).eep

mcu_gcc = atmega88
mcu_dude = m88

CC = avr-gcc
OBJCOPY = avr-objcopy

CFLAGS = -O2 -pedantic -Wall -mmcu=$(mcu_gcc) -DF_CPU=4000000
LDFLAGS = -Wl,-Map,$(bin).map -mmcu=$(mcu_gcc) -lprintf_min

.PHONY: all
all: $(hex) $(eep)

$(bin): $(obj)
	$(CC) -o $@ $(obj) $(LDFLAGS)

$(hex): $(bin)
	$(OBJCOPY) -j .text -j .data -O ihex -R .eeprom $< $@

$(eep): $(bin)
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O ihex $< $@

.PHONY: fuses
fuses:
	avrdude -c usbtiny -p $(mcu_dude) -U lfuse:w:0xd6:m -U hfuse:w:0xdf:m -U efuse:w:0xf9:m

.PHONY: program
program: $(hex)
	avrdude -c usbtiny -p $(mcu_dude) -e -U flash:w:$(hex)

.PHONY: clean
clean:
	rm -f $(bin) $(obj) $(hex) $(eep) $(bin).map

.PHONY: piprog
piprog: $(hex)
	ssh pi4 '[ -d /neutrino/proj/midiknobs ] || mount /neutrino/proj && cd /neutrino/proj/midiknobs/fw && make program'
