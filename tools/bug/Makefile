ifeq ("$(origin O)", "command line")
	OUTPUT := $(O)/
endif

# The default target of this Makefile is...
all:

include config/utilities.mak

ifneq ($(OUTPUT),)
# check that the output directory actually exists
OUTDIR := $(shell cd $(OUTPUT) && /bin/pwd)
$(if $(OUTDIR),, $(error output directory "$(OUTPUT)" does not exist))
endif

# Define V to have a more verbose compile.
#
# Define PYTHON to point to the python binary if the default
# `python' is not correct; for example: PYTHON=python2
#
# Define PYTHON_CONFIG to point to the python-config binary if
# the default `$(PYTHON)-config' is not correct.
#
# Define ASCIIDOC8 if you want to format documentation with AsciiDoc 8
#
# Define DOCBOOK_XSL_172 if you want to format man pages with DocBook XSL v1.72.
#
# Define LDFLAGS=-static to build a static binary.
#
# Define EXTRA_CFLAGS=-m64 or EXTRA_CFLAGS=-m32 as appropriate for cross-builds.
#
# Define WERROR=0 to disable treating any warnings as errors.

$(OUTPUT)PERF-VERSION-FILE: .FORCE-PERF-VERSION-FILE
	@$(SHELL_PATH) util/PERF-VERSION-GEN $(OUTPUT)
-include $(OUTPUT)PERF-VERSION-FILE

uname_M := $(shell uname -m 2>/dev/null || echo not)

ARCH ?= $(shell echo $(uname_M) | sed -e s/i.86/i386/ -e s/sun4u/sparc64/ \
				  -e s/arm.*/arm/ -e s/sa110/arm/ \
				  -e s/s390x/s390/ -e s/parisc64/parisc/ \
				  -e s/ppc.*/powerpc/ -e s/mips.*/mips/ \
				  -e s/sh[234].*/sh/ )

CC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar

# Additional ARCH settings for x86
ifeq ($(ARCH),i386)
        ARCH := x86
endif
ifeq ($(ARCH),x86_64)
	ARCH := x86
	IS_X86_64 := 0
	ifeq (, $(findstring m32,$(EXTRA_CFLAGS)))
		IS_X86_64 := $(shell echo __x86_64__ | ${CC} -E -xc - | tail -n 1)
	endif
	ifeq (${IS_X86_64}, 1)
		RAW_ARCH := x86_64
		ARCH_CFLAGS := -DARCH_X86_64
		ARCH_INCLUDE = ../../arch/x86/lib/memcpy_64.S
	endif
endif

# Treat warnings as errors unless directed not to
ifneq ($(WERROR),0)
	CFLAGS_WERROR := -Werror
endif

#
# Include saner warnings here, which can catch bugs:
#

EXTRA_WARNINGS := -Wformat
EXTRA_WARNINGS := $(EXTRA_WARNINGS) -Wformat-security
EXTRA_WARNINGS := $(EXTRA_WARNINGS) -Wformat-y2k
EXTRA_WARNINGS := $(EXTRA_WARNINGS) -Wshadow
EXTRA_WARNINGS := $(EXTRA_WARNINGS) -Winit-self
EXTRA_WARNINGS := $(EXTRA_WARNINGS) -Wpacked
EXTRA_WARNINGS := $(EXTRA_WARNINGS) -Wredundant-decls
EXTRA_WARNINGS := $(EXTRA_WARNINGS) -Wstrict-aliasing=3
EXTRA_WARNINGS := $(EXTRA_WARNINGS) -Wswitch-default
EXTRA_WARNINGS := $(EXTRA_WARNINGS) -Wswitch-enum
EXTRA_WARNINGS := $(EXTRA_WARNINGS) -Wno-system-headers
EXTRA_WARNINGS := $(EXTRA_WARNINGS) -Wundef
EXTRA_WARNINGS := $(EXTRA_WARNINGS) -Wwrite-strings
EXTRA_WARNINGS := $(EXTRA_WARNINGS) -Wbad-function-cast
EXTRA_WARNINGS := $(EXTRA_WARNINGS) -Wmissing-declarations
EXTRA_WARNINGS := $(EXTRA_WARNINGS) -Wmissing-prototypes
EXTRA_WARNINGS := $(EXTRA_WARNINGS) -Wnested-externs
EXTRA_WARNINGS := $(EXTRA_WARNINGS) -Wold-style-definition
EXTRA_WARNINGS := $(EXTRA_WARNINGS) -Wstrict-prototypes
EXTRA_WARNINGS := $(EXTRA_WARNINGS) -Wdeclaration-after-statement

