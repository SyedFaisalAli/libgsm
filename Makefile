CROSS_COMPILE=
CC=$(CROSS_COMPILE)gcc
AR=$(CROSS_COMPILE)ar
SOURCES=gsm.c
OBJECTS=$(SOURCES:.c=.o)
CFLAGS=-g
OUT=libgsm.a

all: $(OUT) test

$(OUT): $(OBJECTS)
	$(AR) rcs $@ $^

%.o: %.c
	$(CC) $(CFLAGS) --std=c99 -c $<


test: $(OBJECTS)
	$(CC) -g -o gsm gsmtest.c -L. -lgsm

clean:
	rm *.o

.PHONY: clean test
