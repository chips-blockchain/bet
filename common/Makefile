CC=gcc
SRC=cJSON.c json.c json_escaped.c
OBJ=$(patsubst %.c,%.o,$(SRC))
HOME=/root/bet
.PHONY: all clean compile_common

compile_common:$(OBJ)
%.o: %.c
	$(CC) -c -fpic $^ -o $(patsubst $@,$@,$@) -I $(HOME) -I $(HOME)/ccan

all: compile_common
	$(CC) -shared -o libcommon.so $(OBJ)

clean:
	rm -f *.o
	rm -f *.so
