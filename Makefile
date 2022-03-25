#! /usr/bin/make

# Extract version from git, or if we're from a zipfile, use dirname
VERSION=$(shell git describe --always --dirty=-modded --abbrev=7 2>/dev/null || pwd | sed -n 's|.*/c\{0,1\}bet-v\{0,1\}\([0-9a-f.rc\-]*\)$$|\1|gp')

ifeq ($(VERSION),)
$(error "ERROR: git is required for generating version information")
endif

# --quiet / -s means quiet, dammit!
ifeq ($(findstring s,$(word 1, $(MAKEFLAGS))),s)
ECHO := :
SUPPRESS_OUTPUT := > /dev/null
else
ECHO := echo
SUPPRESS_OUTPUT :=
endif

DISTRO=$(shell lsb_release -is 2>/dev/null || echo unknown)-$(shell lsb_release -rs 2>/dev/null || echo unknown)
PKGNAME = bet

SORT=LC_ALL=C sort

-include config.vars

ifeq ($V,1)
VERBOSE = $(ECHO) '$(2)'; $(2)
else
VERBOSE = $(ECHO) $(1); $(2)
endif

ifneq ($(VALGRIND),0)
VG=VALGRIND=1 valgrind -q --error-exitcode=7
VG_TEST_ARGS = --track-origins=yes --leak-check=full --show-reachable=yes --errors-for-leak-kinds=all
endif

SANITIZER_FLAGS :=

ifneq ($(ASAN),0)
SANITIZER_FLAGS += -fsanitize=address
endif

ifneq ($(UBSAN),0)
SANITIZER_FLAGS += -fsanitize=undefined
endif

ifneq ($(FUZZING), 0)
SANITIZER_FLAGS += -fsanitize=fuzzer-no-link
endif

ifeq ($(COVERAGE),1)
COVFLAGS = --coverage
endif

ifeq ($(PIE),1)
PIE_CFLAGS=-fPIE -fPIC
PIE_LDFLAGS=-pie
endif

ifeq ($(COMPAT),1)
# We support compatibility with pre-0.6.
COMPAT_CFLAGS=-DCOMPAT_V052=1 -DCOMPAT_V060=1 -DCOMPAT_V061=1 -DCOMPAT_V062=1 -DCOMPAT_V070=1 -DCOMPAT_V072=1 -DCOMPAT_V073=1 -DCOMPAT_V080=1 -DCOMPAT_V081=1 -DCOMPAT_V082=1 -DCOMPAT_V090=1
endif

# This is where we add new features as bitcoin adds them.
FEATURES :=

# These are filled by individual Makefiles
ALL_PROGRAMS :=
# ALL_TEST_PROGRAMS :=
ALL_FUZZ_TARGETS :=
ALL_C_SOURCES :=
ALL_C_HEADERS := header_versions_gen.h version_gen.h

CPPFLAGS +=
# -DBINTOPKGLIBEXECDIR="\"$(shell sh tools/rel.sh $(bindir) $(pkglibexecdir))\""
CFLAGS = $(CPPFLAGS) $(CWARNFLAGS) $(CDEBUGFLAGS) $(COPTFLAGS) $(EXTERNAL_INCLUDE_FLAGS) -I . -I/usr/local/include $(SQLITE3_CFLAGS) $(FEATURES) $(COVFLAGS) -DJSMN_PARENT_LINKS $(PIE_CFLAGS) $(COMPAT_CFLAGS)
# If CFLAGS is already set in the environment of make (to whatever value, it
# does not matter) then it would export it to subprocesses with the above value
# we set, including CWARNFLAGS which by default contains -Wall -Werror. This
# breaks at least libwally-core which tries to switch off some warnings with
# -Wno-whatever. So, tell make to not export our CFLAGS to subprocesses.
unexport CFLAGS

# We can get configurator to run a different compile cmd to cross-configure.
CONFIGURATOR_CC := $(CC)

LDFLAGS += $(PIE_LDFLAGS) $(SANITIZER_FLAGS) $(COPTFLAGS)
CFLAGS += $(SANITIZER_FLAGS)

