#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <errno.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>

#define bool int
#define false 0
#define true 1


#define MAX_ADDR_LEN 256

int sockTarget = -1;
int sockListen = -1;
int sockHost = -1;


static int AllocTokens(const char *in, char delim, char *out[], int max) {
  char *str   =strdup(in);
  char *start= str;
  char *word = str;
  int  cnt = 0;

  for (;*str; str++) {
    if (*str == delim) {

      // Make this null, so we can copy it
      *str = 0;

      // Add it to the array;
      if (cnt < max)
        out[cnt++] = strdup(word);

      // Start scanning after the delim
      str++;
      word = str;
    }
  }

  if (*word)
    if (cnt < max)
      out[cnt++] = strdup(word);

  free(start);
  return cnt;
}

static void FreeTokens(char *strings[], int max) {
  int cnt = 0;
  for (cnt = 0; cnt < max; cnt++) {
    if (strings[cnt]) {
      free(strings[cnt]);
      strings[cnt] = 0;
    }
  }
}


bool DebugSocketStrToAddr(const char *saddr, void *daddr, uint32_t len) {
  struct sockaddr_in *saddr_in = (struct sockaddr_in *) daddr;
  char *ip_port[2];
  int ip_port_cnt = 0;

  char *octets[4];
  int octets_cnt = 0;

  unsigned host = 0;
  unsigned port = 0;

  if (len != sizeof(struct sockaddr_in))
    return false;

  ip_port_cnt = AllocTokens(saddr, ':', ip_port, 2);
  if (ip_port_cnt > 0) {
    octets_cnt = AllocTokens(ip_port[0], '.', octets, 4);
    if (4 == octets_cnt) {
      int a;
      for (a = 0; a < 4; a++) {
         host <<= 8;
         host |= (atoi(octets[a]) & 0xFF);
      }
    }
    FreeTokens(octets, octets_cnt);
  }
  if (ip_port_cnt > 1) {
    port = atoi(ip_port[1]);
  }
  FreeTokens(ip_port, ip_port_cnt);

  saddr_in->sin_family = AF_INET;
  saddr_in->sin_addr.s_addr = htonl(host);
  saddr_in->sin_port = htons(port);

  return true;
}


int DebugSocketAddrToStr(void *saddr, uint32_t len, char *daddr, uint32_t max) {
  char tmp[MAX_ADDR_LEN];
  struct sockaddr_in *saddr_in = (struct sockaddr_in *)(saddr);

  unsigned int host;
  unsigned int port;

  if (len != sizeof(struct sockaddr_in))
    return false;

  host = htonl(saddr_in->sin_addr.s_addr);
  port = htons(saddr_in->sin_port);

  if (snprintf(tmp,
          sizeof(tmp),
          "%d.%d.%d.%d:%d",
          (host >> 24) & 0xFF,
          (host >> 16) & 0xFF,
          (host >>  8) & 0xFF,
          (host >>  0) & 0xFF,
          port) > (int) max)
    return false;

  strcpy(daddr, tmp);
  return true;
}



bool bindSock(int sock, const char *str) {
  struct sockaddr_in saddr;
  socklen_t addrlen = sizeof(saddr);

  DebugSocketStrToAddr(str, &saddr, addrlen);
  if (bind(sock, (struct sockaddr *) &saddr, addrlen)) {
    printf("Failure in bind.\n");
    return false;
  }
  return true;
}

bool connSock(int sock, const char *str) {
  struct sockaddr_in saddr;
  socklen_t addrlen = (socklen_t) sizeof(saddr);

  if (DebugSocketStrToAddr(str, &saddr, addrlen) != true)
    return false;

  // Check if we WOULDBLOCK
  if (connect(sock, (struct sockaddr *) &saddr, sizeof(saddr)))
    return false;

  return true;
}

void sockClose(int *sockid) {
    shutdown(*sockid, SHUT_RDWR);
    close(*sockid);
    *sockid = -1;
}

int sockAvail(int *sockid) {
  struct timeval timeout;
  fd_set fds;
  int cnt;

  // We want a "non-blocking" check
  timeout.tv_sec = 0;
  timeout.tv_usec= 0;
  
  FD_ZERO(&fds);
  FD_SET((*sockid), &fds);
  cnt = select((*sockid)+1, 0, 0, &fds, &timeout);
  if (cnt != 0) {
    sockClose(sockid);
    return -1;
  }

  // Check if this file handle can select on read
  timeout.tv_sec = 0;
  timeout.tv_usec= 1000;

  FD_ZERO(&fds);
  FD_SET((*sockid), &fds);
  cnt = select((*sockid+1), &fds, 0, 0, &timeout);
  if (-1 == cnt) {
    sockClose(sockid);
    return -1;
  }

  if (cnt > 0)
    return 1;

  return 0;
}

