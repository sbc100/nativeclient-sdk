// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SOVER_COMPOSITE_H_
#define SOVER_COMPOSITE_H_

#include "ppapi/cpp/image_data.h"
#include "ppapi/cpp/point.h"
#include "ppapi/cpp/rect.h"

void SOverComposite(const pp::ImageData& source,
                    const pp::Rect& src_rect,
                    pp::ImageData* destination,
                    const pp::Point& dst_point);

#endif  // SOVER_COMPOSITE_H_
