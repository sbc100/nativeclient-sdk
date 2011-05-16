// rsp_console.cpp : Defines the entry point for the console application.
//
#include <conio.h>
#include "debugger/base/debug_socket.h"
#include "debugger/rsp/rsp_packetizer.h"
#include "debugger/rsp/rsp_packet_util.h"

const int kReadBufferSize = 1024;
debug::Socket connection;

class RspConsolePacketConsumer : public rsp::PacketConsumer {
 public:
  void OnPacket(const debug::Blob& body, bool valid_checksum);
  void OnUnexpectedChar(char unexpected_char);
  void OnBreak();
};

void RspConsolePacketConsumer::OnPacket(const debug::Blob& body, bool valid_checksum) {
  printf("\nR%s>%s\n", (valid_checksum ? "" : "-checksum-error"), body.ToString().c_str());
  printf(">");
  connection.WriteAll("+", 1);
}

void RspConsolePacketConsumer::OnUnexpectedChar(char unexpected_char) {
  printf("\nR>unexpected [%c]\n", unexpected_char);
  printf(">");
}

void RspConsolePacketConsumer::OnBreak() {
  printf("\nR>Ctrl-C\n");
  printf(">");
}

int main(int argc, char* argv[]) {
  int port = 2345;  // TODO: read theam from command line
  const char* host_name = "172.29.216.11";

  port = 4014;
  host_name = "localhost";

  rsp::Packetizer rsp_packetizer;
  RspConsolePacketConsumer consm;
  rsp_packetizer.SetPacketConsumer(&consm);
  printf(">");

  while (true) {
    if (!connection.IsConnected()) {
      printf("\nConnecting to %s:%d ...", host_name, port);
      connection.ConnectTo(host_name, port);
      printf("%s\n>", connection.IsConnected() ? "Ok" : "Failed");
    }
    else {
      char buff[kReadBufferSize];
      for (int i = 0; i < 100; i++) {
        size_t read_bytes = connection.Read(buff,
                                            sizeof(buff) - 1,
                                            0);
        if (read_bytes > 0) {
          buff[read_bytes] = 0;
          printf("\nr>%s\n", buff);
          rsp_packetizer.OnData(buff, read_bytes);
        } else {
          break;
        }
      }
    }
    
    if (!_kbhit())
      continue;

    char cmd[300] = {0};
    gets_s(cmd, sizeof(cmd));
    if (0 == strcmp(cmd, "quit"))
      break;
    else {
      debug::Blob msg;
      rsp::PacketUtil::AddEnvelope(cmd, &msg);
      //printf("s>%s\n", msg.ToString().c_str());
      if (connection.IsConnected())
        connection.WriteAll(msg);
    }
  }
	return 0;
}

