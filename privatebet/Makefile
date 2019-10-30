PROJNAME := bet
LIB := libbet
SRCFILES := cards777.c  client.c  commands.c  gfshare.c  host.c  network.c  oracle.c  payment.c  states.c  table.c poker.c cashier.c
OBJFILES := $(patsubst %.c,%.o,$(SRCFILES))
CC := gcc
WARNINGS := -Werror

CFLAGS := -g -fPIC -std=c99 $(WARNINGS)

.PHONY: all clean

LIGHTNING := lightning

all: $(PROJNAME)

clean:
	$(RM) $(OBJFILES) 
	$(RM) $(PROJNAME)	    
	$(RM) $(LIB).a
	$(RM) *.o


$(PROJNAME): $(OBJFILES)
	        $(CC) $(CFLAGS) -o $@ bet.c $^ ../ccan/obj/*.o ../crypto777/libcrypto777.a -Wl,-rpath=$(HOME)/$(LIGHTNING)/external -L$(HOME)/$(LIGHTNING)/external  -lcurl -ldl -lnng -lbacktrace -lpthread -lm -lcli -lwallycore -lsecp256k1 -lsodium -ljsmn -lbase58 -lbacktrace -L/usr/local/lib -lm -lgmp -lsqlite3 -L/usr/local/lib -lboost_system -lboost_filesystem -lboost_program_options -lboost_thread -lboost_chrono -lssl -lcrypto -levent -lpthread -lstdc++  -lwebsockets

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@  -I ../includes/curl
