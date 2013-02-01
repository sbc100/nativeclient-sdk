# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

#
# GNU Make based build file.  For details on GNU Make see:
#   http://www.gnu.org/software/make/manual/make.html
#
#

NACLPORTS_STAMP=out/naclports.stamp
COCOS2DX_STAMP=out/cocos2dx.stamp

all: $(NACLPORTS_STAMP) $(COCOS2DX_STAMP)
	$(MAKE) -f game.mk

naclports $(NACLPORTS_STAMP): build/build-naclports.sh
	build/build-naclports.sh
	touch $(NACLPORTS_STAMP)

cocos2dx $(COCOS2DX_STAMP): $(NACLPORTS_STAMP) build/build-cocos2dx.sh
	build/build-cocos2dx.sh
	touch $(COCOS2DX_STAMP)

clean:
	$(MAKE) -f game.mk clean
	$(RM) $(NACLPORTS_STAMP) $(COCOS2DX_STAMP)
