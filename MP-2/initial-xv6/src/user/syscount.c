#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if(argc < 3){
    fprintf(2, "Usage: syscount <mask> <command> [args]\n");
    exit(1);
  }

  int mask = atoi(argv[1]);

  // 1. Enable counting for this process
  if (getSysCount(mask) < 0) {
      fprintf(2, "syscount: failed to set mask\n");
      exit(1);
  }

  // 2. Fork to run the command
  int pid = fork();
  if(pid < 0){
    fprintf(2, "syscount: fork failed\n");
    exit(1);
  }

  if(pid == 0){
    // CHILD
    // Execute the command passed in argv[2]
    exec(argv[2], &argv[2]);
    
    // If exec returns, it failed
    fprintf(2, "syscount: exec failed\n");
    exit(1);
  } else {
    // PARENT
    // Wait for child to exit.
    // The counts will aggregate in the kernel during child exit.
    wait(0);
  }

  // When this program exits, the kernel will detect it is the 
  // top-level tracer and print the result.
  exit(0);
}