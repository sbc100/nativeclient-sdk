// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/base/debug_command_line.h"
#include "gtest/gtest.h"

namespace {
const char* kCmdLine =
    "c:\aabb\aa.exe -version 133 -name some -flag -port 2345 -a b";

// debug::CommandLine test fixture.
class CommandLineTest : public ::testing::Test {
 public:
  CommandLineTest() : cl_(kCmdLine) {}
  debug::CommandLine cl_;
};

// Unit tests start here.
TEST_F(CommandLineTest, Constructor) {
  char* argv[] = {"aa.exe", "-version", "1"};
  debug::CommandLine cl(sizeof(argv) / sizeof(argv[0]), argv);
  EXPECT_EQ((sizeof(argv) / sizeof(argv[0])) - 1, cl.GetParametersNum());
  EXPECT_EQ(2, cl.GetParametersNum());
  EXPECT_STREQ("aa.exe", cl.GetProgramName().c_str());
  EXPECT_STREQ("-version", cl.GetParameter(0).c_str());
  EXPECT_STREQ("1", cl.GetParameter(1).c_str());
}

TEST_F(CommandLineTest, Parse) {
  debug::CommandLine cl("aa.exe -version 1");
  EXPECT_EQ(2, cl.GetParametersNum());
  EXPECT_STREQ("aa.exe", cl.GetProgramName().c_str());
  EXPECT_STREQ("-version", cl.GetParameter(0).c_str());
  EXPECT_STREQ("1", cl.GetParameter(1).c_str());
}

TEST_F(CommandLineTest, ParseWithQuotes) {
  const char* cmd = "\"c:\aa bb\aa.exe\" -version 1 -name \"some namex\"";
  debug::CommandLine cl(cmd);
  EXPECT_EQ(4, cl.GetParametersNum());
  EXPECT_STREQ("c:\aa bb\aa.exe", cl.GetProgramName().c_str());
  EXPECT_STREQ("some namex", cl.GetParameter(3).c_str());
}

TEST_F(CommandLineTest, ToString) {
  char* argv[] = {"d:\\src\\test\\debug.exe", "-port", "4014"};
  debug::CommandLine cl(sizeof(argv) / sizeof(argv[0]), argv);
  EXPECT_STREQ("d:\\src\\test\\debug.exe -port 4014", cl.ToString().c_str());
  EXPECT_STREQ(kCmdLine, cl_.ToString().c_str());
}

TEST_F(CommandLineTest, ToStringWithQuotes) {
  char* argv[] = {"d:\\src\\a test\\debug.exe", "-port", "4014 or 4016"};
  debug::CommandLine cl(sizeof(argv) / sizeof(argv[0]), argv);
  EXPECT_STREQ("\"d:\\src\\a test\\debug.exe\" -port \"4014 or 4016\"",
               cl.ToString().c_str());
}

TEST_F(CommandLineTest, GetStringSwitch) {
  EXPECT_STREQ("133", cl_.GetStringSwitch("version", "").c_str());
  EXPECT_STREQ("some", cl_.GetStringSwitch("name", "").c_str());
  EXPECT_STREQ("b", cl_.GetStringSwitch("a", "").c_str());
  EXPECT_STREQ("def", cl_.GetStringSwitch("kuku", "def").c_str());
}

TEST_F(CommandLineTest, GetIntSwitch) {
  EXPECT_EQ(133, cl_.GetIntSwitch("version", 0));
  EXPECT_EQ(666, cl_.GetIntSwitch("kuku", 666));
  EXPECT_EQ(0, cl_.GetIntSwitch("name", 11));
  EXPECT_EQ(2345, cl_.GetIntSwitch("port", 0));
}

TEST_F(CommandLineTest, HasSwitch) {
  EXPECT_TRUE(cl_.HasSwitch("version"));
  EXPECT_TRUE(cl_.HasSwitch("-version"));
  EXPECT_TRUE(cl_.HasSwitch("--version"));

  EXPECT_TRUE(cl_.HasSwitch("name"));
  EXPECT_TRUE(cl_.HasSwitch("flag"));
  EXPECT_TRUE(cl_.HasSwitch("port"));
  EXPECT_FALSE(cl_.HasSwitch("kuku"));
}

TEST_F(CommandLineTest, CopyConstr) {
  debug::CommandLine cl(cl_);
  EXPECT_STREQ(cl_.ToString().c_str(), cl.ToString().c_str());
}

TEST_F(CommandLineTest, AssignOp) {
  debug::CommandLine cl;
  cl = cl_;
  EXPECT_STREQ(cl_.ToString().c_str(), cl.ToString().c_str());
}
}  // namespace

