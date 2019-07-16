PROJNAME := bet
LIB := libbet
SRCFILES := cards777.c  client.c  commands.c  gfshare.c  host.c  network.c  oracle.c  payment.c  states.c  table.c poker.c
OBJFILES := $(patsubst %.c,%.o,$(SRCFILES))
CC := gcc
WARNINGS := -w -fpermissive

CFLAGS := -g -fPIC -std=c99 $(WARNINGS)

.PHONY: all clean


#all: bet_chips $(PROJNAME)

all: $(PROJNAME)

clean:
	$(RM) $(OBJFILES) 
	$(RM) $(PROJNAME)	    
	$(RM) $(LIB).a
	$(RM) *.o 

$(PROJNAME): $(OBJFILES)
	        $(CC) $(CFLAGS) -o $@ bet.c $^ ../ccan/obj/*.o ../crypto777/libcrypto777.a -Wl,-rpath=$(HOME)/lightning/external -L$(HOME)/lightning/external -lcurl -ldl -lnng -lbacktrace -lpthread -lm -lcli -lwallycore -lsecp256k1 -lsodium -ljsmn -lbase58 -lbacktrace -L/usr/local/lib -lm -lgmp -lsqlite3  ../../chips3/src/libbitcoin_cli.a ../../chips3/src/univalue/libunivalue.so ../../chips3/src/libbitcoin_util.a ../../chips3/src/crypto/libbitcoin_crypto.a -L/usr/local/lib -lboost_system -lboost_filesystem -lboost_program_options -lboost_thread -lboost_chrono -lssl -lcrypto -levent -lpthread -lstdc++

#$(PROJNAME): $(OBJFILES)
#	$(CC) $(CFLAGS) -o $@ bet.c $^ ../ccan/obj/*.o ../crypto777/libcrypto777.a -Wl,-rpath=$(HOME)/lightning/external -L$(HOME)/lightning/external -lcurl -ldl -lnng -lbacktrace -lpthread -lm -lcli -lwallycore -lsecp256k1 -lsodium -ljsmn -lbase58 -lbacktrace -L/usr/local/lib -lm -lgmp -lsqlite3 bet_chips-bitcoin-cli.o  ../../chips3/src/libbitcoin_cli.a ../../chips3/src/univalue/libunivalue.so ../../chips3/src/libbitcoin_util.a ../../chips3/src/crypto/libbitcoin_crypto.a -L/usr/local/lib -lboost_system -lboost_filesystem -lboost_program_options -lboost_thread -lboost_chrono -lssl -lcrypto -levent -lpthread -lstdc++

#bet_chips: 
#	g++ -std=c++11 -DHAVE_CONFIG_H -I ../../chips3/src/ -I ../../chips3/src/config  -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2 -I ../../chips3/src/ -I ../../chips3/src/obj  -DBOOST_SP_USE_STD_ATOMIC -DBOOST_AC_USE_STD_ATOMIC -pthread -I/usr/local/include -I ../../chips3/src/leveldb/include -I ../../chips3/src/leveldb/helpers/memenv   -I ../../chips3/src/secp256k1/include -I ../../chips3/src/univalue/include  -DHAVE_BUILD_INFO -D__STDC_FORMAT_MACROS -Wstack-protector -fstack-protector-all  -fPIC -g -O2 -Wall -Wextra -Wformat -Wvla -Wformat-security -Wno-unused-parameter -Wno-implicit-fallthrough -MT chips_cli-bitcoin-cli.o -MD -MP -MF ../../chips3/src/.deps/chips_cli-bitcoin-cli.Tpo -c -o bet_chips-bitcoin-cli.o ../../chips3/src/bitcoin-cli_1.cpp

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@  -I ../includes/curl
