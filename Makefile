CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
LDFLAGS = -lpthread

SRC = src
OBJ = obj

all: agenteM monitor

$(OBJ):
	mkdir -p $(OBJ)

$(OBJ)/parser.o: $(SRC)/parser.c include/parser.h include/comun.h | $(OBJ)
	$(CC) $(CFLAGS) -c $< -o $@
$(OBJ)/buffer.o: $(SRC)/buffer.c include/buffer.h include/comun.h | $(OBJ)
	$(CC) $(CFLAGS) -c $< -o $@
$(OBJ)/agenteM.o: $(SRC)/agenteM.c include/comun.h include/parser.h | $(OBJ)
	$(CC) $(CFLAGS) -c $< -o $@
$(OBJ)/monitor.o: $(SRC)/monitor.c include/comun.h include/buffer.h include/parser.h | $(OBJ)
	$(CC) $(CFLAGS) -c $< -o $@
agenteM: $(OBJ)/agenteM.o $(OBJ)/parser.o
	$(CC) $^ -o $@
monitor: $(OBJ)/monitor.o $(OBJ)/parser.o $(OBJ)/buffer.o
	$(CC) $^ -o $@ $(LDFLAGS)
clean:
	rm -rf $(OBJ) agenteM monitor consolidado.csv
.PHONY: all clean
