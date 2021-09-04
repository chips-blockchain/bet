PROJNAME := bet
CASHIER := cashierd
LIB := libbet
SRCFILES := cards777.c  client.c  commands.c  gfshare.c  host.c  network.c  oracle.c  payment.c  states.c  table.c poker.c cashier.c storage.c config.c misc.c heartbeat.c
OBJFILES := $(patsubst %.c,%.o,$(SRCFILES))
# CC := gcc-8

UNAME_S := $(shell uname -s)
UNAME_P := $(shell uname -p)
ifeq ($(UNAME_S),Darwin)
WARNINGS := -Werror -Qunused-arguments
else
WARNINGS := -Werror -Wunused-variable
endif

CFLAGS := -g -fPIC -std=c99 $(WARNINGS)

.PHONY: all clean

LIGHTNING := lightning

all: $(PROJNAME) build_dir_tree

clean:
	$(RM) $(OBJFILES) 
	$(RM) $(PROJNAME)	    
	$(RM) $(CASHIER)
	$(RM) $(LIB).a
	$(RM) *.o

EXTERNAL_LIBS_DIR := $(shell cd ../external; pwd)
# $(info $$EXTERNAL_LIBS_DIR is [${EXTERNAL_LIBS_DIR}])

TARGET_DIR := $(shell cd ../external; cd $(shell ${CC} -dumpmachine); pwd)
# $(info $$TARGET_DIR is [${TARGET_DIR}])

LIBWEBSOCKETS_HEADERS = "$(TARGET_DIR)/libwebsockets-build/include"
LIBWALLY_CORE_HEADERS = "$(EXTERNAL_LIBS_DIR)/libwally-core/include"
LIBJSMN_HEADERS = "$(EXTERNAL_LIBS_DIR)/jsmn"
LIBDLG_HEADERS = "$(EXTERNAL_LIBS_DIR/dlg"

LIBWEBSOCKETS_LDFLAGS = "$(TARGET_DIR)/libwebsockets-build/lib"

ifeq ($(UNAME_S),Darwin)
	ifneq ($(filter arm%,$(UNAME_P)),)
		export LDFLAGS+="-L/opt/homebrew/opt/openssl/lib"
		export CPPFLAGS+="-I/opt/homebrew/opt/openssl/include"
	endif
endif

$(info $$CC is [${CC}])

$(PROJNAME): $(OBJFILES)
	        $(CC) $(CFLAGS) -o $@ bet.c $^ ../crypto777/libcrypto777.a -L$(TARGET_DIR) -I$(LIBWEBSOCKETS_HEADERS) -I$(LIBWALLY_CORE_HEADERS) -I$(LIBJSMN_HEADERS) -L$(LIBWEBSOCKETS_LDFLAGS) $(LDFLAGS) $(CPPFLAGS) -lcurl -ldl -lnng -lpthread -lm -lwallycore -lsecp256k1 -ljsmn -lwebsockets -ldlg -L/usr/local/lib -lm -lgmp -lsqlite3 -L/usr/local/lib -lssl -lcrypto -levent -lpthread -lstdc++
	cp $(PROJNAME) $(CASHIER)
%.o: %.c
	$(CC) $(CFLAGS) -L$(TARGET_DIR) -I$(LIBWEBSOCKETS_HEADERS) -I$(LIBWALLY_CORE_HEADERS) -I$(LIBJSMN_HEADERS) -L$(LIBWEBSOCKETS_LDFLAGS) $(LDFLAGS) $(CPPFLAGS) -c $< -o $@  -I ../includes/curl

build_dir_tree:
	sh ./scripts/build_dir_tree.sh
