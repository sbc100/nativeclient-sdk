#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/nacl_syscalls.h>

//#define CRASH_IT

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

int print_loop() {
  int count = 0;
  timespec t = {1,0};

#ifdef CRASH_IT
  char *ptr = NULL;

  if (*ptr == 0)
    return 1;
#endif

  while (1) {
    print_line(count);
    ++count;
    nanosleep(&t,0);
  }

  return 2;
}

int foo() {
  if (print_loop())
    return 0;

  return 1;
}

class MyFooBarClass {
public:
  int MyFooVar;
  int MyBarVar;
};

int main(int argc, const char *argv[]){
  MyFooBarClass fbclass;

  fbclass.MyFooVar = 0;
  printf("Hello World\n");
  g_GlobalData++;
  foo();
  return 0;
}
