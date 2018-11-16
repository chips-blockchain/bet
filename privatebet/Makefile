PROJNAME := bet
LIB := libbet
SRCFILES := cards777.c  client.c  commands.c  gfshare.c  host.c  network.c  oracle.c  payment.c  states.c  table.c
OBJFILES := $(patsubst %.c,%.o,$(SRCFILES))
CC := gcc
WARNINGS := -w -fpermissive

CFLAGS := -g -fPIC -std=c99 $(WARNINGS)

.PHONY: all clean

all: $(PROJNAME)

clean:
	$(RM) $(OBJFILES) 
	$(RM) $(PROJNAME)	    
	$(RM) $(LIB).a


$(PROJNAME): $(OBJFILES)
	$(CC) $(WARNINGS) -o $@ bet.c $^ ../ccan/obj/*.o ../crypto777/libcrypto777.a -lcurl -ldl -lnng -lbacktrace -lpthread -lm -lcli -L$(HOME)/lightning/external -lwallycore -lsecp256k1 -lsodium -ljsmn -lbase58 -lbacktrace -L/usr/local/lib -lm -lgmp -lsqlite3 ../../chips3/src/bet_chips-bitcoin-cli.o  ../../chips3/src/libbitcoin_cli.a ../../chips3/src/univalue/libunivalue.so ../../chips3/src/libbitcoin_util.a ../../chips3/src/crypto/libbitcoin_crypto.a -L/usr/local/lib -lboost_system -lboost_filesystem -lboost_program_options -lboost_thread -lboost_chrono -lssl -lcrypto -levent -lpthread -lstdc++

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
