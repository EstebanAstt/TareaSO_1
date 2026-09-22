CC = gcc
CFLAGS = -Wall -Wextra -std=gnu11 -Iinclude
TARGET = mishell

SRCS = src/main.c src/shell.c src/pipes.c src/redireccion.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o *.o mishell