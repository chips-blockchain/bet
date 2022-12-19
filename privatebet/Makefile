PROJNAME := bet
CASHIER := cashierd
LIB := libbet
SRCFILES := cards777.c  client.c  commands.c  gfshare.c  host.c  network.c  oracle.c  payment.c  states.c  table.c poker.c cashier.c storage.c config.c misc.c heartbeat.c help.c err.c vdxf.c player.c blinder.c dealer.c
OBJFILES := $(patsubst %.c,%.o,$(SRCFILES))
# CC := gcc-8
BET_VERSION=$(shell git describe --always --abbrev=7 2>/dev/null || pwd | sed -n 's|.*/c\{0,1\}bet-v\{0,1\}\([0-9a-f.rc\-]*\)$$|\1|gp')
# UPDATE_BET_VERSION=$(shell echo "char BET_VERSION[200]=\"${BET_VERSION}\";" > bet_version.h)

UNAME_S := $(shell uname -s)
UNAME_P := $(shell uname -p)
ifeq ($(UNAME_S),Darwin)
WARNINGS := -Werror -Qunused-arguments
else
#WARNINGS := -Werror -Wunused-variable
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
LIBJSMN_HEADERS = "$(EXTERNAL_LIBS_DIR)/jsmn"
LIBDLG_HEADERS = "$(EXTERNAL_LIBS_DIR)/dlg"
LIBINIPARSER_HEADERS = "$(EXTERNAL_LIBS_DIR)/iniparser-build/src"
LIBINIPARSER_LDFLAGS = "$(EXTERNAL_LIBS_DIR)/iniparser"
LIBWEBSOCKETS_LDFLAGS = "$(TARGET_DIR)/libwebsockets-build/lib"

ifeq ($(UNAME_S),Darwin)
	ifneq ($(filter arm%,$(UNAME_P)),)
		export LDFLAGS+="-L/opt/homebrew/opt/openssl/lib"
		export CPPFLAGS+="-I/opt/homebrew/opt/openssl/include"
	endif
endif

$(info $$CC is [${CC}])
$(info $$BET_VERSION is [${BET_VERSION}])

$(PROJNAME): $(OBJFILES)
			$(shell echo "#define BET_VERSION \"${BET_VERSION}\"" > bet_version.h)
	        $(CC) $(CFLAGS) -o $@ bet.c $^ ../external/iniparser/libiniparser.a ../crypto777/libcrypto777.a -L$(TARGET_DIR) -I$(LIBWEBSOCKETS_HEADERS) -I$(LIBJSMN_HEADERS) -I$(LIBINIPARSER_HEADERS) -L$(LIBWEBSOCKETS_LDFLAGS) $(LDFLAGS) $(CPPFLAGS) -lcurl -ldl -lnng -lpthread -lm -ljsmn -lwebsockets -ldlg -L/usr/local/lib -lm -lgmp -lsqlite3 -L/usr/local/lib -lssl -lcrypto -levent -lpthread -lstdc++
	cp $(PROJNAME) $(CASHIER)
%.o: %.c
	$(CC) $(CFLAGS) -L$(TARGET_DIR) -I$(LIBINIPARSER_HEADERS) -I$(LIBWEBSOCKETS_HEADERS) -I$(LIBJSMN_HEADERS) -L$(LIBINIPARSER_LDFLAGS) -L$(LIBWEBSOCKETS_LDFLAGS) $(LDFLAGS) $(CPPFLAGS) -c $< -o $@  -I ../includes/curl

build_dir_tree:
	sh ./scripts/build_dir_tree.sh