ifeq ("$(origin DEBUG)", "command line")
  PERF_DEBUG = $(DEBUG)
endif
ifndef PERF_DEBUG
  CFLAGS_OPTIMIZE = -O6
endif

CFLAGS = -fno-omit-frame-pointer -ggdb3 -Wall -Wextra -std=gnu99 $(CFLAGS_WERROR) $(CFLAGS_OPTIMIZE) -D_FORTIFY_SOURCE=2 $(EXTRA_WARNINGS) $(EXTRA_CFLAGS)
EXTLIBS = -lpthread -lrt -lelf -lm
ALL_CFLAGS = $(CFLAGS) -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64
ALL_LDFLAGS = $(LDFLAGS)
STRIP ?= strip

# Among the variables below, these:
#   bugexecdir
#   template_dir
#   mandir
#   infodir
#   htmldir
#   ETC_PERFCONFIG (but not sysconfdir)
# can be specified as a relative path some/where/else;
# this is interpreted as relative to $(prefix) and "bug" at
# runtime figures out where they are based on the path to the executable.
# This can help installing the suite in a relocatable way.

# Make the path relative to DESTDIR, not to prefix
ifndef DESTDIR
prefix = $(HOME)
endif
bindir_relative = bin
bindir = $(prefix)/$(bindir_relative)
mandir = share/man
infodir = share/info
bugexecdir = libexec/bug-core
sharedir = $(prefix)/share
template_dir = share/bug-core/templates
htmldir = share/doc/bug-doc
ifeq ($(prefix),/usr)
sysconfdir = /etc
ETC_PERFCONFIG = $(sysconfdir)/bugconfig
else
sysconfdir = $(prefix)/etc
ETC_PERFCONFIG = etc/bugconfig
endif
lib = lib

export prefix bindir sharedir sysconfdir

RM = rm -f
MKDIR = mkdir
FIND = find
INSTALL = install

# sparse is architecture-neutral, which means that we need to tell it
# explicitly what architecture to check for. Fix this up for yours..
SPARSE_FLAGS = -D__BIG_ENDIAN__ -D__powerpc__

-include config/feature-tests.mak

ifeq ($(call try-cc,$(SOURCE_HELLO),-Werror -fstack-protector-all),y)
	CFLAGS := $(CFLAGS) -fstack-protector-all
endif

ifeq ($(call try-cc,$(SOURCE_HELLO),-Werror -Wstack-protector),y)
       CFLAGS := $(CFLAGS) -Wstack-protector
endif

ifeq ($(call try-cc,$(SOURCE_HELLO),-Werror -Wvolatile-register-var),y)
       CFLAGS := $(CFLAGS) -Wvolatile-register-var
endif

### --- END CONFIGURATION SECTION ---

# Those must not be GNU-specific; they are shared with perl/ which may
# be built by a different compiler. (Note that this is an artifact now
# but it still might be nice to keep that distinction.)
BASIC_CFLAGS = -Iutil/include -Iarch/$(ARCH)/include
BASIC_LDFLAGS =

# Guard against environment variables
BUILTIN_OBJS =
LIB_H =
LIB_OBJS =
PYRF_OBJS =
SCRIPT_SH =

grep-libs = $(filter -l%,$(1))
strip-libs = $(filter-out -l%,$(1))

$(OUTPUT)python/bug.so: $(PYRF_OBJS)
	$(QUIET_GEN)CFLAGS='$(BASIC_CFLAGS)' $(PYTHON_WORD) util/setup.py \
	  --quiet build_ext; \
	mkdir -p $(OUTPUT)python && \
	cp $(PYTHON_EXTBUILD_LIB)bug.so $(OUTPUT)python/
#
# No Perl scripts right now:
#

SCRIPTS = $(patsubst %.sh,%,$(SCRIPT_SH))

#
# Single 'bug' binary right now:
#
PROGRAMS += $(OUTPUT)bug

LANG_BINDINGS =

# what 'all' will build and 'install' will install, in bugexecdir
ALL_PROGRAMS = $(PROGRAMS) $(SCRIPTS)

# what 'all' will build but not install in bugexecdir
OTHER_PROGRAMS = $(OUTPUT)bug

# Set paths to tools early so that they can be used for version tests.
ifndef SHELL_PATH
	SHELL_PATH = /bin/sh
