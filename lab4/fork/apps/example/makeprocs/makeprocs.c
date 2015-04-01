#include "usertraps.h"
#include "misc.h"


void main (int argc, char *argv[])
{

  int child_pid;

  Printf ("the main program process ID is %d\n", (int) getpid ());
  child_pid = fork();
  if (child_pid != 0) {
    Printf ("this is the parent process, with id %d\n", (int) getpid ());
    Printf ("the child's process ID is %d\n", child_pid);
  } else {
    Printf ("this is the child process, with id %d\n", (int) getpid ());
  }





}
