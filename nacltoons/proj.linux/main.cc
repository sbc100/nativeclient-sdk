// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "../src/app_delegate.h"
#include "cocos2d.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string>

USING_NS_CC;

int main(int argc, char **argv)
{
    // create the application instance
    AppDelegate app;

    CCEGLView* eglView = CCEGLView::sharedOpenGLView();
    eglView->setFrameSize(700, 500);
    char respath[PATH_MAX];
    if (!getcwd(respath, PATH_MAX))
      return 1;
    strcat(respath, "/../../../data/res");
    CCFileUtils::sharedFileUtils()->addSearchPath(respath);

    return CCApplication::sharedApplication()->run();
}