ifeq ($(STATIC),1)
# For MacOS, Jacob Rapoport <jacob@rumblemonkey.com> changed this to:
#  -L/usr/local/lib -Wl,-lgmp -lsqlite3 -lz -Wl,-lm -lpthread -ldl $(COVFLAGS)
# But that doesn't static link.
LDLIBS = -L/usr/local/lib -Wl,-dn -lgmp $(SQLITE3_LDLIBS) -lz -Wl,-dy -lm -lpthread -ldl $(COVFLAGS)
else
LDLIBS = -L/usr/local/lib -lm -lgmp $(SQLITE3_LDLIBS) -lz $(COVFLAGS)
endif

default: show-flags all-programs

show-flags: config.vars
	@$(ECHO) "CC: $(CC) $(CFLAGS) -c -o"
	@$(ECHO) "LD: $(LINK.o) $(filter-out %.a,$^) $(LOADLIBES) $(EXTERNAL_LDLIBS) $(LDLIBS) -o"

# ccan/config.h: config.vars configure ccan/tools/configurator/configurator.c
# 	./configure --reconfigure

# config.vars:
# 	@echo 'File config.vars not found: you must run ./configure before running make.' >&2
# 	@exit 1

%.o: %.c
	@$(call VERBOSE, "cc $<", $(CC) $(CFLAGS) -c -o $@ $<)

# Git doesn't maintain timestamps, so we only regen if
# We place the SHA inside some generated files so we can tell if they need updating.
# Usage: $(call SHA256STAMP_CHANGED)
SHA256STAMP_CHANGED = [ x"`sed -n 's/.*SHA256STAMP://p' $@ 2>/dev/null`" != x"`cat $(sort $(filter-out FORCE,$^)) | $(SHA256SUM) | cut -c1-64`" ]
# Usage: $(call SHA256STAMP,commentprefix)
SHA256STAMP = echo '$(1) SHA256STAMP:'`cat $(sort $(filter-out FORCE,$^)) | $(SHA256SUM) | cut -c1-64` >> $@

include external/Makefile

# We make pretty much everything depend on these.
ALL_GEN_HEADERS := $(filter %gen.h,$(ALL_C_HEADERS))
ALL_GEN_SOURCES := $(filter %gen.c,$(ALL_C_SOURCES))
ALL_NONGEN_HEADERS := $(filter-out %gen.h,$(ALL_C_HEADERS))
ALL_NONGEN_SOURCES := $(filter-out %gen.c,$(ALL_C_SOURCES))
ALL_NONGEN_SRCFILES := $(ALL_NONGEN_HEADERS) $(ALL_NONGEN_SOURCES)

# Programs to install in bindir and pkglibexecdir.
# TODO: $(EXEEXT) support for Windows?  Needs more coding for
# the individual Makefiles, however.
BIN_PROGRAMS = 
PKGLIBEXEC_PROGRAMS = 

# Don't delete these intermediaries.
.PRECIOUS: $(ALL_GEN_HEADERS) $(ALL_GEN_SOURCES)

# Every single object file.
ALL_OBJS := $(ALL_C_SOURCES:.c=.o)

check-units:

check: check-units installcheck

# Keep includes in alpha order.
check-src-include-order/%: %
	@if [ "$$(grep '^#include' < $<)" != "$$(grep '^#include' < $< | $(SORT))" ]; then echo "$<:1: includes out of order"; grep '^#include' < $<; echo VERSUS; grep '^#include' < $< | $(SORT); exit 1; fi

# Keep includes in alpha order, after including "config.h"
check-hdr-include-order/%: %
	@if [ "$$(grep '^#include' < $< | head -n1)" != '#include "config.h"' ]; then echo "$<:1: doesn't include config.h first"; exit 1; fi
	@if [ "$$(grep '^#include' < $< | tail -n +2)" != "$$(grep '^#include' < $< | tail -n +2 | $(SORT))" ]; then echo "$<:1: includes out of order"; exit 1; fi

# Make sure Makefile includes all headers.
# check-makefile:
# 	@if [ x"$(CCANDIR)/config.h `find $(CCANDIR)/ccan -name '*.h' | grep -v /test/ | $(SORT) | tr '\n' ' '`" != x"$(CCAN_HEADERS) " ]; then echo CCAN_HEADERS incorrect; exit 1; fi