endif
ifndef PERL_PATH
	PERL_PATH = /usr/bin/perl
endif

export PERL_PATH

LIB_FILE=$(OUTPUT)libbug.a

LIB_H += ../../include/linux/rbtree.h
LIB_H += ../../include/linux/list.h
LIB_H += ../../include/linux/const.h
LIB_H += ../../include/linux/hash.h
LIB_H += ../../include/linux/stringify.h
LIB_H += util/include/linux/compiler.h
LIB_H += util/include/linux/const.h
LIB_H += util/include/linux/ctype.h
LIB_H += util/include/linux/kernel.h
LIB_H += util/include/linux/list.h
LIB_H += util/include/linux/module.h
LIB_H += util/include/linux/poison.h
LIB_H += util/include/linux/prefetch.h
LIB_H += util/include/linux/rbtree.h
LIB_H += util/include/linux/string.h
LIB_H += util/include/linux/types.h
LIB_H += util/include/linux/linkage.h
LIB_H += util/include/asm/asm-offsets.h
LIB_H += util/include/asm/bug.h
LIB_H += util/include/asm/byteorder.h
LIB_H += util/include/asm/hweight.h
LIB_H += util/include/asm/swab.h
LIB_H += util/include/asm/system.h
LIB_H += util/include/asm/uaccess.h
LIB_H += util/include/asm/cpufeature.h
LIB_H += bug.h
LIB_H += util/cache.h
LIB_H += util/debug.h
LIB_H += util/debugfs.h
LIB_H += util/exec_cmd.h
LIB_H += util/types.h
LIB_H += util/levenshtein.h
LIB_H += util/parse-options.h
LIB_H += util/quote.h
LIB_H += util/util.h
LIB_H += util/help.h
LIB_H += util/strbuf.h
LIB_H += util/strlist.h
LIB_H += util/strfilter.h
LIB_H += util/tool.h
LIB_H += util/run-command.h
LIB_H += util/sigchain.h
LIB_H += util/color.h
LIB_H += util/values.h
LIB_H += $(ARCH_INCLUDE)

LIB_OBJS += $(OUTPUT)util/abspath.o
LIB_OBJS += $(OUTPUT)util/alias.o
LIB_OBJS += $(OUTPUT)util/config.o
LIB_OBJS += $(OUTPUT)util/ctype.o
LIB_OBJS += $(OUTPUT)util/debugfs.o
LIB_OBJS += $(OUTPUT)util/environment.o
LIB_OBJS += $(OUTPUT)util/exec_cmd.o
LIB_OBJS += $(OUTPUT)util/help.o
LIB_OBJS += $(OUTPUT)util/levenshtein.o
LIB_OBJS += $(OUTPUT)util/parse-options.o
LIB_OBJS += $(OUTPUT)util/path.o
LIB_OBJS += $(OUTPUT)util/rbtree.o
LIB_OBJS += $(OUTPUT)util/hweight.o
LIB_OBJS += $(OUTPUT)util/run-command.o
LIB_OBJS += $(OUTPUT)util/quote.o
LIB_OBJS += $(OUTPUT)util/strbuf.o
LIB_OBJS += $(OUTPUT)util/string.o
LIB_OBJS += $(OUTPUT)util/strlist.o
LIB_OBJS += $(OUTPUT)util/strfilter.o
LIB_OBJS += $(OUTPUT)util/usage.o
LIB_OBJS += $(OUTPUT)util/wrapper.o
LIB_OBJS += $(OUTPUT)util/sigchain.o
LIB_OBJS += $(OUTPUT)util/color.o
LIB_OBJS += $(OUTPUT)util/pager.o
LIB_OBJS += $(OUTPUT)util/values.o
LIB_OBJS += $(OUTPUT)util/debug.o
LIB_OBJS += $(OUTPUT)util/util.o

BUILTIN_OBJS += $(OUTPUT)builtin-help.o
BUILTIN_OBJS += $(OUTPUT)builtin-test.o

PERFLIBS = $(LIB_FILE)

#
# Platform specific tweaks
#

# We choose to avoid "if .. else if .. else .. endif endif"
# because maintaining the nesting to match is a pain.  If
# we had "elif" things would have been much nicer...

-include config.mak.autogen
-include config.mak

#-include arch/$(ARCH)/Makefile

ifneq ($(OUTPUT),)
	BASIC_CFLAGS += -I$(OUTPUT)
