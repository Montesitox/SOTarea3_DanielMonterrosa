CC = gcc
CFLAGS = -Wall -Wextra -g -std=c11 -D_POSIX_C_SOURCE=200809L -Iinclude -pthread
LDFLAGS = -pthread -lm -lrt
SRC = src/box.c src/meeseeks.c src/caso_a_texto.c src/caso_b_calculo.c src/caso_c_externo.c src/ipc.c src/utils.c
BIN = bin/box

all: $(BIN)

$(BIN): $(SRC)
	mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $(SRC) $(LDFLAGS)

clean:
	rm -f src/*.o $(BIN)

.PHONY: all clean
