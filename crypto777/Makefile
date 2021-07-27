# CC=gcc
CFLAGS= -w -g -c -fpic
SRC=bet.c
OBJ=$(patsubst %.c,%.o,$(SRC))
HOME=/root/bet


default:
	$(CC) -c -w -O2 *.c  -I ../includes
	$(AR) rcu libcrypto777.a *.o

clean:
	$(RM) *.o
	$(RM) -f libcrypto777.a
