# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

#
# GNU Make based build file.  For details on GNU Make see:
#   http://www.gnu.org/software/make/manual/make.html
#
#

CHROME_PATH?=google-chrome
NACL_ARCH?=x86_64
DEBUG?=1
ifeq ($(DEBUG),1)
  CONFIG=Debug
else
  CONFIG=Release
endif

NACLPORTS_STAMP=out/naclports.stamp
COCOS2DX_STAMP=out/cocos2dx.stamp

all: $(NACLPORTS_STAMP) $(COCOS2DX_STAMP)
	@echo '@@@BUILD_STEP build game@@@'
	$(MAKE) -f game.mk NACL_ARCH=$(NACL_ARCH) DEBUG=$(DEBUG)

naclports $(NACLPORTS_STAMP): build/build-naclports.sh
	build/build-naclports.sh
	touch $(NACLPORTS_STAMP)

cocos2dx $(COCOS2DX_STAMP): $(NACLPORTS_STAMP) build/build-cocos2dx.sh
	build/build-cocos2dx.sh
	touch $(COCOS2DX_STAMP)

clean:
	$(MAKE) -f game.mk clean
	$(RM) $(NACLPORTS_STAMP) $(COCOS2DX_STAMP)

publish: all
	mkdir -p out/publish
	cp out/newlib/$(CONFIG)/*.nexe out/publish
	cp out/newlib/$(CONFIG)/*.nmf out/publish
	cp -r data/* out/publish

run: publish
	$(MAKE) -f game.mk RUN

run-app: publish
	$(CHROME_PATH) --no-first-run --user-data-dir=out/user-data-dir --load-extension=out/publish chrome://newtab

.PHONY: all naclports cocos2dx clean publish run run-app