endif

FLAGS_LIBELF=$(ALL_CFLAGS) $(ALL_LDFLAGS) $(EXTLIBS)
ifneq ($(call try-cc,$(SOURCE_LIBELF),$(FLAGS_LIBELF)),y)
	FLAGS_GLIBC=$(ALL_CFLAGS) $(ALL_LDFLAGS)
	ifneq ($(call try-cc,$(SOURCE_GLIBC),$(FLAGS_GLIBC)),y)
		msg := $(error No gnu/libc-version.h found, please install glibc-dev[el]/glibc-static);
	else
		msg := $(error No libelf.h/libelf found, please install libelf-dev/elfutils-libelf-devel);
	endif
endif

ifneq ($(call try-cc,$(SOURCE_ELF_MMAP),$(FLAGS_COMMON)),y)
	BASIC_CFLAGS += -DLIBELF_NO_MMAP
endif

ifdef NO_STRLCPY
	BASIC_CFLAGS += -DNO_STRLCPY
else
	ifneq ($(call try-cc,$(SOURCE_STRLCPY),),y)
		BASIC_CFLAGS += -DNO_STRLCPY
	endif
endif

ifneq ($(findstring $(MAKEFLAGS),s),s)
ifndef V
	QUIET_CC       = @echo '   ' CC $@;
	QUIET_AR       = @echo '   ' AR $@;
	QUIET_LINK     = @echo '   ' LINK $@;
	QUIET_MKDIR    = @echo '   ' MKDIR $@;
	QUIET_GEN      = @echo '   ' GEN $@;
endif
endif

ifdef ASCIIDOC8
	export ASCIIDOC8
endif

# Shell quote (do not use $(call) to accommodate ancient setups);

ETC_PERFCONFIG_SQ = $(subst ','\'',$(ETC_PERFCONFIG))

DESTDIR_SQ = $(subst ','\'',$(DESTDIR))
bindir_SQ = $(subst ','\'',$(bindir))
bindir_relative_SQ = $(subst ','\'',$(bindir_relative))
mandir_SQ = $(subst ','\'',$(mandir))
infodir_SQ = $(subst ','\'',$(infodir))
bugexecdir_SQ = $(subst ','\'',$(bugexecdir))
template_dir_SQ = $(subst ','\'',$(template_dir))
htmldir_SQ = $(subst ','\'',$(htmldir))
prefix_SQ = $(subst ','\'',$(prefix))

SHELL_PATH_SQ = $(subst ','\'',$(SHELL_PATH))

LIBS = -Wl,--whole-archive $(PERFLIBS) -Wl,--no-whole-archive -Wl,--start-group $(EXTLIBS) -Wl,--end-group

ALL_CFLAGS += $(BASIC_CFLAGS)
ALL_CFLAGS += $(ARCH_CFLAGS)
ALL_LDFLAGS += $(BASIC_LDFLAGS)

export INSTALL SHELL_PATH


### Build rules

SHELL = $(SHELL_PATH)

all: shell_compatibility_test $(ALL_PROGRAMS) $(LANG_BINDINGS) $(OTHER_PROGRAMS)

please_set_SHELL_PATH_to_a_more_modern_shell:
	@$$(:)

shell_compatibility_test: please_set_SHELL_PATH_to_a_more_modern_shell

strip: $(PROGRAMS) $(OUTPUT)bug
	$(STRIP) $(STRIP_OPTS) $(PROGRAMS) $(OUTPUT)bug

$(OUTPUT)bug.o: bug.c $(OUTPUT)common-cmds.h $(OUTPUT)PERF-CFLAGS
	$(QUIET_CC)$(CC) -DPERF_VERSION='"$(PERF_VERSION)"' \
		'-DPERF_HTML_PATH="$(htmldir_SQ)"' \
		$(ALL_CFLAGS) -c $(filter %.c,$^) -o $@

$(OUTPUT)bug: $(OUTPUT)bug.o $(BUILTIN_OBJS) $(PERFLIBS)
	$(QUIET_LINK)$(CC) $(ALL_CFLAGS) $(ALL_LDFLAGS) $(OUTPUT)bug.o \
               $(BUILTIN_OBJS) $(LIBS) -o $@

