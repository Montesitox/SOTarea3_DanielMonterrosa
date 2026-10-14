CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude -pthread
SRC = src/box.c src/meeseeks.c src/caso_a_texto.c src/caso_b_calculo.c src/caso_c_externo.c src/ipc.c src/utils.c
OBJ = $(SRC:.c=.o)
BIN = bin/box

all: $(BIN)

$(BIN): $(SRC)
	mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $(SRC)

clean:
	rm -f src/*.o $(BIN)

.PHONY: all clean
