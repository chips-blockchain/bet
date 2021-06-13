SUBMODULES =					\
	external/libwally-core			\
	external/jsmn				\
	external/nng			\
	external/libwebsockets

TOP := ../..
ifdef BUILD
CROSSCOMPILE_OPTS := --host="$(MAKE_HOST)" --build="$(BUILD)"
TARGET_DIR := external/$(MAKE_HOST)
else
TARGET_DIR := external/$(shell ${CC} -dumpmachine)
endif

UNAME_S := $(shell uname -s)
UNAME_P := $(shell uname -p)
ifeq ($(UNAME_S),Darwin)
	ifneq ($(filter arm%,$(UNAME_P)),)
        export OPENSSL_ROOT_DIR=/opt/homebrew/opt/openssl
    endif
endif

LIBWALLY_HEADERS := external/libwally-core/include/wally_bip32.h		\
			external/libwally-core/include/wally_core.h	\
			external/libwally-core/include/wally_psbt.h	\
			external/libwally-core/include/wally_crypto.h
LIBSECP_HEADERS := external/libwally-core/src/secp256k1/include/secp256k1_ecdh.h		\
		external/libwally-core/src/secp256k1/include/secp256k1_recovery.h		\
		external/libwally-core/src/secp256k1/include/secp256k1.h
JSMN_HEADERS := external/jsmn/jsmn.h
NNG_HEADERS := external/nng/include/nng.h \
		external/nng/include/nng/compat/nanomsg/nn.h
LIBWEBSOCKETS_HEADERS := external/libwebsockets/include/libwebsockets.h

EXTERNAL_HEADERS := $(LIBWALLY_HEADERS) $(LIBSECP_HEADERS) $(JSMN_HEADERS) $(NNG_HEADERS) $(LIBWEBSOCKETS_HEADERS)
EXTERNAL_LIBS := ${TARGET_DIR}/libwallycore.a ${TARGET_DIR}/libsecp256k1.a ${TARGET_DIR}/libjsmn.a ${TARGET_DIR}/libnng.a ${TARGET_DIR}/libwebsockets.a

EXTERNAL_LIBS_CLEAN := ${TARGET_DIR}/libwallycore.a ${TARGET_DIR}/libsecp256k1.a ${TARGET_DIR}/libjsmn.a ${TARGET_DIR}/libnng.a ${TARGET_DIR}/libwebsockets.a

EXTERNAL_INCLUDE_FLAGS :=					\
	-I external/libwally-core/include/			\
	-I external/libwally-core/src/secp256k1/include/	\
	-I external/jsmn/					\
	-I external/nng/include/				\
	-I external/libwebsockets/include/

EXTERNAL_LDLIBS := -L${TARGET_DIR} $(patsubst lib%.a,-l%,$(notdir $(EXTERNAL_LIBS)))

submodcheck: 
	@tools/refresh-submodules.sh $(SUBMODULES)
	@cd external/libwally-core && ../../tools/refresh-submodules.sh src/secp256k1

$(EXTERNAL_HEADERS): submodcheck

# libsecp included in libwally.
# Wildcards here are magic.  See http://stackoverflow.com/questions/2973445/gnu-makefile-rule-generating-a-few-targets-from-a-single-source-file
$(TARGET_DIR)/libsecp256k1.% $(TARGET_DIR)/libwallycore.%: $(TARGET_DIR)/libwally-core-build/src/secp256k1/libsecp256k1.la $(TARGET_DIR)/libwally-core-build/src/libwallycore.la
	$(MAKE) -C $(TARGET_DIR)/libwally-core-build DESTDIR=$$(pwd)/$(TARGET_DIR) install-exec

# Build libwally-core.
$(TARGET_DIR)/libwally-core-build/src/libwallycore.% $(TARGET_DIR)/libwally-core-build/src/secp256k1/libsecp256k1.%: $(LIBWALLY_HEADERS) $(LIBSECP_HEADERS)
	cd external/libwally-core && ./tools/autogen.sh
	mkdir -p ${TARGET_DIR}/libwally-core-build
	cd ${TARGET_DIR}/libwally-core-build \
	&& PYTHON_VERSION=3 CFLAGS=-std=c99 ${TOP}/libwally-core/configure CC="$(CC)" \
		--enable-static=yes \
		$(CROSSCOMPILE_OPTS) \
		--enable-module-recovery \
		--enable-module-extrakeys \
		--enable-module-schnorrsig \
		--enable-elements \
		--enable-shared=no \
		--prefix=/ \
		--libdir=/ \
		--enable-debug \
	&& $(MAKE)

