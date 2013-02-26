# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

#
# GNU Make based build file.  For details on GNU Make see:
#   http://www.gnu.org/software/make/manual/make.html
#
#

ifndef BUILDBOT_BUILDERNAME
# By default, unless we are building on the buildbot set NACL_ARCH to so that
# we only build for a single architecture.
NACL_ARCH ?= x86_64
endif

# Use this variable to enable/disable nacl_mounts an an alternative to
# nacl_io.  When you do this you will also want to clean the cocos build using:
# "build/build_cocos2dx.sh clean"
#NACL_MOUNTS = 1

DEBUG ?= 1
ifeq ($(DEBUG),1)
  CONFIG := Debug
else
  CONFIG := Release
endif

OUT_DIR := out
PUBLISH_DIR := $(OUT_DIR)/publish

all: cocos2dx
	@echo '@@@BUILD_STEP build game@@@'
ifdef NACL_MOUNTS
	$(MAKE) -f game.mk NACL_ARCH=$(NACL_ARCH) DEBUG=$(DEBUG) NACL_MOUNTS=$(NACL_MOUNTS)
else
	$(MAKE) -f game.mk NACL_ARCH=$(NACL_ARCH) DEBUG=$(DEBUG)
endif

cocos2dx : build/build-cocos2dx.sh
	NACL_MOUNTS=$(NACL_MOUNTS) NACL_ARCH=$(NACL_ARCH) build/build-cocos2dx.sh

really-clean: clean
	$(RM) -r $(OUT_DIR)

clean:
	$(MAKE) -f game.mk clean

publish: all
	rm -fr $(PUBLISH_DIR)
	mkdir -p $(PUBLISH_DIR)
	cp $(OUT_DIR)/newlib/$(CONFIG)/*.nexe $(PUBLISH_DIR)
	cp $(OUT_DIR)/newlib/$(CONFIG)/*.nmf $(PUBLISH_DIR)
	cp -r data/* $(PUBLISH_DIR)
ifndef NACL_MOUNTS
	cp -r data/res $(PUBLISH_DIR)/Resources
endif

CHROME_ARGS += --no-first-run --user-data-dir=$(OUT_DIR)/user-data-dir

run: publish
	NACL_ARCH=$(NACL_ARCH) CHROME_ARGS="$(CHROME_ARGS)" $(MAKE) -f game.mk RUN

run-app: publish
	$(CHROME_PATH) $(CHROME_ARGS) --load-extension=$(PUBLISH_DIR) chrome://newtab

.PHONY: all naclports cocos2dx clean publish run run-app really-clean
