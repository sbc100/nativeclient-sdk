// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef FRONTEND_H_
#define FRONTEND_H_

#include "cocos2d.h"

USING_NS_CC;

/**
 * Front end scene and layer.
 */
class FrontEnd : public CCLayerColor {
 public:
  FrontEnd() {}
  ~FrontEnd() {}
  CREATE_FUNC(FrontEnd);
  virtual bool init();
  static CCScene* scene();

 private:
  // menu callbacks
  void StartGame(CCObject* sender);
};

#endif // !FRONTEND_H_
