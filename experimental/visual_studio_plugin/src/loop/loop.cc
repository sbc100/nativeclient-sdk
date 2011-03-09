#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/nacl_syscalls.h>
int g_GlobalData = 0;
void print_line(int count) {
  printf("Running");
  int i = 0;
  for (i = 0; i < count % 5; ++i) {
    printf(".");
  }
  for (; i < 5; ++i) {
    printf(" ");
  }
  printf("\r");
  fflush(stdout);
}

void print_loop() {
  int count = 0;
  timespec t = {1,0}; 

  while (1) {
    print_line(count);
    ++count;
    nanosleep(&t,0);
  }
}

void foo() {
  print_loop();
}

int main(int argc, const char *argv[]){
  printf("Hello World\n\n");
  g_GlobalData++;
  foo();
  return 0;
}