# We exclude test files, which need to do weird include tricks!
SRC_TO_CHECK := $(ALL_NONGEN_SOURCES)
check-src-includes: $(SRC_TO_CHECK:%=check-src-include-order/%)
check-hdr-includes: $(ALL_NONGEN_HEADERS:%=check-hdr-include-order/%)

# cppcheck gets confused by list_for_each(head, i, list): thinks i is uninit.
# .cppcheck-suppress:
# 	@git ls-files -- "*.c" "*.h" | grep -vE '^ccan/' | xargs grep -n '_for_each' | sed 's/\([^:]*:.*\):.*/uninitvar:\1/' > $@

# check-cppcheck: .cppcheck-suppress
# 	@trap 'rm -f .cppcheck-suppress' 0; git ls-files -- "*.c" "*.h" | grep -vE '^ccan/' | xargs cppcheck -q --language=c --std=c11 --error-exitcode=1 --suppressions-list=.cppcheck-suppress --inline-suppr

check-shellcheck:
	@git ls-files -- "*.sh" | xargs shellcheck

# check-tmpctx:
# 	@if git grep -n 'tal_free[(]tmpctx)' | grep -Ev '^ccan/|/test/|^common/setup.c:|^common/utils.c:'; then echo "Don't free tmpctx!">&2; exit 1; fi

# check-discouraged-functions:
# 	@if git grep -E "[^a-z_/](fgets|fputs|gets|scanf|sprintf)\(" -- "*.c" "*.h" ":(exclude)ccan/"; then exit 1; fi

check-source: check-makefile check-cppcheck check-shellcheck check-tmpctx check-discouraged-functions

full-check: check check-source

# Simple target to be used on CI systems to check that all the derived
# files were checked in and updated. It depends on the generated
# targets, and checks if any of the tracked files changed. If they did
# then one of the gen-targets caused this change, meaning either the
# gen-target is not reproducible or the files were forgotten.
#
# Do not run on your development tree since it will complain if you
# have a dirty tree.
check-gen-updated: $(ALL_GEN_HEADERS) $(ALL_GEN_SOURCES)
	@echo "Checking for generated files being changed by make"
	git diff --exit-code HEAD $?

coverage/coverage.info: check pytest
	mkdir coverage || true
	lcov --capture --directory . --output-file coverage/coverage.info

coverage: coverage/coverage.info
	genhtml coverage/coverage.info --output-directory coverage

# We make libwallycore.la a dependency, so that it gets built normally, without ncc.
# Ncc can't handle the libwally source code (yet).

# Ignore test/ directories.
TAGS: FORCE
	$(RM) TAGS; find * -name test -type d -prune -o -name '*.[ch]' -print -o -name '*.py' -print | xargs etags --append
FORCE::

# [TODO]: FIXME 'Hack to get around this bug'
ALL_PROGRAMS += foo
# Can't add to ALL_OBJS, as that makes a circular dep.

version_gen.h: FORCE
	@(echo "#define VERSION \"$(VERSION)\"" && echo "#define BUILD_FEATURES \"$(FEATURES)\"") > $@.new
	@if cmp $@.new $@ >/dev/null 2>&1; then rm -f $@.new; else mv $@.new $@; $(ECHO) Version updated; fi

# All binaries require the external libs, ccan and system library versions.
$(ALL_PROGRAMS) $(ALL_FUZZ_TARGETS): $(EXTERNAL_LIBS)

# Without this rule, the (built-in) link line contains
# external/libwallycore.a directly, which causes a symbol clash (it
# uses some ccan modules internally).  We want to rely on -lwallycore etc.
# (as per EXTERNAL_LDLIBS) so we filter them out here.
# $(ALL_PROGRAMS):
# 	@$(call VERBOSE, "ld $@", $(LINK.o) $(filter-out %.a,$^) $(LOADLIBES) $(EXTERNAL_LDLIBS) $(LDLIBS) -o $@)

# We special case the fuzzing target binaries, as they need to link against libfuzzer,
# which brings its own main().
FUZZ_LDFLAGS = -fsanitize=fuzzer
$(ALL_FUZZ_TARGETS):
	@$(call VERBOSE, "ld $@", $(LINK.o) $(filter-out %.a,$^) $(LOADLIBES) $(EXTERNAL_LDLIBS) $(LDLIBS) $(FUZZ_LDFLAGS) -o $@)


