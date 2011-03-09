/*
 * Copyright 2009 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */
using System;

namespace Google.NaClVsx {
  static class NaClGuids {
    public const string kGuidNaClVsxPackageCmdSetString =
        "03b5cf30-7327-4a5c-a3c9-96e7dc092b96";

    public const string kGuidNaClVsxPackagePkgString =
        "e882b8b3-696d-4633-b119-99aaee515316";

    public const string kGuidNaClVsxProjectFactoryString =
        "16DE46F3-7ED9-4CD4-8543-9C2A6A5F11F0";

    public static readonly Guid kGuidNaClVsxPackageCmdSet =
        new Guid(kGuidNaClVsxPackageCmdSetString);

    public static readonly Guid kGuidNaClVsxProjectFactory =
        new Guid(kGuidNaClVsxProjectFactoryString);
  } ;
}