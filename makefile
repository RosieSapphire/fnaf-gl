CC=clang
INC=-Iinclude -I/usr/include
LIB=-lglfw -lopenal -lsndfile -lfreetype -lm
CORES=-j8

CFLAGS=-std=c99 -Wall -Wextra -Weverything

SRC=main.c glad.c shader.c sprite.c sound.c helpers.c file.c texture.c assets.c font.c
OBJ=main.o glad.o shader.o sprite.o sound.o helpers.o file.o texture.o assets.o font.o

BIN=five-nights-at-freddys

all: release

release: CFLAGS += -O2 
release: $(BIN)

debug: CFLAGS += -Og -ggdb3 -Werror -D DEBUG
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
