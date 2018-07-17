CC=gcc
CFLAGS= -w -g -c -fpic
SRC=bet.c
OBJ=$(patsubst %.c,%.o,$(SRC))
HOME=/root/bet


default:
	gcc -c -w -O2 *.c  
	ar rcu libcrypto777.a *.o

clean:
	rm *.o
	rm -f libcrypto777.a
