// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/rsp/rsp_packet.h"
#include <deque>
#include <map>
#include "debugger/base/debug_blob.h"
#include "gtest/gtest.h"

namespace {
rsp::Packet* kNullPacketPtr = NULL;

// RspPacket test fixture.
class RspPacketTest : public ::testing::Test {
 public:
  rsp::Packet* ParseMsg(const char* msg_str, const char* hint=NULL) {
    debug::Blob msg(msg_str);
    return rsp::Packet::CreateFromBlob(&msg, hint);
  }

template <class T>
  bool TestParseAndPrintout(T* dummy, const char* message) {
    delete dummy;
    rsp::Packet* obj = ParseMsg(message);
    if (NULL == obj)
      return false;

    T* pack = rsp::packet_cast<T>(obj);
    if (pack != obj) {
      delete obj;
      return false;
    }

    debug::Blob blob;
    pack->ToBlob(&blob);
    delete pack;
    std::string s = blob.ToString();
    return (s == message);
  }
};

//#define TestParseAndPrintout(message)



// Unit tests start here.
TEST_F(RspPacketTest, EmptyPacket) {
  rsp::Packet* obj = ParseMsg("");
  ASSERT_NE(kNullPacketPtr, obj);

  rsp::EmptyPacket* ep = rsp::packet_cast<rsp::EmptyPacket>(obj);
  EXPECT_EQ(obj, ep);
  ASSERT_NE(kNullPacketPtr, ep);

  debug::Blob blob;
  ep->ToBlob(&blob);
  EXPECT_EQ(0, blob.size());
  delete obj;
}

TEST_F(RspPacketTest, TypingPacketVisitor) {
  std::deque<rsp::Packet*> packs;
  packs.push_back(new rsp::Packet);
  packs.push_back(new rsp::EmptyPacket);
  packs.push_back(new rsp::QuerySupportedCommand);
  packs.push_back(new rsp::QuerySupportedReply);
  packs.push_back(new rsp::GetStopReasonCommand);
  packs.push_back(new rsp::StopReply);
  packs.push_back(new rsp::ReadMemoryCommand);
  packs.push_back(new rsp::WriteMemoryCommand);
  packs.push_back(new rsp::BlobReply);
  packs.push_back(new rsp::ReadRegistersCommand);
  packs.push_back(new rsp::WriteRegistersCommand);
  packs.push_back(new rsp::ErrorReply);
  packs.push_back(new rsp::OkReply);
  packs.push_back(new rsp::SetCurrentThread);
  packs.push_back(new rsp::GetCurrentThreadCommand);
  packs.push_back(new rsp::GetCurrentThreadReply);
  packs.push_back(new rsp::ContinueCommand);
  packs.push_back(new rsp::StepCommand);
  packs.push_back(new rsp::IsThreadAliveCommand);
  packs.push_back(new rsp::QXferFeaturesReadCommand);
  packs.push_back(new rsp::QXferReply);
  packs.push_back(new rsp::GetThreadInfoCommand);
  packs.push_back(new rsp::GetThreadInfoReply);

  std::map<int, int> type_ids;
  for (size_t i = 0; i < packs.size(); i++) {
    rsp::TypingPacketVisitor vis;
    rsp::Packet* packet = packs[i];
    packet->AcceptVisitor(&vis);

    bool found = type_ids.end() != type_ids.find(vis.type_);
    EXPECT_FALSE(found);

    int type0 = vis.type_;
    rsp::Packet* packet2 = packet->Create();
    packet2->AcceptVisitor(&vis);
    EXPECT_EQ(type0, vis.type_);

    type_ids[type0] = 1;
    delete packet;
    delete packet2;
  }
}

TEST_F(RspPacketTest, QuerySupportedCommand) {
  const char* kMsg = "qSupported:xmlRegisters=i386;qRelocInsn+";
  rsp::Packet* obj = ParseMsg(kMsg);
  ASSERT_NE(kNullPacketPtr, obj);

  rsp::QuerySupportedCommand* pack =
      rsp::packet_cast<rsp::QuerySupportedCommand>(obj);
  EXPECT_EQ(obj, pack);
  ASSERT_NE(kNullPacketPtr, pack);

  EXPECT_EQ(2, pack->GetFeaturesNum());
  EXPECT_STREQ("i386", pack->GetFeature("xmlRegisters").c_str());
  EXPECT_STREQ("+", pack->GetFeature("qRelocInsn").c_str());
  EXPECT_STREQ("xmlRegisters", pack->GetFeatureName(0).c_str());
  EXPECT_STREQ("qRelocInsn", pack->GetFeatureName(1).c_str());
  EXPECT_STREQ("", pack->GetFeatureName(2).c_str());

  debug::Blob blob;
  pack->ToBlob(&blob);
  EXPECT_STREQ(kMsg, blob.ToString().c_str());
  delete obj;
}

TEST_F(RspPacketTest, QuerySupportedReply) {
  const char* kMsg = "PacketSize=7cf;qXfer:libraries:read+;qXfer:features:read+";
  rsp::Packet* obj = ParseMsg(kMsg, "qSupported$Reply");
  ASSERT_NE(kNullPacketPtr, obj);

  rsp::QuerySupportedReply* pack =
      rsp::packet_cast<rsp::QuerySupportedReply>(obj);
  EXPECT_EQ(obj, pack);
  ASSERT_NE(kNullPacketPtr, pack);

  EXPECT_EQ(3, pack->GetFeaturesNum());
  EXPECT_STREQ("PacketSize", pack->GetFeatureName(0).c_str());
  EXPECT_STREQ("qXfer:libraries:read", pack->GetFeatureName(1).c_str());
  EXPECT_STREQ("qXfer:features:read", pack->GetFeatureName(2).c_str());

  EXPECT_STREQ("7cf", pack->GetFeature("PacketSize").c_str());
  EXPECT_STREQ("+", pack->GetFeature("qXfer:libraries:read").c_str());
  EXPECT_STREQ("+", pack->GetFeature("qXfer:features:read").c_str());

  debug::Blob blob;
  pack->ToBlob(&blob);
  EXPECT_STREQ(kMsg, blob.ToString().c_str());
  delete obj;
}

TEST_F(RspPacketTest, StopReply1) {
  const char* kMsg = "S15";
  rsp::Packet* obj = ParseMsg(kMsg);
  ASSERT_NE(kNullPacketPtr, obj);

  rsp::StopReply* pack = rsp::packet_cast<rsp::StopReply>(obj);
  EXPECT_EQ(obj, pack);
  ASSERT_NE(kNullPacketPtr, pack);

  EXPECT_EQ(21, pack->signal_number());
  EXPECT_EQ(rsp::StopReply::SIGNALED, pack->stop_reason());
  EXPECT_EQ(0, pack->pid());

  debug::Blob blob;
  pack->ToBlob(&blob);
  EXPECT_STREQ(kMsg, blob.ToString().c_str());

  pack->set_signal_number(5);
  pack->ToBlob(&blob);
  EXPECT_STREQ("S05", blob.ToString().c_str());
  delete obj;
}

TEST_F(RspPacketTest, StopReply2) {
  const char* kMsg = "W21;process:138c";
  rsp::Packet* obj = ParseMsg(kMsg);
  ASSERT_NE(kNullPacketPtr, obj);

  rsp::StopReply* pack = rsp::packet_cast<rsp::StopReply>(obj);
  EXPECT_EQ(obj, pack);
  ASSERT_NE(kNullPacketPtr, pack);

  EXPECT_EQ(33, pack->exit_code());
  EXPECT_EQ(rsp::StopReply::EXITED, pack->stop_reason());
  EXPECT_EQ(0x138c, pack->pid());

  debug::Blob blob;
  pack->ToBlob(&blob);
  EXPECT_STREQ(kMsg, blob.ToString().c_str());

  pack->set_exit_code(5);
  pack->ToBlob(&blob);
  EXPECT_STREQ("W05;process:138c", blob.ToString().c_str());

  pack->set_pid(10);
  pack->ToBlob(&blob);
  EXPECT_STREQ("W05;process:a", blob.ToString().c_str());

  delete obj;
}

TEST_F(RspPacketTest, StopReply3) {
  const char* kMsg = "X21;process:138c";
  rsp::Packet* obj = ParseMsg(kMsg);
  ASSERT_NE(kNullPacketPtr, obj);

  rsp::StopReply* pack = rsp::packet_cast<rsp::StopReply>(obj);
  EXPECT_EQ(obj, pack);
  ASSERT_NE(kNullPacketPtr, pack);

  EXPECT_EQ(33, pack->signal_number());
  EXPECT_EQ(rsp::StopReply::TERMINATED, pack->stop_reason());
  EXPECT_EQ(0x138c, pack->pid());

  debug::Blob blob;
  pack->ToBlob(&blob);
  EXPECT_STREQ(kMsg, blob.ToString().c_str());

  pack->set_signal_number(5);
  pack->ToBlob(&blob);
  EXPECT_STREQ("X05;process:138c", blob.ToString().c_str());

  pack->set_pid(10);
  pack->ToBlob(&blob);
  EXPECT_STREQ("X05;process:a", blob.ToString().c_str());

  delete obj;
}

TEST_F(RspPacketTest, ReadMemoryCommand) {
  const char* kMsg = "mcffffff80,40";
  rsp::Packet* obj = ParseMsg(kMsg);
  ASSERT_NE(kNullPacketPtr, obj);

  rsp::ReadMemoryCommand* pack = rsp::packet_cast<rsp::ReadMemoryCommand>(obj);
  EXPECT_EQ(obj, pack);
  ASSERT_NE(kNullPacketPtr, pack);

  uint64_t kAddr = 0xcffffff80LL;
  uint64_t addr = pack->addr();
  
  EXPECT_EQ(kAddr, addr);
  EXPECT_EQ(64, pack->num_of_bytes());

  debug::Blob blob;
  pack->ToBlob(&blob);
  std::string s = blob.ToString();
  EXPECT_STREQ(kMsg, blob.ToString().c_str());
  delete obj;
}

TEST_F(RspPacketTest, WriteMemoryCommand) {
  const char* kMsg = "Mcffffff80,1:cc";
  rsp::Packet* obj = ParseMsg(kMsg);
  ASSERT_NE(kNullPacketPtr, obj);

  rsp::WriteMemoryCommand* pack =
    rsp::packet_cast<rsp::WriteMemoryCommand>(obj);
  EXPECT_EQ(obj, pack);
  ASSERT_NE(kNullPacketPtr, pack);

  uint64_t kAddr = 0xcffffff80LL;
  uint64_t addr = pack->addr();
  EXPECT_EQ(kAddr, addr);
  EXPECT_EQ(1, pack->data().size());
  EXPECT_STREQ("cc", pack->data().ToHexString().c_str());
  delete obj;
}

TEST_F(RspPacketTest, BlobReply) {
  const char* kMsgs[] = {
      "",
      "0b00cdcd2b00cdcd5300cdcd2b00cdcd",
      "cc",
      "00000000",
      "123456789abcdeff" };

  size_t num = sizeof(kMsgs) / sizeof(kMsgs[0]);
  for (size_t i = 0; i < num; i++) {
    const char* kMsg = kMsgs[i];
    rsp::Packet* obj = ParseMsg(kMsg, "blob$Reply");
    ASSERT_NE(kNullPacketPtr, obj) << "msg=[" << kMsg << "]";

    rsp::BlobReply* pack =
      rsp::packet_cast<rsp::BlobReply>(obj);
    EXPECT_EQ(obj, pack) << "msg=[" << kMsg << "]";
    ASSERT_NE(kNullPacketPtr, pack) << "msg=[" << kMsg << "]";

    size_t expected_bytes = strlen(kMsg) / 2;
    EXPECT_EQ(expected_bytes, pack->data().size()) << "msg=[" << kMsg << "]";

    debug::Blob blob;
    pack->ToBlob(&blob);
    std::string s = blob.ToString();
    EXPECT_STREQ(kMsg, blob.ToString().c_str());
    delete obj;
  }
}

TEST_F(RspPacketTest, SingleWordCommands) {
  EXPECT_TRUE(TestParseAndPrintout(new rsp::ReadRegistersCommand, "g"));
  EXPECT_TRUE(TestParseAndPrintout(
      new rsp::WriteRegistersCommand,
      "G00a500000000000c0ffffff00000000d85123020"));
  EXPECT_TRUE(TestParseAndPrintout(new rsp::WriteMemoryCommand,
                                   "Mcffffff80,1:cc"));
  EXPECT_TRUE(TestParseAndPrintout(new rsp::GetStopReasonCommand, "?"));
  EXPECT_TRUE(TestParseAndPrintout(new rsp::ErrorReply, "E02"));
  EXPECT_TRUE(TestParseAndPrintout(new rsp::ErrorReply, "E15"));
  EXPECT_TRUE(TestParseAndPrintout(new rsp::OkReply, "OK"));
  EXPECT_TRUE(TestParseAndPrintout(new rsp::SetCurrentThread, "Hc-1"));
  EXPECT_TRUE(TestParseAndPrintout(new rsp::SetCurrentThread, "Hc0"));
  EXPECT_TRUE(TestParseAndPrintout(new rsp::SetCurrentThread, "Hg-1"));
  EXPECT_TRUE(TestParseAndPrintout(new rsp::SetCurrentThread, "Hg0"));
  EXPECT_TRUE(TestParseAndPrintout(new rsp::SetCurrentThread, "Hg138c"));
  EXPECT_TRUE(TestParseAndPrintout(new rsp::SetCurrentThread, "Hc138c"));
  EXPECT_TRUE(TestParseAndPrintout(new rsp::GetCurrentThreadCommand, "qC"));
  EXPECT_TRUE(TestParseAndPrintout(new rsp::GetCurrentThreadReply, "QC138c"));
  EXPECT_TRUE(TestParseAndPrintout(new rsp::ContinueCommand, "c"));
  EXPECT_TRUE(TestParseAndPrintout(new rsp::StepCommand, "s"));
  EXPECT_TRUE(TestParseAndPrintout(new rsp::IsThreadAliveCommand, "T138c"));
  EXPECT_TRUE(TestParseAndPrintout(new rsp::QXferFeaturesReadCommand,
                                   "qXfer:features:read:target.xml:0,7ca"));
  EXPECT_TRUE(TestParseAndPrintout(new rsp::GetThreadInfoCommand,
                                   "qfThreadInfo"));
  EXPECT_TRUE(TestParseAndPrintout(new rsp::GetThreadInfoCommand,
                                   "qsThreadInfo"));
}

TEST_F(RspPacketTest, WriteRegistersCommand) {
  rsp::Packet* obj = ParseMsg("G00a500000000000c0ffffff00000000d85123020");
  ASSERT_NE(kNullPacketPtr, obj);

  rsp::WriteRegistersCommand* pack =
    rsp::packet_cast<rsp::WriteRegistersCommand>(obj);
  ASSERT_NE(kNullPacketPtr, pack);

  EXPECT_EQ(20, pack->data().size());
  EXPECT_EQ(0x00, pack->data().GetAt(0));
  EXPECT_EQ(0xa5, pack->data().GetAt(1));
  EXPECT_EQ(0x20, pack->data().GetAt(19));
  delete obj;
}

TEST_F(RspPacketTest, SetCurrentThread) {
  rsp::SetCurrentThread* obj =
      rsp::packet_cast<rsp::SetCurrentThread>(ParseMsg("Hc-1"));
  ASSERT_NE(kNullPacketPtr, obj);
  EXPECT_EQ(rsp::SetCurrentThread::FOR_CONTINUE, obj->subtype());
  EXPECT_EQ(-1, obj->thread_id());
  delete obj;

  obj = rsp::packet_cast<rsp::SetCurrentThread>(ParseMsg("Hc0"));
  ASSERT_NE(kNullPacketPtr, obj);
  EXPECT_EQ(rsp::SetCurrentThread::FOR_CONTINUE, obj->subtype());
  EXPECT_EQ(0, obj->thread_id());
  delete obj;

  obj = rsp::packet_cast<rsp::SetCurrentThread>(ParseMsg("Hg-1"));
  ASSERT_NE(kNullPacketPtr, obj);
  EXPECT_EQ(rsp::SetCurrentThread::FOR_READ, obj->subtype());
  EXPECT_EQ(-1, obj->thread_id());
  delete obj;

  obj = rsp::packet_cast<rsp::SetCurrentThread>(ParseMsg("Hg0"));
  ASSERT_NE(kNullPacketPtr, obj);
  EXPECT_EQ(rsp::SetCurrentThread::FOR_READ, obj->subtype());
  EXPECT_EQ(0, obj->thread_id());
  delete obj;

  obj = rsp::packet_cast<rsp::SetCurrentThread>(ParseMsg("Hg138c"));
  ASSERT_NE(kNullPacketPtr, obj);
  EXPECT_EQ(rsp::SetCurrentThread::FOR_READ, obj->subtype());
  EXPECT_EQ(0x138c, obj->thread_id());
  delete obj;

  obj = rsp::packet_cast<rsp::SetCurrentThread>(ParseMsg("Hc138c"));
  ASSERT_NE(kNullPacketPtr, obj);
  EXPECT_EQ(rsp::SetCurrentThread::FOR_CONTINUE, obj->subtype());
  EXPECT_EQ(0x138c, obj->thread_id());
  delete obj;
}

TEST_F(RspPacketTest, IsThreadAliveCommand) {
  rsp::IsThreadAliveCommand* obj =
      rsp::packet_cast<rsp::IsThreadAliveCommand>(ParseMsg("T2e0"));
  ASSERT_NE(kNullPacketPtr, obj);
  EXPECT_EQ(0x2e0, obj->value());
}

TEST_F(RspPacketTest, QXferFeaturesReadCommand) {
  rsp::QXferFeaturesReadCommand* obj =
      rsp::packet_cast<rsp::QXferFeaturesReadCommand>(
          ParseMsg("qXfer:features:read:target.xml:0,7ca"));
  ASSERT_NE(kNullPacketPtr, obj);
  EXPECT_STREQ("target.xml", obj->file_name().c_str());
  EXPECT_EQ(0, obj->offset());
  EXPECT_EQ(0x7ca, obj->length());
}

TEST_F(RspPacketTest, QXferReply) {
  const char* kMsg =
      "l<target><architecture>i386:x86-64</architecture></target>";
  rsp::QXferReply* obj =
      rsp::packet_cast<rsp::QXferReply>(ParseMsg(kMsg, "qXfer$Reply"));
  ASSERT_NE(kNullPacketPtr, obj);
  EXPECT_STREQ("<target><architecture>i386:x86-64</architecture></target>",
               obj->body().c_str());
  EXPECT_TRUE(obj->eom());

  debug::Blob blob;
  obj->ToBlob(&blob);
  EXPECT_STREQ(kMsg, blob.ToString().c_str());
  delete obj;
}

TEST_F(RspPacketTest, GetThreadInfoReply) {
  const char* kMsg = "m1598,138c";
  rsp::GetThreadInfoReply* obj =
      rsp::packet_cast<rsp::GetThreadInfoReply>(ParseMsg(kMsg, "GetThreadInfo$Reply"));
  ASSERT_NE(kNullPacketPtr, obj);
  EXPECT_FALSE(obj->eom());
  EXPECT_EQ(2, obj->threads_ids().size());
  EXPECT_EQ(0x1598, obj->threads_ids()[0]);
  EXPECT_EQ(0x138c, obj->threads_ids()[1]);

  debug::Blob blob;
  obj->ToBlob(&blob);
  EXPECT_STREQ(kMsg, blob.ToString().c_str());
  delete obj;
}

TEST_F(RspPacketTest, GetThreadInfoReply2) {
  const char* kMsg = "l138c";
  rsp::GetThreadInfoReply* obj =
      rsp::packet_cast<rsp::GetThreadInfoReply>(ParseMsg(kMsg, "GetThreadInfo$Reply"));
  ASSERT_NE(kNullPacketPtr, obj);
  EXPECT_TRUE(obj->eom());
  EXPECT_EQ(1, obj->threads_ids().size());
  EXPECT_EQ(0x138c, obj->threads_ids()[0]);

  debug::Blob blob;
  obj->ToBlob(&blob);
  EXPECT_STREQ(kMsg, blob.ToString().c_str());
  delete obj;
}

}  // namespace
