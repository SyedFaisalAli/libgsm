CROSS_COMPILE=
CC=$(CROSS_COMPILE)gcc
AR=$(CROSS_COMPILE)ar
SOURCES=gsm.c
OBJECTS=$(SOURCES:.c=.o)
CFLAGS=-g -std=c99
OUT=libgsms.a

all: $(OUT) test

$(OUT): $(OBJECTS)
	$(AR) rcs $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $<


test: $(OBJECTS)
	$(CC) $(CFLAGS) -o gsm gsmtest.c -L. -lgsms

clean:
	rm *.o

.PHONY: clean test
