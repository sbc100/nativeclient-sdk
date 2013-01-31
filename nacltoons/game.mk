NACL_ARCH = x86_64
NACL_LIBC = newlib
OS ?= linux

ifeq ($(NACL_ARCH),arm)
TOOLCHAIN_DIR=$(OS)_arm_newlib
else
TOOLCHAIN_DIR=$(OS)_x86_newlib
endif

NACL_TOOLCHAIN_ROOT=$(NACL_SDK_ROOT)/toolchain/$(TOOLCHAIN_DIR)
NACL_BIN_PATH=$(NACL_TOOLCHAIN_ROOT)/bin

CC      = $(NACL_BIN_PATH)/$(NACL_ARCH)-nacl-gcc
CXX     = $(NACL_BIN_PATH)/$(NACL_ARCH)-nacl-g++
CCFLAGS = -Wall
CXXFLAGS = -Wall
VISIBILITY =

COCOS2DX_PATH = out/cocos2dx
NACLPORTS_PATH = out/naclports
INCLUDES =		-IClasses \
			-I$(COCOS2DX_PATH) \
			-I$(COCOS2DX_PATH)/cocoa \
			-I$(COCOS2DX_PATH)/include \
			-I$(COCOS2DX_PATH)/platform/nacl \
			-I$(COCOS2DX_PATH)/kazmath/include \
			-I$(NACL_SDK_ROOT)/include \
			-I$(NACLPORTS_PATH)/include


OBJECTS = ./main.o \
        Classes/AppDelegate.o \
        Classes/HelloWorldScene.o

SHAREDLIBS = -pthread

ifdef DEBUG
BIN_DIR = bin/debug
CCFLAGS += -g3 -O0
CXXFLAGS += -g3 -O0
SHAREDLIBS += -L$(COCOS2DX_PATH)/lib/newlib_$(NACL_ARCH)/Debug
SHAREDLIBS += -L$(NACLPORTS_PATH)/lib/newlib_$(NACL_ARCH)/Debug
DEFINES += -DDEBUG -DCOCOS2D_DEBUG=1
STATICLIBS = -L$(NACL_SDK_ROOT)/lib/$(NACL_LIBC)_$(NACL_ARCH)/Debug
else
BIN_DIR = bin/release
CCFLAGS += -O3
CXXFLAGS += -O3
SHAREDLIBS += -L$(COCOS2DX_PATH)/lib/newlib_$(NACL_ARCH)/Release
SHAREDLIBS += -L$(NACLPORTS_PATH)/lib/newlib_$(NACL_ARCH)/Release
DEFINES += -DNDEBUG
STATICLIBS = -L$(NACL_SDK_ROOT)/lib/$(NACL_LIBC)_$(NACL_ARCH)/Release
endif

STATICLIBS += -lfontconfig -lfreetype -lexpat -lxml2 -lpng12 -ljpeg -ltiff -lppapi_gles2 -lnacl-mounts -lppapi -lppapi_cpp
SHAREDLIBS += -lcocos2d -lz

TARGET = $(BIN_DIR)/HelloCpp.nexe
NMF = $(basename $(TARGET)).nmf

.PHONY: game
game: $(TARGET) $(NMF)

####### Build rules
$(TARGET): $(OBJECTS)
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(DEFINES) $(OBJECTS) -o $@ $(SHAREDLIBS) $(STATICLIBS)

####### Compile
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(DEFINES) $(VISIBILITY) -c $< -o $@

%.o: %.c
	$(CC) $(CCFLAGS) $(INCLUDES) $(DEFINES) $(VISIBILITY) -c $< -o $@

clean::
	rm -f $(OBJECTS) $(TARGET) core


$(NMF): $(TARGET)
	$(NACL_SDK_ROOT)/tools/create_nmf.py -o $(NMF) $(TARGET) --objdump=i686-nacl-objdump -L$(NACL_SDK_ROOT)/toolchain/linux_x86_newlib/x86_64-nacl/lib/ -s $(BIN_DIR)

run: all
	$(NACL_SDK_ROOT)/tools/httpd.py --no_dir_check

.PHONY: run clean nmf
