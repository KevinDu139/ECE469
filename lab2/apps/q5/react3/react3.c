
#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
    sem_t s_procs_completed;    // Semaphore to signal the original process that we're done
    sem_t s_n;                  // Semaphore for N atom
    sem_t s_o2;                 // Semaphore for O2 atom
    sem_t s_no2;                // Semaphore for NO2 atom

    int numReactions =0;           // Number of reactions to complete
    int currReactions =0;          // Current number of reactions completed

    Printf("Reaction 3: starting\n");

    if (argc != 6 ) { 
        Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_proc_semaphore> <handle_to_N_semaphore> <handle_to_O2_semaphore> <handle_to_NO2_semaphore> <number of reactions to complete>\n"); 
        Exit();
    } 

    // Convert the command-line strings into integers for use as handles
    s_procs_completed = dstrtol(argv[1], NULL, 10);
    s_n = dstrtol(argv[2], NULL, 10);
    s_o2 = dstrtol(argv[3], NULL, 10);
    s_no2 = dstrtol(argv[4], NULL, 10);
    numReactions = dstrtol(argv[5], NULL, 10);


    while(currReactions < numReactions) {
           
        //Wait until N is available
        if(sem_wait(s_n) != SYNC_SUCCESS) {
            Printf("Bad Semaphore s_n\n");
            Exit();
        }

        //Wait until O2 is available
        if(sem_wait(s_o2) != SYNC_SUCCESS) {
            Printf("Bad Semaphore s_o2\n");
            Exit();
        }

        //Signal created NO2
        if(sem_signal(s_no2) != SYNC_SUCCESS) {
            Printf("Bad semaphore s_no2\n");
            Exit();
        }

        Printf("Reaction 3: NO2 produced\n");

        currReactions++;
    }

    // Signal the semaphore to tell the original process that we're done
    Printf("Reaction 3: PID %d is complete.\n", getpid());

    if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
        Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
        Exit();
    }
}