$(OUTPUT)builtin-help.o: builtin-help.c $(OUTPUT)common-cmds.h $(OUTPUT)PERF-CFLAGS
	$(QUIET_CC)$(CC) -o $@ -c $(ALL_CFLAGS) \
		'-DPERF_HTML_PATH="$(htmldir_SQ)"' \
		'-DPERF_MAN_PATH="$(mandir_SQ)"' \
		'-DPERF_INFO_PATH="$(infodir_SQ)"' $<

$(OUTPUT)common-cmds.h: util/generate-cmdlist.sh command-list.txt

$(OUTPUT)common-cmds.h: $(wildcard Documentation/bug-*.txt)
	$(QUIET_GEN). util/generate-cmdlist.sh > $@+ && mv $@+ $@

$(SCRIPTS) : % : %.sh
	$(QUIET_GEN)$(INSTALL) '$@.sh' '$(OUTPUT)$@'

# These can record PERF_VERSION
$(OUTPUT)bug.o bug.spec \
	$(SCRIPTS) \
	: $(OUTPUT)PERF-VERSION-FILE

$(OUTPUT)%.o: %.c $(OUTPUT)PERF-CFLAGS
	$(QUIET_CC)$(CC) -o $@ -c $(ALL_CFLAGS) $<
$(OUTPUT)%.s: %.c $(OUTPUT)PERF-CFLAGS
	$(QUIET_CC)$(CC) -S $(ALL_CFLAGS) $<
$(OUTPUT)%.o: %.S
	$(QUIET_CC)$(CC) -o $@ -c $(ALL_CFLAGS) $<

$(OUTPUT)util/exec_cmd.o: util/exec_cmd.c $(OUTPUT)PERF-CFLAGS
	$(QUIET_CC)$(CC) -o $@ -c $(ALL_CFLAGS) \
		'-DPERF_EXEC_PATH="$(bugexecdir_SQ)"' \
		'-DBINDIR="$(bindir_relative_SQ)"' \
		'-DPREFIX="$(prefix_SQ)"' \
		$<

$(OUTPUT)util/config.o: util/config.c $(OUTPUT)PERF-CFLAGS
	$(QUIET_CC)$(CC) -o $@ -c $(ALL_CFLAGS) -DETC_PERFCONFIG='"$(ETC_PERFCONFIG_SQ)"' $<

$(OUTPUT)util/rbtree.o: ../../lib/rbtree.c $(OUTPUT)PERF-CFLAGS
	$(QUIET_CC)$(CC) -o $@ -c $(ALL_CFLAGS) -DETC_PERFCONFIG='"$(ETC_PERFCONFIG_SQ)"' $<

$(OUTPUT)bug-%: %.o $(PERFLIBS)
	$(QUIET_LINK)$(CC) $(ALL_CFLAGS) -o $@ $(ALL_LDFLAGS) $(filter %.o,$^) $(LIBS)

