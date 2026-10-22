# TEG - Tarefa 2: grafo de palavras de 4 letras
# Uso: make | make run | make test | make debug | make clean

CC      ?= gcc
CFLAGS  ?= -std=c11 -Wall -Wextra -Wpedantic -O2
LDFLAGS ?=

BIN  := teg
SRC  := src/main.c src/grafo.c src/analise.c src/dijkstra.c
OBJ  := $(SRC:src/%.c=build/%.o)
HDR  := $(wildcard src/*.h)

.PHONY: all run test debug clean

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

build/%.o: src/%.c $(HDR) | build
	$(CC) $(CFLAGS) -c -o $@ $<

build:
	mkdir -p build

run: $(BIN)
	./$(BIN) entrada.txt

# Testes automatizados (compilam com sanitizers e conferem as saídas)
test:
	bash tests/run_tests.sh

# Compilação com AddressSanitizer + UndefinedBehaviorSanitizer
debug: clean
	$(MAKE) CFLAGS="-std=c11 -Wall -Wextra -Wpedantic -g -O0 -fsanitize=address,undefined" \
	        LDFLAGS="-fsanitize=address,undefined"

clean:
	rm -rf build $(BIN)
