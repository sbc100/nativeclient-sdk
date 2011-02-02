# Makefile for the <PROJECT_NAME> example.

.PHONY: all clean

# The files the project starts with.  Add your code files here!
CFILES = <PROJECT_NAME>.c

CFLAGS = -Wall -Wno-long-long -pthread -Werror
#Add your includes here.
INCLUDES =
# These libraries are necessary for PPAPI and NaCl.
LDFLAGS = -lppruntime \
          -lpthread \
          -lgoogle_nacl_platform \
          -lgio \
          -lsrpc \
          $(ARCH_FLAGS)

# Add your .nmf files for each nexe you build here.
all: check_variables <PROJECT_NAME>.nmf <PROJECT_NAME>_dbg.nmf

# common.mk has rules to build .o files from .cc files.
-include common.mk

<PROJECT_NAME>.nmf: <PROJECT_NAME>_x86_32.nexe <PROJECT_NAME>_x86_64.nexe
	@echo "Creating <PROJECT_NAME>.nmf..."
	$(PYTHON) generate_nmf.py --nmf $@ \
	 --x86-64 <PROJECT_NAME>_x86_64.nexe \
         --x86-32 <PROJECT_NAME>_x86_32.nexe

<PROJECT_NAME>_dbg.nmf: <PROJECT_NAME>_x86_32_dbg.nexe \
        <PROJECT_NAME>_x86_64_dbg.nexe
	@echo "Creating <PROJECT_NAME>_dbg.nmf..."
	$(PYTHON) generate_nmf.py --nmf $@ \
	 --x86-64 <PROJECT_NAME>_x86_64_dbg.nexe \
	 --x86-32 <PROJECT_NAME>_x86_32_dbg.nexe

# Have to link with g++: the implementation of the PPAPI proxy
# is a C++ implementation

<PROJECT_NAME>_x86_32.nexe: $(OBJECTS_X86_32)
	$(CPP) $^ $(LDFLAGS) -m32 -o $@

<PROJECT_NAME>_x86_64.nexe: $(OBJECTS_X86_64)
	$(CPP) $^ $(LDFLAGS) -m64 -o $@

<PROJECT_NAME>_x86_32_dbg.nexe: $(OBJECTS_X86_32_DBG)
	$(CPP) $^ $(LDFLAGS) -m32 -o $@

<PROJECT_NAME>_x86_64_dbg.nexe: $(OBJECTS_X86_64_DBG)
	$(CPP) $^ $(LDFLAGS) -m64 -o $@

clean:
	-$(RM) *.nmf *.o *.obj *.nexe

# This target is used by the SDK build system to produce a pre-built version
# of the .nexe.  You do not need to call this target.
install_prebuilt: <PROJECT_NAME>.nmf
	-$(RM) $(OBJECTS_X86_32) $(OBJECTS_X86_64)
