CC=gcc
CFLAGS= -w -g -c -fpic
SRC=bet.c
OBJ=$(patsubst %.c,%.o,$(SRC))
HOME=/root/bet
.PHONY: all clean compile_common

all:	
	gcc -w -g -o bet bet.c -L/root/bet/commom -lcommon -lnng -lcurl -lbacktrace -lpthread -lm	
	

clean:
	rm -f *.o
	rm -f *.so
