CC=clang
INC=-Iinclude/
LIB=-Llib -lglfw3 -lpthread -ldl -lcglm -lfreetype -lalut -lopenal -lsndfile -lm

CFLAGS=-std=c99

SRC=main.c glad.c file.c texture.c sprite.c shader.c mouse.c sound.c
OBJ=main.o glad.o file.o texture.o sprite.o shader.o mouse.o sound.o

BIN=five_nights_at_freddys

all: release

release: $(BIN)
release: CFLAGS += -O2 -Wall -Wextra -Weverything

debug: $(BIN)
debug: CFLAGS += -Og -ggdb3 -Wall -Wextra -Weverything -Werror -D DEBUG

run: release
	clear
	./$(BIN)

gdb: debug
	clear
	gdb ./$(BIN) --tui

valgrind: debug
	clear
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

cleanrun:
	make clean
	make run -j8

cleandebug:
	make clean
	make gdb -j8

format:
	astyle -A3 -s -f -xg -k3 -xj -v src/*.c
	astyle -A3 -s -f -xg -k3 -xj -v include/*.h
