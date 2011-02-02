# Makefile for the <PROJECT_NAME> project.

.PHONY: all clean

# The files the project starts with.  Add your code files here!
CCFILES = <PROJECT_NAME>.cc

#Add your includes here.
INCLUDES =
# These libraries are necessary for PPAPI (Pepper 2) and NaCl.
LDFLAGS = -lppruntime \
          -lppapi_cpp \
          -lgoogle_nacl_platform \
          -lgio \
          -lpthread \
          -lsrpc \
          $(ARCH_FLAGS)

# Add your .nmf files for each nexe you build here.
all: check_variables <PROJECT_NAME>.nmf <PROJECT_NAME>_dbg.nmf

# common.mk has rules to build .o files from .cc files.
-include common.mk

# Targets to create the manifests.
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

# Targets to create the nexes.
<PROJECT_NAME>_x86_32.nexe: $(OBJECTS_X86_32)
	$(CPP) $^ $(LDFLAGS) -m32 -o $@
	$(NACL_STRIP) $@ -o $@

<PROJECT_NAME>_x86_64.nexe: $(OBJECTS_X86_64)
	$(CPP) $^ $(LDFLAGS) -m64 -o $@
	$(NACL_STRIP) $@ -o $@

<PROJECT_NAME>_x86_32_dbg.nexe: $(OBJECTS_X86_32_DBG)
	$(CPP) $^ $(LDFLAGS) -m32 -o $@

<PROJECT_NAME>_x86_64_dbg.nexe: $(OBJECTS_X86_64_DBG)
	$(CPP) $^ $(LDFLAGS) -m64 -o $@

# Target to clean up
clean:
	-$(RM) *.nmf *.o *.obj *.nexe

