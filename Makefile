CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
SRC = src/main.c src/partition.c src/buddy.c
OBJ = $(SRC:.c=.o)
BIN = tp2_memoria

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $(BIN) $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(BIN)

.PHONY: all clean
