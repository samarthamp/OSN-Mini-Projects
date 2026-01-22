#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  printf("MLFQ Test Starting...\n");
  printf("1. I will spin now. Quickly press Ctrl+P repeatedly to watch my Priority drop.\n");
  printf("2. Wait approx 48-50 ticks to see Priority Boost back to 0.\n");

  int pid = getpid();
  
  volatile long x = 0; 
  
  // Spin loop
  for(long i = 0; i < 500000000; i++) {
      x = x + 1; 
      
      // Print occasionally to show it is alive and USE the pid variable
      if(i % 10000000 == 0) {
          printf("PID %d is spinning... (x=%d)\n", pid, x);
      }
  }

  printf("Test finished.\n");
  exit(0);
}