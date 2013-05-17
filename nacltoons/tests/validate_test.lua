-- Copyright (c) 2013 The Chromium Authors. All rights reserved.
-- Use of this source code is governed by a BSD-style license that can be
-- found in the LICENSE file.

require "lunit"

module("validate_test", lunit.testcase, package.seeall)

validate = require "validate"

function test_EmptyGameDef()
    validate.ValidateGameDef('dummygame.def', { })
end

function test_GameDefInvalidKey()
    local function doError()
        validate.ValidateGameDef('dummygame.def', { foo = 1 })
    end
    assert_error("invalid key failed to generate error", doError)
end

function test_LevelDefEmpty()
    validate.ValidateLevelDef('dummylevel.def', { }, { })
end

function test_LevelDefInvalidKey()
    local function doError()
        validate.ValidateLevelDef('dummylevel.def', { }, { foo = 1 })
    end
    assert_error("invalid key failed to generate error", doError)
end
