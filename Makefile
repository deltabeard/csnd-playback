# This file is for use with GNU Make only.
# Project details
NAME		:= CSND-Playback
DESCRIPTION	:= Use CSND to play music
COMPANY		:= Deltabeard
AUTHOR		:= Mahyar Koshkouei
LICENSE_SPDX	:= BSD-0

#include $(DEVKITARM)/3ds_rules
export PATH := $(DEVKITPRO)/tools/bin:$(DEVKITPRO)/devkitARM/bin:$(PATH)
PREFIX	:= arm-none-eabi-
CC	:= $(PREFIX)gcc
CXX	:= $(PREFIX)g++
ARCH	:= armv6k
CFLAGS	:= -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft -D__3DS__ \
	-I$(DEVKITPRO)/libctru/include
LDFLAGS	= -specs=3dsx.specs -L$(DEVKITPRO)/libctru/lib -lctru

SRCS := $(wildcard src/*.c) $(wildcard src/**/*.c)
OBJS += $(SRCS:.c=.o)
TARGET := $(NAME).elf

override CFLAGS += -Iinc $(EXTRA_CFLAGS)
override LDFLAGS += $(EXTRA_LDFLAGS)

all: $(TARGET)

%.elf: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	$(RM) $(OBJS) $(TARGET)
