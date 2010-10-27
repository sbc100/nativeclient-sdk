@rem Copyright (c) 2010 The Native Client SDK  Authors. All rights reserved.
@rem Use of this source code is governed by a BSD-style license that can be
@rem found in the LICENSE file.

setlocal

set SCONS_DIR=%~dp0third_party\scons\scons-local
@call %~dp0third_party\swtoolkit\hammer.bat %*
