CC ?= gcc
CFLAGS ?= -O2 -std=c11 -Wall -Wextra

all: text

text: src/text.c
	$(CC) $(CFLAGS) $< -o $@ -lm

test: text
	./text

clean:
	rm -f text
