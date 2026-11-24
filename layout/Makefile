CC ?= gcc
CFLAGS ?= -O2 -std=c11 -Wall -Wextra

all: layout

layout: src/layout.c
	$(CC) $(CFLAGS) $< -o $@ -lm

test: layout
	./layout

clean:
	rm -f layout
