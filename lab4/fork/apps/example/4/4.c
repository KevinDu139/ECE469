#include "usertraps.h"
#include "misc.h"

//*******DESCRIPTION******//
// Test program to push stack past 1 page. Should allocate a new page
// without seg faulting or killing the process. Do this with a
// recursive program in this case calculating factorial

int factorial(int n)
{
  if(n == 0) { return 1; }
  return (1 + factorial(n - 1));
}

void main (int argc, char *argv[])
{
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
  int n = 1000; // Number to calculate factorial

  if (argc != 2) { 
    Printf("Usage: %s <handle_to_procs_completed_semaphore>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);

  // State test case
  Printf("Test Stack Growth (%d): Start\n", getpid());

  // Call recursive program with a very large number of recursions
  Printf("Calling Recursive Function %d\n", n);
  n = factorial(n);

  // Print Result
  Printf("%d Recursive Calls Made\n", n);

  // Signal the semaphore to tell the original process that we're done
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("hello_world (%d): Bad semaphore s_procs_completed (%d)!\n", getpid(), s_procs_completed);
    Exit();
  }

  Printf("Stack Growth Test (%d): Done!\n", getpid());
}
