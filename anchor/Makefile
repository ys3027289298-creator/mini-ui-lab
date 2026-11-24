CC ?= gcc
CFLAGS ?= -O2 -std=c11 -Wall -Wextra

all: anchor

anchor: src/anchor.c
	$(CC) $(CFLAGS) $< -o $@ -lm

test: anchor
	./anchor

clean:
	rm -f anchor
