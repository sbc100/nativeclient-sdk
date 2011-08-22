VPATH = ../base ../rsp ../core

BASE_CCFILES = \
		debug_blob.cc \
		debug_command_line.cc \
		debug_socket.cc

RSP_CCFILES = \
		rsp_blob_utils.cc \
		rsp_common_replies.cc \
		rsp_control_packets.cc \
		rsp_info_packets.cc \
		rsp_packet_utils.cc \
		rsp_packets.cc \
		rsp_packetizer.cc \
		rsp_threads_packets.cc

CORE_CCFILES = \
		debug_api.cc \
		debuggee_process.cc \
		debuggee_thread.cc

SERVER_CCFILES = \
		debug_server.cc \
		gdb_registers.cc \
		nacl-gdb_server.cc

CCFILES = $(BASE_CCFILES) $(RSP_CCFILES) $(CORE_CCFILES) $(SERVER_CCFILES)

CFLAGS = -Wall -Wno-long-long -Werror
INCLUDES = -I../../

.SUFFIXES: .cc .o
CPP = g++
OBJDIR = obj
OPT_FLAGS = -O2

$(OBJDIR)/%64.o:: %.cc
	$(CPP) $(CFLAGS) -m64 $(INCLUDES) -c -o $@ $<

$(OBJDIR)/%32.o:: %.cc
	$(CPP) $(CFLAGS) -m32 $(INCLUDES) -c -o $@ $<

$(OBJDIR):
	mkdir -p $(OBJDIR)

OBJECTS_32 = $(patsubst %,${OBJDIR}/%,$(CCFILES:%.cc=%32.o))
OBJECTS_64 = $(patsubst %,${OBJDIR}/%,$(CCFILES:%.cc=%64.o))

dbg32.exe: $(OBJECTS_32)
	$(CPP) -m32 $^ $(LDFLAGS) -o $@

dbg64.exe: $(OBJECTS_64)
	$(CPP) -m64 $^ $(LDFLAGS) -o $@