# Everything depends on the CCAN headers, and Makefile
# $(CCAN_OBJS) $(CDUMP_OBJS): $(CCAN_HEADERS) Makefile

# Except for CCAN, we treat everything else as dependent on external/ bitcoin/ common/ wire/ and all generated headers, and Makefile
$(ALL_OBJS): $(ALL_GEN_HEADERS) $(EXTERNAL_HEADERS) Makefile

# Now ALL_PROGRAMS is fully populated, we can expand it.
build_dep:
	make --directory crypto777
	# make --directory external/jsmn
build_dep1:
	make --directory privatebet
all-programs: $(ALL_PROGRAMS) build_dep build_dep1
all-test-programs: $(ALL_FUZZ_TARGETS)

distclean: clean
	make --directory external/ distclean

maintainer-clean: distclean
	@echo 'This command is intended for maintainers to use; it'
	@echo 'deletes files that may need special tools to rebuild.'
	$(RM) $(ALL_GEN_HEADERS) $(ALL_GEN_SOURCES)

# We used to have gen_ files, now we have _gen files.
obsclean:
	$(RM) gen_*.h */gen_*.[ch] */*/gen_*.[ch]

clean: obsclean
	$(RM) $(CDUMP_OBJS) $(ALL_OBJS)
	$(RM) $(ALL_PROGRAMS)
	$(RM) $(ALL_FUZZ_TARGETS)
	make --directory privatebet clean
	make --directory external/jsmn clean
	make --directory external/libwally-core clean
	make --directory crypto777 clean
	find . -name '*gcda' -delete
	find . -name '*gcno' -delete
	find . -name '*.nccout' -delete

# Installation directories
exec_prefix = $(PREFIX)
bindir = $(exec_prefix)/bin
libexecdir = $(exec_prefix)/libexec
pkglibexecdir = $(libexecdir)/$(PKGNAME)
plugindir = $(pkglibexecdir)/plugins
datadir = $(PREFIX)/share
docdir = $(datadir)/doc/$(PKGNAME)
mandir = $(datadir)/man
man1dir = $(mandir)/man1
man5dir = $(mandir)/man5
man7dir = $(mandir)/man7
man8dir = $(mandir)/man8

# Commands
MKDIR_P = mkdir -p
INSTALL = install
INSTALL_PROGRAM = $(INSTALL)
INSTALL_DATA = $(INSTALL) -m 644

# Tags needed by some package systems.
PRE_INSTALL = :
NORMAL_INSTALL = :
POST_INSTALL = :
PRE_UNINSTALL = :
NORMAL_UNINSTALL = :
POST_UNINSTALL = :

# Target to create directories.
installdirs:
	@$(NORMAL_INSTALL)
	$(MKDIR_P) $(DESTDIR)$(bindir)
	$(MKDIR_P) $(DESTDIR)$(pkglibexecdir)
	$(MKDIR_P) $(DESTDIR)$(plugindir)
	$(MKDIR_P) $(DESTDIR)$(man1dir)
	$(MKDIR_P) $(DESTDIR)$(man5dir)
	$(MKDIR_P) $(DESTDIR)$(man7dir)
	$(MKDIR_P) $(DESTDIR)$(man8dir)
	$(MKDIR_P) $(DESTDIR)$(docdir)

# $(PLUGINS) is defined in plugins/Makefile.

install-program: installdirs $(BIN_PROGRAMS) $(PKGLIBEXEC_PROGRAMS) $(PLUGINS)
	@$(NORMAL_INSTALL)
	$(INSTALL_PROGRAM) $(BIN_PROGRAMS) $(DESTDIR)$(bindir)
	$(INSTALL_PROGRAM) $(PKGLIBEXEC_PROGRAMS) $(DESTDIR)$(pkglibexecdir)
	[ -z "$(PLUGINS)" ] || $(INSTALL_PROGRAM) $(PLUGINS) $(DESTDIR)$(plugindir)

MAN1PAGES = $(filter %.1,$(MANPAGES))
MAN5PAGES = $(filter %.5,$(MANPAGES))
MAN7PAGES = $(filter %.7,$(MANPAGES))
MAN8PAGES = $(filter %.8,$(MANPAGES))
DOC_DATA = README.md doc/INSTALL.md doc/HACKING.md LICENSE

install-data: installdirs $(MAN1PAGES) $(MAN5PAGES) $(MAN7PAGES) $(MAN8PAGES) $(DOC_DATA)
	@$(NORMAL_INSTALL)
	$(INSTALL_DATA) $(MAN1PAGES) $(DESTDIR)$(man1dir)
	$(INSTALL_DATA) $(MAN5PAGES) $(DESTDIR)$(man5dir)
	$(INSTALL_DATA) $(MAN7PAGES) $(DESTDIR)$(man7dir)
	$(INSTALL_DATA) $(MAN8PAGES) $(DESTDIR)$(man8dir)
	$(INSTALL_DATA) $(DOC_DATA) $(DESTDIR)$(docdir)

install: install-program install-data

uninstall:
	@$(NORMAL_UNINSTALL)
	@for f in $(BIN_PROGRAMS); do \
	  $(ECHO) rm -f $(DESTDIR)$(bindir)/`basename $$f`; \
	  rm -f $(DESTDIR)$(bindir)/`basename $$f`; \
	done
	@for f in $(PLUGINS); do \
	  $(ECHO) rm -f $(DESTDIR)$(plugindir)/`basename $$f`; \
	  rm -f $(DESTDIR)$(plugindir)/`basename $$f`; \
	done
	@for f in $(PKGLIBEXEC_PROGRAMS); do \
	  $(ECHO) rm -f $(DESTDIR)$(pkglibexecdir)/`basename $$f`; \
	  rm -f $(DESTDIR)$(pkglibexecdir)/`basename $$f`; \
	done
	@for f in $(MAN1PAGES); do \
	  $(ECHO) rm -f $(DESTDIR)$(man1dir)/`basename $$f`; \
	  rm -f $(DESTDIR)$(man1dir)/`basename $$f`; \
	done
	@for f in $(MAN5PAGES); do \
	  $(ECHO) rm -f $(DESTDIR)$(man5dir)/`basename $$f`; \
	  rm -f $(DESTDIR)$(man5dir)/`basename $$f`; \
	done
	@for f in $(MAN7PAGES); do \
	  $(ECHO) rm -f $(DESTDIR)$(man7dir)/`basename $$f`; \
	  rm -f $(DESTDIR)$(man7dir)/`basename $$f`; \
	done
	@for f in $(MAN8PAGES); do \
	  $(ECHO) rm -f $(DESTDIR)$(man8dir)/`basename $$f`; \
	  rm -f $(DESTDIR)$(man8dir)/`basename $$f`; \
	done
	@for f in $(DOC_DATA); do \
	  $(ECHO) rm -f $(DESTDIR)$(docdir)/`basename $$f`; \
	  rm -f $(DESTDIR)$(docdir)/`basename $$f`; \
	done

installcheck: all-programs
	@rm -rf testinstall || true
	$(MAKE) DESTDIR=$$(pwd)/testinstall install
	testinstall$(bindir)/lightningd --test-daemons-only --lightning-dir=testinstall
	$(MAKE) DESTDIR=$$(pwd)/testinstall uninstall
	@if test `find testinstall '!' -type d | wc -l` -ne 0; then \
		echo 'make uninstall left some files in testinstall directory!'; \
		exit 1; \
	fi
	@rm -rf testinstall || true

.PHONY: installdirs install-program install-data install uninstall \
	installcheck ncc bin-tarball show-flags

# Make a tarball of opt/clightning/, optionally with label for distribution.
bin-tarball: clightning-$(VERSION)-$(DISTRO).tar.xz
clightning-$(VERSION)-$(DISTRO).tar.xz: DESTDIR=$(shell pwd)/
clightning-$(VERSION)-$(DISTRO).tar.xz: prefix=opt/clightning
clightning-$(VERSION)-$(DISTRO).tar.xz: install
	trap "rm -rf opt" 0; tar cvfa $@ opt/

