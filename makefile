CC=clang
INC=-Iinclude -I/usr/include
LIB=-lglfw -lopenal -lsndfile -lm
CORES=-j8

CFLAGS=-std=c99

SRC=main.c glad.c shader.c sprite.c sound.c mouse.c helpers.c file.c texture.c
OBJ=main.o glad.o shader.o sprite.o sound.o mouse.o helpers.o file.o texture.o

BIN=five-nights-at-freddys

all: release

release: CFLAGS += -O2 -Wall -Wextra -Weverything
release: $(BIN)

debug: CFLAGS += -ggdb3 -Wall -Wextra -Weverything -Werror -D DEBUG
debug: $(BIN)

run:
	make clean
	make release $(CORES)
	clear
	./$(BIN)

gdb:
	clear
	make clean
	make debug $(CORES)
	gdb ./$(BIN) --tui

valgrind:
	make clean
	clear
	make debug $(CORES)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --gen-suppressions=all ./$(BIN)

$(BIN): $(OBJ)
	$(CC) $^ -o $(BIN) $(LIB)
	rm -rf *.o
	@echo "COMPILED SUCCESSFULLY"

%.o: src/%.c
	$(CC) $(CFLAGS) -c $^ $(INC)

clean:
	rm -rf $(BIN) *.o src/*.orig include/*.orig
	clear

format:
	astyle -A3 -s -f -xg -k3 -xj -v src/*.c
	astyle -A3 -s -f -xg -k3 -xj -v include/*.h
