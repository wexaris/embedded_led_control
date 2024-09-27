# Name of the resulting program/hex file
PROGNAME ?= test

# Build options; pins[0-3]
LED_PIN_0 ?= 1
LED_PIN_1 ?= 2
SERIAL_BAUD_RATE ?= 9600

# Directories
SRC = src
BIN ?= bin
OBJ ?= obj

# Valid avr-gcc device from this list:
# http://www.atmel.com/webdoc/avrlibcreferencemanual/index_1supp_devices.html
DEVICE ?= atmega328
# Processor speed (check your datasheet)
F_CPU ?= 16000000UL

# AvrDude Device Name is often slightly different from gcc device name
DUDE_DEVICE ?= atmega328p
PROGRAMMER ?= arduino
BAUD ?= 115200
PORT ?= /dev/ttyACM0

# Compilation settings
CC = avr-gcc
CXX = avr-g++
CPPFLAGS = -Wall -Os -mmcu=$(DEVICE) -DF_CPU=$(F_CPU)
BUILD_OPTS = -DLED_PIN_0=$(LED_PIN_0) -DLED_PIN_1=$(LED_PIN_1) -DUART_BAUD=$(SERIAL_BAUD_RATE)

all: build

### Compile program ###
build: $(PROGNAME)
$(PROGNAME): $(BIN)/$(PROGNAME).hex size

# obj <- c
.PRECIOUS: $(OBJ)/%.o
$(OBJ)/%.o: $(SRC)/%.c
	@mkdir -p $(OBJ)
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(BUILD_OPTS) $^ -o $@

# obj <- cpp
$(OBJ)/%.o: $(SRC)/%.cpp
	@mkdir -p $(OBJ)
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) $(BUILD_OPTS) $^ -o $@


### Upload to controller ###
install: $(BIN)/$(PROGNAME).hex
	avrdude -v -p$(DUDE_DEVICE) -c$(PROGRAMMER) -P $(PORT) -b$(BAUD) -D -Uflash:w:$<:i

# hex <- elf
$(BIN)/%.hex: $(BIN)/%.elf
	avr-objcopy -j .text -j .data -O ihex $^ $@

# elf <- [obj]
objects := $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(wildcard $(SRC)/*.c*))
objects := $(patsubst $(SRC)/%.cpp, $(OBJ)/%.o, $(objects))

.PRECIOUS: $(BIN)/%.elf
$(BIN)/%.elf: $(objects)
	@mkdir -p $(BIN)
	$(CC) $(CPPFLAGS) $^ -o $@


.PHONY: size
size:
	avr-size --format=avr --mcu=$(DEVICE) $(BIN)/$(PROGNAME).elf

.PHONY: clean
clean:
	$(RM) $(BIN)/* $(OBJ)/*

### Predefined variables
# $^ - all prereqs
# $@ - target
# $< - first prereq
