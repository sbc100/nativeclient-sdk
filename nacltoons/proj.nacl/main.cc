// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CCStdC.h"
#include "cocos2d.h"
#include "CCInstance.h"
#include "CCModule.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <fcntl.h>
#include <sys/stat.h>
#include <AL/alc.h>

#include "app_delegate.h"

USING_NS_CC;
AppDelegate g_app;

void* cocos_main(void* arg) {
  CocosPepperInstance* instance = (CocosPepperInstance*)arg;
  fprintf(stderr, "in cocos_main\n");

  // Any application that uses OpenAL on NaCl needs to call this
  // before starting OpenAL.
  alSetPpapiInfo(instance->pp_instance(),
                 pp::Module::Get()->get_browser_interface());

  CCEGLView::g_instance = instance;
  CCEGLView* eglView = CCEGLView::sharedOpenGLView();
  fprintf(stderr, "calling setFrameSize\n");
  eglView->setFrameSize(instance->Size().width(), instance->Size().height());
  fprintf(stderr, "calling application->run\n");
  int rtn = CCApplication::sharedApplication()->run();
  fprintf(stderr, "app run returned: %d\n", rtn);
  return NULL;
}

namespace pp {
  Module* CreateModule() {
    return new CocosPepperModule();
  }
}
