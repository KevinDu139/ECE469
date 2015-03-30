#include "usertraps.h"
#include "misc.h"

/******DESCRIPION******/
// Access a memory address outside of virtual memory space.
//  This should cause a segfault and kill the process.

void main (int argc, char *argv[])
{
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
  int* memory_location; // Somewhere in the middle of 1024KB memory space

  if (argc != 2) { 
    Printf("Usage: %s <handle_to_procs_completed_semaphore>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);

  // Print test name
  Printf("Test Out of Bounds Virtual Access (%d): Start\n", getpid());

  // Bad practice but signal the original process first
  // Do this because this process is going to die and we don't
  // want the original process to hand
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Out of Bounds Virtual Access (%d): Bad semaphore s_procs_completed (%d)!\n", getpid(), s_procs_completed);
    Exit();
  }

  // Access address in middle of virtual memory
  // Virtual memory is 1024KB so accessing memory
  // location 0x8000000 is somewhere outside of
  // the virtual memory and should seg fault
  memory_location = 0x8000000;
  Printf("Accessing Memory Location: %d (decimal)\n", memory_location);
  *memory_location = 0xDEADBEEF;

  Printf("Test Out of Bounds Virtual Access (%d): Done!\n", getpid());
}
