LIBELF_DEPS = libelf/Makefile.in libelf/configure libelf/configure.in
LIBELF_OUT = libelf/lib/libelf.a

LIBDWARF_DEPS = libdwarf/Makefile.in libdwarf/configure libdwarf/configure.in
LIBDWARF_OUT = libdwarf/libdwarf.a

all: libelf libdwarf dwarfdump

libelf/Makefile: $(LIBELF_DEPS)
	cd libelf; ./configure

$(LIBELF_OUT): libelf/Makefile
	cd libelf; make

libelf: $(LIBELF_OUT)

libdwarf/Makefile: $(LIBDWARF_DEPS)
	cd libdwarf; ./configure

$(LIBDWARF_OUT): libdwarf/makefile
	cd libdwarf; make

libdwarf: $(LIBDWARF_OUT)

dwarfdump/Makefile: dwarfdump/Makefile.in   \
                    dwarfdump/configure.in  \
                    dwarfdump/configure
	cd dwarfdump; configure

dwarfdump/dwarfdump.exe: dwarfdump/Makefile
	cd dwarfdump; make

dwarfdump: dwarfdump/dwarfdump.exe
