/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */


#include <stdio.h>

#include "native_client/src/trusted/debug_stub/debug_packet.h"
#include "native_client/src/trusted/debug_stub/debug_socket_impl.h"
#include "native_client/src/trusted/debug_stub/debug_socket.h"

#include <string>
using std::string;
using namespace nacl_debug_conn;

int VerifyAddrXVert(const string &in_addr, const string &cmp_addr) {
  string out_addr;
  void *nativeAddr;
  uint32_t addrLen;
  char *out_str = new char[MAX_ADDR_LEN];
  int fail = 0;

  DebugSocketAddrSize(&addrLen);
  nativeAddr = malloc(addrLen);

  if (DebugSocketStrToAddr(in_addr.data(), nativeAddr, addrLen) != DSE_OK) {
    printf("Failed to convert string to address.\n");
    fail++;
  }

  if (DebugSocketAddrToStr(nativeAddr, addrLen, out_str, MAX_ADDR_LEN) != DSE_OK) {
    printf("Failed to convert string to address.\n");
    fail++;
  }
  else
    out_addr = out_str;

  if (cmp_addr != out_addr) {
    printf("Failed address convertion %s != %s\n",
      out_addr.data(), cmp_addr.data());
    fail++;
  }

  delete[] out_str;
  return fail;
}

#define TXRX_SIZE 1024
int SocketTest() {
  const char *addr="127.0.0.1:4014";
  DebugSocket *listen = 0;
  DebugSocket *client = 0;  
  DebugSocket *server = 0;
  int failed = 1;

  char *src = new char[TXRX_SIZE];
  char *dst = new char[TXRX_SIZE];
  int tx = 0, rx = 0;

  memset(dst, 0, TXRX_SIZE);
  for (int a = 0; a < TXRX_SIZE; a++)
    src[a] = static_cast<char>(a);

  listen = DebugSocket::CreateServer(addr, 1);
  if (listen == NULL) {
    printf("Failed to listen on %s\n", addr);
    goto SocketFailed;
  }

  client = DebugSocket::CreateClient(addr);
  if (client == NULL) {
    printf("Failed to connect to %s\n", addr);
    goto SocketFailed;
  }

  server = listen->Accept();
  if (server == NULL) {
    printf("Failed to accept on %s\n", addr);
    goto SocketFailed;
  }

  if ((tx = client->Write(src, TXRX_SIZE)) != TXRX_SIZE) {
    printf("Failed to send on client, reported %d bytes.\n", tx);
    goto SocketFailed;
  }

  if ((tx = server->Read(dst, TXRX_SIZE)) != TXRX_SIZE) {
    printf("Failed to read on server, reported %d bytes.\n", rx);
    goto SocketFailed;
  }

  if (memcmp(src, dst, TXRX_SIZE)) {
    printf("RX buffer does not match TX buffer client->server.\n", rx);
    goto SocketFailed;
  }

  if ((tx = server->Write(src, TXRX_SIZE)) != TXRX_SIZE) {
    printf("Failed to send on server, reported %d bytes.\n", tx);
    goto SocketFailed;
  }
  if ((tx = client->Read(dst, TXRX_SIZE)) != TXRX_SIZE) {
    printf("Failed to read on client, reported %d bytes.\n", rx);
    goto SocketFailed;
  }
  if (memcmp(src, dst, TXRX_SIZE)) {
    printf("RX buffer does not match TX buffer server->client.\n", rx);
    goto SocketFailed;
  }

  // Must have passed
  failed = 0;

 SocketFailed:
  delete listen;
  delete client;

  if (server)
    delete server;

  delete[] src;
  delete[] dst;

  return failed;
}


int PacketTest() {
  DebugPacket *pkt = new DebugPacket();
  void *miscPtr = &pkt;
  int failed = 1;
  intptr_t num;
  char ch;

  pkt->Clear();
  pkt->AddRawChar('m');
  pkt->AddPointer(miscPtr);

  if (!pkt->GetRawChar(&ch)) {
    printf("Failed to get packet char.", ch);
    goto PacketFailed;
  }
  if (ch != 'm') {
    printf("Got wrong char 'm' vs '%c'.", ch);
    goto PacketFailed;
  }
  
  if (!pkt->GetPointer(&miscPtr)) {
    printf("Failed to get packet pointer.");
    goto PacketFailed;
  }
  if (miscPtr != &pkt) {
    printf("Got wrong pointer %x vs '%x.", miscPtr, &pkt);
    goto PacketFailed;
  }

  pkt->Clear();
  pkt->AddNumberSep(100,',');
  pkt->AddNumberSep(200, 0);

  if (!pkt->GetNumberSep(&num, &ch)) {
    printf("Failed to get packet pointer.");
    goto PacketFailed;
  }
  if (num != 100) {
    printf("Got wrong number '100' vs '%d'.", num);
    goto PacketFailed;
  }

  if (!pkt->GetNumberSep(&num, &ch)) {
    printf("Failed to get packet pointer.");
    goto PacketFailed;
  }
  if (num != 200) {
    printf("Got wrong number '200' vs '%d'.", num);
    goto PacketFailed;
  }

  failed = 0;
 PacketFailed:
  delete pkt;

  return failed;
}


int main(int argc, char* argv[]) {
  UNREFERENCED_PARAMETER(argc);
  UNREFERENCED_PARAMETER(argv);
  int errors = 0;

  DebugSocketInit();

  // Verify Address translation
  errors += VerifyAddrXVert("", "0.0.0.0:0");
  errors += VerifyAddrXVert("0", "0.0.0.0:0");
  errors += VerifyAddrXVert(":", "0.0.0.0:0");
  errors += VerifyAddrXVert("0:", "0.0.0.0:0");
  errors += VerifyAddrXVert(":0", "0.0.0.0:0");
  errors += VerifyAddrXVert(":4014", "0.0.0.0:4014");
  errors += VerifyAddrXVert("127.0.0.1", "127.0.0.1:0");
  errors += VerifyAddrXVert("127.0.0.1:4014", "127.0.0.1:4014");

  // Verify Socket Interface
  errors += SocketTest();

  // Verify Packet Interface
  errors += PacketTest();

  DebugSocketExit();
  if (errors == 0)
  {
    printf("PASS\n");    
    return 0;
  } 

  printf("FAILED\n");
  return -1;
}
