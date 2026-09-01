CC = gcc
CFLAGS = -Wall -Wextra -std=gnu11

all: mishell

mishell: main.c
	$(CC) $(CFLAGS) -o mishell main.c

clean:
	rm -f mishell