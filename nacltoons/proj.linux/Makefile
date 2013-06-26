EXECUTABLE = nacltoons

COCOS_ROOT = ../third_party/cocos2d-x
LUA_YAML_ROOT = ../third_party/lua-yaml

INCLUDES = -I.. -I../src -I../src/third_party -I../bindings \
           -I$(COCOS_ROOT)/samples/Cpp/TestCpp/Classes/Box2DTestBed

USE_BOX2D = 1

SOURCES = main.cc \
    app_delegate.cc \
    game_manager.cc \
    level_layer.cc \
    bindings/LuaCocos2dExtensions.cpp \
    bindings/lua_level_layer.cpp \
    bindings/LuaBox2D.cpp \
    samples/Cpp/TestCpp/Classes/Box2DTestBed/GLES-Render.cpp \
    lua-yaml/lyaml.c \
    lua-yaml/api.c \
    lua-yaml/dumper.c \
    lua-yaml/emitter.c \
    lua-yaml/loader.c \
    lua-yaml/parser.c \
    lua-yaml/reader.c \
    lua-yaml/scanner.c \
    lua-yaml/writer.c \
    lua-yaml/b64.c

include $(COCOS_ROOT)/cocos2dx/proj.linux/cocos2dx.mk
OBJECTS := $(OBJECTS:.cc=.o)

# lua-yaml has some build warnings so filter out -Werror from the CFLAGS
CFLAGS := $(filter-out -Werror,$(CFLAGS))

INCLUDES += -I$(COCOS_ROOT)/scripting/lua/cocos2dx_support
INCLUDES += -I$(COCOS_ROOT)/scripting/lua/lua
INCLUDES += -I$(COCOS_ROOT)/external
INCLUDES += -I$(COCOS_ROOT)/extensions
INCLUDES += -I$(LUA_YAML_ROOT)

SHAREDLIBS += -lcocos2d -llua -lcocosdenshion -lbox2d -lextension
COCOS_LIBS = $(LIB_DIR)/libcocos2d.so $(LIB_DIR)/libbox2d.a $(LIB_DIR)/libextension.a

cocos $(COCOS_LIBS):
	USE_BOX2D=1 $(MAKE) -C $(COCOS_ROOT)

$(TARGET): $(OBJECTS) $(STATICLIBS) $(CORE_MAKEFILE_LIST) $(COCOS_LIBS) cocos
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

$(OBJ_DIR)/%.o: ../third_party/%.c $(CORE_MAKEFILE_LIST)
	@mkdir -p $(@D)
	$(LOG_CC)$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) $(VISIBILITY) -c $< -o $@

validate: ../third_party/lua-yaml/yaml.so
	../lua.sh ../data/res/validate.lua ../data/res/sample_game/game.def

run: publish

debug: $(TARGET) publish
	cd $(dir $^) && gdb ./$(notdir $<) --ex run

publish: validate
	@mkdir -p $(BIN_DIR)
	cp -ar ../data/res/* $(BIN_DIR)

.PHONY: publish cocos validate
