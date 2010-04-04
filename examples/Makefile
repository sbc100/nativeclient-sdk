# Copyright 2010, The Native Client SDK Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can
# be found in the LICENSE file.

# Main makefile for the Native Client SDK examples.  These targets set certain
# make variables and then call the descendant makefiles in each of the
# examples.

.PHONY: all develop publish clean run_hello_world run_pi_generator run_tumbler

EXAMPLES = hello_world pi_generator tumbler
DEVELOP_LIBS = trusted_gpu

all: develop publish

-include common.mk

ifeq ($(PLATFORM), mac)
  CHROME_APP = /Applications/Chromium.app/Contents/MacOS/Chromium
endif
ifeq ($(PLATFORM), linux)
  CHROME_APP = ./chrome
endif
ifeq ($(PLATFORM), win)
  CHROME_APP = ./chrome
endif

# Make development versions of all the modules.
develop:
	mkdir -p develop/$(PLATFORM)_$(TARGET)
	@for examp in $(DEVELOP_LIBS) $(EXAMPLES) ; do \
	    (cd $$examp; $(MAKE) -w develop); \
	done

# Make publish versions of all the modules.
publish:
	mkdir -p publish/$(PLATFORM)_$(TARGET)
	@for examp in $(EXAMPLES) ; do \
	    (cd $$examp; $(MAKE) -w publish); \
	done

# Run the published versions of a specific module.
run_hello_world run_pi_generator run_tumbler:
	$(MAKE) publish
	@is_running=`ps -ef | grep httpd.py | grep -v grep`; \
	if [ -z "$$is_running" ] ; then \
	  (python httpd.py &) ; \
	else \
	  echo "localhost HTTP server is running"; \
	fi
	@(target=`echo $@ | sed -e "s/^run_\(.*\)/\1.html/"`; \
	    $(CHROME_APP) \
	    --no-sandbox --enable-nacl --internal-pepper \
	    http://localhost:5103/publish/$(PLATFORM)_$(TARGET)/$$target)

clean::
	@for examp in $(DEVELOP_LIBS) $(EXAMPLES) ; do \
	    (cd $$examp; $(MAKE) clean); \
	done
