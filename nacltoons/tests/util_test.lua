-- Copyright (c) 2013 The Chromium Authors. All rights reserved.
-- Use of this source code is governed by a BSD-style license that can be
-- found in the LICENSE file.

require "lunit"

module("util_test", lunit.testcase, package.seeall)

util = require "util"

test_table = {mylist = { 1, 2, 3 }, foo = 'bar', hello = 123, subtable = { key = 'value' }}

function test_TableToString()
    expected = [[
{
  hello = 123,
  subtable = {
    key = 'value',
  }
  foo = 'bar',
  mylist = {
    1,
    2,
    3,
  }
}
]]
    result = util.TableToString(test_table)
    assert_equal(expected, result)
end

function test_TableToYaml()
    expected = [[
hello: 123
subtable:key: value
foo: bar
mylist:
 - 1
 - 2
 - 3
]]
    result = util.TableToYaml(test_table, nil, nil, true)
    assert_equal(expected, result)
end

function test_TableToYamlOneLine()
    expected = '{ hello: 123, subtable: { key: value }, foo: bar, mylist: [1, 2, 3] }'
    result = util.TableToYamlOneLine(test_table)
    assert_equal(expected, result)
end
