CC=gcc
CFLAGS= -w -g -c -fpic
SRC=bet.c
OBJ=$(patsubst %.c,%.o,$(SRC))
HOME=/root/bet
.PHONY: all clean

all:clean	
	
	gcc -w -g -o bet bet.c ../ccan/obj/*.o ../external/jsmn/jsmn.o ../crypto777/libcrypto777.a -lcurl -ldl -lnng -lbacktrace -lpthread -lm
 
clean:
	#do nothing
