CC = gcc
CFLAGS = -Wall -Wextra -std=gnu11 -Iinclude
SRC = src/main.c src/shell.c src/pipes.c src/redireccion.c src/senales.c
OBJ = $(SRC:.c=.o)
TARGET = mishell

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lreadline

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET)

.PHONY: all clean