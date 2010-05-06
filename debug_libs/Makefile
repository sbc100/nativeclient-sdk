# Copyright 2010, The Native Client SDK Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can
# be found in the LICENSE file.

# Main makefile for the Native Client SDK debug support libraries.

.PHONY: all clean install_prebuilt

DEBUG_LIBS = trusted_gpu

all: libs

# Make publish versions of all the modules.
libs: $(foreach lib,$(DEBUG_LIBS),lib_$(lib))

clean: $(foreach lib,$(DEBUG_LIBS),clean_$(lib))

install_prebuilt: $(foreach lib,$(DEBUG_LIBS),install_prebuilt_$(lib))

# Auto-generate all the publish and clean targets.
define DEBUG_LIB_TARGETS

.PHONY: lib_$(1) clean_$(1) install_prebuilt_$(1)

# Publish target for each debug support library
lib_$(1):
	$(MAKE) -C $(1) -f develop.mk

# Clean target for each debug support library
clean_$(1):
	$(MAKE) -C $(1) -f develop.mk clean

# Target used by the SDK build system to make pre-built versions of the debug
# libraries.
install_prebuilt_$(1):
	$(MAKE) -C $(1) -f develop.mk clean
	$(MAKE) -C $(1) -f develop.mk install_prebuilt

endef

$(foreach lib,$(DEBUG_LIBS),$(eval $(call DEBUG_LIB_TARGETS,$(lib))))
