@rem Copyright (c) 2010 The Native Client SDK  Authors. All rights reserved.
@rem Use of this source code is governed by a BSD-style license that can be
@rem found in the LICENSE file.

@echo off
setlocal

set SCONS_DIR=%~dp0third_party\scons\scons-local
set PYMOX=%~dp0third_party\pymox"
set NACL_SDK_ROOT=%~dp0
@call %~dp0third_party\swtoolkit\hammer.bat %*