#define MAX_FILTERS 20
const char *filters[MAX_FILTERS];
int filterLens[MAX_FILTERS];
int filterCnt = 0;
int RxTxCnt = 0;

void RxTx(int *in, int *out, const char *str) {
  int errs = 0;
  int len = 0;
  char tmp[65536];

  while (sockAvail(in) != 0) {
    if (*in == -1) 
      return;

    if (recv(*in, &tmp[len], 1, 0) == 1)
      len++;
    else {
      sockClose(in);
      return;
    }
  } 

  if (*out == -1)
    return;

  tmp[len] = 0;
  if (len) {
    int a, start = 0;
    RxTxCnt++;
    if (tmp[0] == '+')
      start++;
    if (tmp[start] == '$')
      start++;

    for (a=0; a < filterCnt; a++) {
      if (!strncmp(&tmp[start], filters[a], filterLens[a])) {
        printf("[%4d] %s IGNORE '%s'\n", RxTxCnt, str, tmp);
        send(*in, "+$#00", 5, 0);
        return;
      }
    }   
    printf("[%4d] %s %s\n", RxTxCnt, str, tmp);
    if (send(*out, tmp, len, 0) != len)
    {
      sockClose(in);
      errs++;
    }
  }  
}

void help() {
  printf("proxy {l|L|c|C}[ip_addr}:port {l|L|c|C}[ip_addr}:port\n");
  printf("  proxy L:4015 C127.0.0.1:4014 - Listen on 4015, and connect to 4014\n");
  printf("  proxy L:4014 L:4015 - Listen on 41014 and 4015 \n");
  printf("  proxy C127.0.0.1:4014 C127.0.0.1:4015 - Connect to 41014 and 4015 \n");
  printf("\n");
}

int main(int argc, const char *argv[]) {
  int bad; 
  int dirs = 0;
  int list[2] = { 0, 0};
  int conn[2] = { -1, -1};
  int a;
  const char *addr[2];
 
  for (a=1; a < argc; a++) {
    switch(argv[a][0]) {
      case 'l':
      case 'L':
        addr[dirs] = &argv[a][1];
        list[dirs] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (bindSock(list[dirs], addr[dirs]) == false) {
          printf("Failed to bind to '%s'.\n", addr[dirs]);
          goto failed;
        }
        if (listen(list[dirs], 1)) {
          printf("Failed to listen on '%s'.\n", addr[dirs]);
          goto failed;
        }
        printf("Listening on '%s'.\n", addr[dirs]);
        dirs++;
        break;

      case 'c':
      case 'C':
        addr[dirs] = &argv[a][1];
        dirs++;
        break;

      case '-':
        if (filterCnt == MAX_FILTERS) {
          printf("Ignoring filter, max exceeded.\n");
          continue;
        }
        filters[filterCnt] = &argv[a][1];
        filterLens[filterCnt] = strlen(filters[filterCnt]);
        printf("Adding filter: %s\n", filters[filterCnt]);
        filterCnt++;
        break;

      default:
        help();
        exit(-1);
    }
  }
  if (dirs != 2) {
    printf("Expecting two sockets.\n");
    help();
    exit(-1);
  }

  printf("Processing.\n");
  while (1) {

    for (a=0; a < 2; a++) {
      struct sockaddr_in saddr;
      socklen_t addrlen = sizeof(saddr); 
      memset(&saddr, 0, sizeof(saddr));
      saddr.sin_family = AF_INET;

      // We already have a socket
      if (conn[a] != -1)
        continue;

      if (list[a]) {
        printf("Waiting for connection on '%s'.\n", addr[a]);
        conn[a] = accept(list[a], (struct sockaddr *) &saddr, &addrlen);
        if (conn[a] == -1) {
          printf("failed to accept with %d\n", errno);
          continue;
        }
      }
      else {
        printf("Trying to connect to '%s'.\n", addr[a]);
        conn[a] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (connSock(conn[a], addr[a]) == false) {
          printf("Failed to connect to '%s'.\n", addr[a]);
          sockClose(&conn[a]);
          continue;
        }
        printf("Connected to '%s'.\n", addr[a]);
      }
    }

    if((conn[0] != -1) && (conn[1] != -1)) {
      printf("GO....\n");
      while ((conn[0] != -1) && (conn[1] != -1)) {
        RxTx(&conn[0], &conn[1], "A->B");
        RxTx(&conn[1], &conn[0], "B->A");
      }
      printf("STOP....\n");
    }
    else {
      sleep(5);
      printf("\n");
    }
  }

failed:
  for (a=0; a < 2; a++)
  {
    if (conn[a])
      close(conn[a]);
    if (list[a])
      close(list[a]);
  }
  return 0;
}



  
