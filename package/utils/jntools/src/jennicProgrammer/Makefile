# ------------------------------------------------------------------
# JennicModuleProgrammer program makefile
# ------------------------------------------------------------------
# Author:    nlv10677
# Copyright: NXP B.V. 2015. All rights reserved
# ------------------------------------------------------------------

TARGET = iot_jp

EXTRA_CFLAGS = -DDBG_ENABLE -DVERSION="\"$(shell cat ../jennicCommon/version.txt)\""

# Define to trace bootloader comms
# EXTRA_CFLAGS += -DDEBUG_BOOTLOADER 

# Define to trace firmware image info
# EXTRA_CFLAGS += -DDEBUG_FIRMWARE

# Define to trace serial comms
# EXTRA_CFLAGS += -DDEBUG_UART
 
# Define to trace programmer
# EXTRA_CFLAGS += -DDEBUG_PROGRAMMER

OBJECTS = ../jennicCommon/uart.o \
	../jennicCommon/JN51xx_BootLoader.o \
	../jennicCommon/Firmware.o \
	../jennicCommon/programmer.o \
	../IotCommon/dump.o \
	../IotCommon/iotSemaphore.o \
	../IotCommon/newLog.o \
	jp_main.o

BINEXT = FlashProgrammerExtension_JN5168.bin \
	FlashProgrammerExtension_JN5169.bin \
	FlashProgrammerExtension_JN5179.bin

INCLUDES = -I../jennicCommon  -I../IotCommon
EXTRA_CFLAGS += -DPOSIX -DVERSION_MAJOR="\"0\"" -DVERSION_MINOR="\"11\"" -DVERSION_SVN="\"1234\""

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(INCLUDES) -Wall -g -c $< -o $@

all: build

build: $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $(TARGET) -lc

clean:
	-rm -f $(OBJECTS)
	-rm -f $(TARGET)

