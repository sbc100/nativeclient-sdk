#include <stdio.h>
#include <time.h>
#include <string>
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

// We define |print_char_type| so that unit tests can validate
// getting the value of a 1-byte character using functions such as
// SymbolValueToString.
void print_char_type() {
  char c = 'I';
  printf("c=%c\n", c);
  fflush(stdout);
}

int main(int argc, const char *argv[]){
  int x = 0;
  int y = 9;
  std::string test_string;
  printf("Hello World\n\n x: %d y: %d", x, y);
  g_GlobalData++;
  foo();
  print_char_type();
  return 0;
}
