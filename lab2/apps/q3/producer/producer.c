
#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
    buffer_l *ring_buffer;           // Used to access circular buffer in shared memory page
    uint32 h_mem;            // Handle to the shared memory page
    sem_t s_procs_completed; // Semaphore to signal the original process that we're done
    sem_t s_fullslots;
    sem_t s_emptyslots;
    lock_t buffer_lock;

    char text[] = "Hello World";
    int currchar = 0;

    Printf("got to producer\n");
    if (argc != 6) { 
        Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_shared_memory_page> <handle_to_page_mapped_semaphore>\n"); 
        Exit();
    } 

    // Convert the command-line strings into integers for use as handles
    h_mem = dstrtol(argv[1], NULL, 10); // The "10" means base 10
    s_procs_completed = dstrtol(argv[2], NULL, 10);

    // Lock conversion
    buffer_lock = dstrtol(argv[3], NULL, 10);

    // Semaphore conversion
    s_fullslots = dstrtol(argv[4], NULL, 10);
    s_emptyslots = dstrtol(argv[5], NULL, 10);
    
    // Map shared memory page into this process's memory space
    if ((ring_buffer = (buffer_l *)shmat(h_mem)) == NULL) {
        Printf("Could not map the virtual address to the memory in "); Printf(argv[0]); Printf(", exiting...\n");
        Exit();
    }

    while(currchar < BUFFERSIZE) {
        // Wait for available empty slots
        if(sem_wait(s_emptyslots) != SYNC_SUCCESS) {
            Printf("Bad semaphore s_emptyslots (%d) in PID: %d \n", s_emptyslots, getpid());
            Exit();
        }

        // Wait for lock on buffer
	if (lock_acquire(buffer_lock) != SYNC_SUCCESS) {
	  Printf("Bad lock acquire (%d) in ", buffer_lock); Printf(argv[0]); Printf("\n");
	  Exit();
	} 

	// Write character to buffer
	ring_buffer->buffer[ring_buffer->endptr] = text[currchar];

	// Increment buffer pointer
	ring_buffer->endptr = (ring_buffer->endptr + 1) % BUFFERSIZE;

	// Increment character string counter
	Printf("Producer %d inserted: %c\n", getpid(), text[currchar]);
	currchar++;

	// Release lock
	if (lock_release(buffer_lock) != SYNC_SUCCESS) {
	  Printf("Bad lock release (%d) in ", buffer_lock); Printf(argv[0]); Printf("\n");
	  Exit();
	}

	// Signal character has been added to buffer       
        if(sem_signal(s_fullslots) != SYNC_SUCCESS) {
            Printf("Bad semaphore s_fullslots\n");
            Exit();
        }
    }

    // Signal the semaphore to tell the original process that we're done
    Printf("producer: PID %d is complete.\n", getpid());

    if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
        Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
        Exit();
    }
}
