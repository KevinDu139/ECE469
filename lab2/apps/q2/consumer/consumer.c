
#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
  buffer_l *ring_buffer;           // Used to access circular buffer in shared memory page
  uint32 h_mem;            // Handle to the shared memory page
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
  lock_t buffer_lock;      // Common lock to shared memory buffer
  char text[] = "Hello World";
  int currchar = 0;

Printf("got to consumer\n");
  if (argc != 4) { 
    Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_shared_memory_page> <handle_to_page_mapped_semaphore>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  h_mem = dstrtol(argv[1], NULL, 10); // The "10" means base 10
  s_procs_completed = dstrtol(argv[2], NULL, 10);
  buffer_lock = dstrtol(argv[3], NULL, 10);


  // Map shared memory page into this process's memory space
  if ((ring_buffer = (buffer_l *)shmat(h_mem)) == NULL) {
    Printf("Could not map the virtual address to the memory in "); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }

  while(currchar < BUFFERSIZE) {
	  // Synchronize with locks
	  if (lock_acquire(buffer_lock) != SYNC_SUCCESS) {
	    Printf("Bad lock acquire (%d) in ", buffer_lock); Printf(argv[0]); Printf("\n");
	    Exit();
	  } 

    // Check buffer if empty
    if((ring_buffer->endptr) != (ring_buffer->currptr)) {

	// remove character from buffer
	Printf("Consumer %d removed: %c\n", getpid(), ring_buffer->buffer[ring_buffer->currptr]);
        
	// Increment buffer pointer
        ring_buffer->currptr = (ring_buffer->currptr + 1) % BUFFERSIZE;
      
	// Increment character string counter
      	currchar++;
    }

	  // Release lock
	  if (lock_release(buffer_lock) != SYNC_SUCCESS) {
	    Printf("Bad lock release (%d) in ", buffer_lock); Printf(argv[0]); Printf("\n");
	    Exit();
    }

  }
 
  // Signal the semaphore to tell the original process that we're done
  Printf("consumer: PID %d is complete.\n", getpid());
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
}
