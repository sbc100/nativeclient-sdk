EXECUTABLE = nacltoons

INCLUDES = -I.. -I../src -I../src/third_party -I../bindings

COCOS_ROOT = ../../../src/third_party/cocos2d-x

USE_BOX2D = 1

SOURCES = main.cc \
    app_delegate.cc \
    game_manager.cc \
    level_layer.cc \
    bindings/LuaCocos2dExtensions.cpp \
    bindings/lua_level_layer.cpp \
    bindings/LuaBox2D.cpp \
    samples/Cpp/TestCpp/Classes/Box2DTestBed/GLES-Render.cpp

include $(COCOS_ROOT)/cocos2dx/proj.linux/cocos2dx.mk
OBJECTS := $(OBJECTS:.cc=.o)

INCLUDES += -I$(COCOS_ROOT)/scripting/lua/cocos2dx_support
INCLUDES += -I$(COCOS_ROOT)/scripting/lua/lua
INCLUDES += -I$(COCOS_ROOT)/external
INCLUDES += -I$(COCOS_ROOT)/extensions

SHAREDLIBS += -lcocos2d -llua -lcocosdenshion -lbox2d -lextension
COCOS_LIBS = $(LIB_DIR)/libcocos2d.so

$(TARGET): $(OBJECTS) $(STATICLIBS) $(COCOS_LIBS) $(CORE_MAKEFILE_LIST)
	@mkdir -p $(@D)
	$(LOG_LINK)$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@ $(SHAREDLIBS) $(STATICLIBS)

$(OBJ_DIR)/%.o: %.cc $(CORE_MAKEFILE_LIST)
	@mkdir -p $(@D)
	$(LOG_CXX)$(CXX) $(CXXFLAGS) $(INCLUDES) $(DEFINES) $(VISIBILITY) -c $< -o $@

$(OBJ_DIR)/%.o: ../src/%.cc $(CORE_MAKEFILE_LIST)
	@mkdir -p $(@D)
	$(LOG_CXX)$(CXX) $(CXXFLAGS) $(INCLUDES) $(DEFINES) $(VISIBILITY) -c $< -o $@

$(OBJ_DIR)/%.o: ../%.cpp $(CORE_MAKEFILE_LIST)
	@mkdir -p $(@D)
	$(LOG_CXX)$(CXX) $(CXXFLAGS) $(INCLUDES) $(DEFINES) $(VISIBILITY) -c $< -o $@

$(OBJ_DIR)/%.o: $(COCOS_ROOT)/%.cpp $(CORE_MAKEFILE_LIST)
	@mkdir -p $(@D)
	$(LOG_CXX)$(CXX) $(CXXFLAGS) $(INCLUDES) $(DEFINES) $(VISIBILITY) -c $< -o $@

run: publish

publish:
	@mkdir -p $(BIN_DIR)
	cp -ar ../data/res/* $(BIN_DIR)

.PHONY: publish
