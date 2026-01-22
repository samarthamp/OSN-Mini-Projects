#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int pid1, pid2;
  
  printf("LBS Test Starting...\n");

  pid1 = fork();
  if(pid1 == 0){
      // CHILD A (10 Tickets)
      settickets(10);
      int start = uptime();
      volatile int count = 0;
      
      // Run for exactly 100 ticks
      while(uptime() - start < 100){ 
          count++; // Count how many times we loop
      }
      printf("Child A (10 tickets) loop count: %d\n", count);
      exit(0);
  }

  pid2 = fork();
  if(pid2 == 0){
      // CHILD B (50 Tickets)
      settickets(50);
      int start = uptime();
      volatile int count = 0;
      
      while(uptime() - start < 100){ 
          count++;
      }
      printf("Child B (50 tickets) loop count: %d\n", count);
      exit(0);
  }

  wait(0);
  wait(0);
  exit(0);
}