CC=gcc
CFLAGS= -w -g -c -fpic
SRC=bet.c
OBJ=$(patsubst %.c,%.o,$(SRC))
HOME=/root/bet


default:	
	gcc -w -g -o bet-cli bet-cli.c  *.o ../ccan/obj/*.o ../external/jsmn/jsmn.o ../crypto777/libcrypto777.a -lcurl -ldl -lnng -lbacktrace -lpthread -lm
 
clean:
	rm bet