$(LIB_OBJS) $(BUILTIN_OBJS): $(LIB_H)
$(patsubst bug-%,%.o,$(PROGRAMS)): $(LIB_H) $(wildcard */*.h)

# we compile into subdirectories. if the target directory is not the source directory, they might not exists. So
# we depend the various files onto their directories.
DIRECTORY_DEPS = $(LIB_OBJS) $(BUILTIN_OBJS) $(OUTPUT)PERF-VERSION-FILE $(OUTPUT)common-cmds.h
$(DIRECTORY_DEPS): | $(sort $(dir $(DIRECTORY_DEPS)))
# In the second step, we make a rule to actually create these directories
$(sort $(dir $(DIRECTORY_DEPS))):
	$(QUIET_MKDIR)$(MKDIR) -p $@ 2>/dev/null

$(LIB_FILE): $(LIB_OBJS)
	$(QUIET_AR)$(RM) $@ && $(AR) rcs $@ $(LIB_OBJS)

help:
	@echo 'Perf make targets:'
	@echo '  doc		- make *all* documentation (see below)'
	@echo '  man		- make manpage documentation (access with man <foo>)'
	@echo '  html		- make html documentation'
	@echo '  info		- make GNU info documentation (access with info <foo>)'
	@echo '  pdf		- make pdf documentation'
	@echo '  TAGS		- use etags to make tag information for source browsing'
	@echo '  tags		- use ctags to make tag information for source browsing'
	@echo '  cscope	- use cscope to make interactive browsing database'
	@echo ''
	@echo 'Perf install targets:'
	@echo '  NOTE: documentation build requires asciidoc, xmlto packages to be installed'
	@echo '  HINT: use "make prefix=<path> <install target>" to install to a particular'
	@echo '        path like make prefix=/usr/local install install-doc'
	@echo '  install	- install compiled binaries'
	@echo '  install-doc	- install *all* documentation'
	@echo '  install-man	- install manpage documentation'
	@echo '  install-html	- install html documentation'
	@echo '  install-info	- install GNU info documentation'
	@echo '  install-pdf	- install pdf documentation'
	@echo ''
	@echo '  quick-install-doc	- alias for quick-install-man'
	@echo '  quick-install-man	- install the documentation quickly'
	@echo '  quick-install-html	- install the html documentation quickly'
	@echo ''
	@echo 'Perf maintainer targets:'
	@echo '  distclean		- alias to clean'
	@echo '  clean			- clean all binary objects and build output'

doc:
	$(MAKE) -C Documentation all

man:
	$(MAKE) -C Documentation man

html:
	$(MAKE) -C Documentation html

info:
	$(MAKE) -C Documentation info

pdf:
	$(MAKE) -C Documentation pdf

TAGS:
	$(RM) TAGS
	$(FIND) . -name '*.[hcS]' -print | xargs etags -a

tags:
	$(RM) tags
	$(FIND) . -name '*.[hcS]' -print | xargs ctags -a

cscope:
	$(RM) cscope*
	$(FIND) . -name '*.[hcS]' -print | xargs cscope -b

### Detect prefix changes
TRACK_CFLAGS = $(subst ','\'',$(ALL_CFLAGS)):\
             $(bindir_SQ):$(bugexecdir_SQ):$(template_dir_SQ):$(prefix_SQ)

$(OUTPUT)PERF-CFLAGS: .FORCE-PERF-CFLAGS
	@FLAGS='$(TRACK_CFLAGS)'; \
	    if test x"$$FLAGS" != x"`cat $(OUTPUT)PERF-CFLAGS 2>/dev/null`" ; then \
		echo 1>&2 "    * new build flags or prefix"; \
		echo "$$FLAGS" >$(OUTPUT)PERF-CFLAGS; \
            fi

### Testing rules

# GNU make supports exporting all variables by "export" without parameters.
# However, the environment gets quite big, and some programs have problems
# with that.

check: $(OUTPUT)common-cmds.h
	if sparse; \
	then \
		for i in *.c */*.c; \
		do \
			sparse $(ALL_CFLAGS) $(SPARSE_FLAGS) $$i || exit; \
		done; \
	else \
		exit 1; \
	fi

### Installation rules

ifneq ($(filter /%,$(firstword $(bugexecdir))),)
bugexec_instdir = $(bugexecdir)
else
bugexec_instdir = $(prefix)/$(bugexecdir)
endif
bugexec_instdir_SQ = $(subst ','\'',$(bugexec_instdir))

install: all
	$(INSTALL) -d -m 755 '$(DESTDIR_SQ)$(bindir_SQ)'
	$(INSTALL) $(OUTPUT)bug '$(DESTDIR_SQ)$(bindir_SQ)'

install-python_ext:
	$(PYTHON_WORD) util/setup.py --quiet install --root='/$(DESTDIR_SQ)'

install-doc:
	$(MAKE) -C Documentation install

install-man:
	$(MAKE) -C Documentation install-man

install-html:
	$(MAKE) -C Documentation install-html

install-info:
	$(MAKE) -C Documentation install-info

install-pdf:
	$(MAKE) -C Documentation install-pdf

quick-install-doc:
	$(MAKE) -C Documentation quick-install

quick-install-man:
	$(MAKE) -C Documentation quick-install-man

quick-install-html:
	$(MAKE) -C Documentation quick-install-html

### Cleaning rules

clean:
	$(RM) $(LIB_OBJS) $(BUILTIN_OBJS) $(LIB_FILE) $(OUTPUT)bug.o $(LANG_BINDINGS)
	$(RM) $(ALL_PROGRAMS) bug
	$(RM) *.spec *.pyc *.pyo */*.pyc */*.pyo $(OUTPUT)common-cmds.h TAGS tags cscope*
	$(MAKE) -C Documentation/ clean
	$(RM) $(OUTPUT)PERF-VERSION-FILE $(OUTPUT)PERF-CFLAGS
	$(python-clean)

.PHONY: all install clean strip
.PHONY: shell_compatibility_test please_set_SHELL_PATH_to_a_more_modern_shell
.PHONY: .FORCE-PERF-VERSION-FILE TAGS tags cscope .FORCE-PERF-CFLAGS