# If we tell Make that the above builds both, it runs it twice in
# parallel.  So we lie :(
external/jsmn/jsmn.c: external/jsmn/jsmn.h
	[ -f $@ ]

$(TARGET_DIR)/jsmn-build/jsmn.o: external/jsmn/jsmn.c Makefile
	@mkdir -p $(@D)
	$(COMPILE.c) -DJSMN_STRICT=1 -o $@ $<

$(TARGET_DIR)/libjsmn.a: $(TARGET_DIR)/jsmn-build/jsmn.o
	$(AR) rc $@ $<

${TARGET_DIR}/libnng.a: external/nng/include/nng/nng.h
	# If it's Linux, and cmake is not installed, get it temporarily for nng
	ifeq ($(UNAME_S),Linux)
		ifeq (, $(shell which cmake))
			$(shell mkdir $(TARGET_DIR)/cmake)
			$(shell wget https://cmake.org/files/v3.16/cmake-3.16.1-Linux-x86_64.sh)
			$(shell mv cmake-3.16.1-Linux-x86_64.sh $(TARGET_DIR)/cmake/)
			$(shell sh $(TARGET_DIR)/cmake/cmake-3.16.1-Linux-x86_64.sh --prefix=$(TARGET_DIR)/cmake --skip-license)
			# export PATH=$PATH:$(TARGET_DIR)/cmake/bin
			mkdir -p $(TARGET_DIR)/libnng-build
			cd $(TARGET_DIR)/libnng-build && $(TARGET_DIR)/cmake/bin/cmake -G Ninja $(TOP)/nng \
			&& ninja && cp -av libnng.a ..
		else
			mkdir -p $(TARGET_DIR)/libnng-build
			cd $(TARGET_DIR)/libnng-build && cmake -G Ninja $(TOP)/nng \
			&& ninja && cp -av libnng.a ..
		endif
	else
		mkdir -p $(TARGET_DIR)/libnng-build
		cd $(TARGET_DIR)/libnng-build && cmake -G Ninja $(TOP)/nng \
		&& ninja && cp -av libnng.a ..
	endif
	 

${TARGET_DIR}/libwebsockets.a: external/libwebsockets/include/libwebsockets.h
	mkdir -p $(TARGET_DIR)/libwebsockets-build
	cd $(TARGET_DIR)/libwebsockets-build && cmake -DLWS_WITH_HTTP2=1 $(TOP)/libwebsockets \
	&& $(MAKE) && cp -av lib/libwebsockets.a ..

distclean: external-distclean
clean: external-clean

external-clean:
	$(RM) $(EXTERNAL_LIBS_CLEAN) $(TARGET_DIR)/*.la $(TARGET_DIR)/*.o
	$(RM) $(TARGET_DIR)/jsmn-build/jsmn.o
	if [ -f ${TARGET_DIR}/libwally-core-build/Makefile ]; then make -C ${TARGET_DIR}/libwally-core-build clean; fi
	if [ -f ${TARGET_DIR}/libwally-core-build/src/Makefile ]; then make -C ${TARGET_DIR}/libwally-core-build/src clean; fi
	if [ -f ${TARGET_DIR}/libwebsockets-build/Makefile ]; then make -C ${TARGET_DIR}/libwebsockets-build clean; fi
	if [ -f ${TARGET_DIR}/libnng-build/Makefile ]; then make -C ${TARGET_DIR}/libnng-build clean; fi

external-distclean:
	$(RM) -rf ${TARGET_DIR}/libwally-core-build
	$(RM) -rf ${TARGET_DIR}/jsmn-build
	$(RM) -rf ${TARGET_DIR}/libwebsockets-build
	$(RM) -rf ${TARGET_DIR}/libnng-build
	$(RM) -r `git status --ignored --porcelain external/libwally-core | grep '^!! ' | cut -c3-`
	$(info $$TARGET_DIR is [${TARGET_DIR}])