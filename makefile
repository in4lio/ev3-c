TARGET = bin/proto

LIBS = -lm

ifeq ($(OS),Windows_NT)
LIBS := $(LIBS) -lws2_32
endif

PP = python -u $(YUPP_HOME)/yup.py
PPFLAGS = --pp-no-skip-c-comment -q

CC = gcc
CFLAGS = -g -Wall -std=gnu99

DIROBJ = object

.PHONY: default all clean

default: $(TARGET)
all: default

OBJECTS = $(patsubst %.c, $(DIROBJ)/%.o, $(wildcard *.c))
HEADERS = $(wildcard $(DIRSOU)/*.h)

proto.c: proto.yu-c
	$(PP) $(PPFLAGS) $<

$(DIROBJ)/%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

clean:
	-rm -f $(DIROBJ)/*.o
	-rm -f $(TARGET)
