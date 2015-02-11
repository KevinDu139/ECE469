
#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
    sem_t s_procs_completed;    // Semaphore to signal the original process that we're done
    sem_t s_n2;                // Semaphore for N2 atom
    sem_t s_n;                 // Semaphore for N atom

    int numReactions=0;           // Number of reactions to complete
    int currReactions=0;          // Number of currently completed reactions

    int i;                      // Loop counter

    Printf("Reaction 1: starting\n");

    if (argc != 6 ) { 
        Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_proc_semaphore> <handle_to_N2_semaphore> <handle_to_N_semaphore> <number of reactions to complete> \n"); 
        Exit();
    } 

    // Convert the command-line strings into integers for use as handles
    s_procs_completed = dstrtol(argv[1], NULL, 10);
    s_n2 = dstrtol(argv[2], NULL, 10);
    s_n = dstrtol(argv[3], NULL, 10);
    numReactions = dstrtol(argv[4], NULL, 10);


    while(currReactions < numReactions) {
           
        //Wait until there is N2 available
            if(sem_wait(s_n2) != SYNC_SUCCESS) {
                Printf("Bad Semaphore s_n2\n");
                Exit();
            }

        //Split 2x N
        for(i=0; i < 2; i++) {
            //Signal additional N
            if(sem_signal(s_n) != SYNC_SUCCESS) {
                Printf("Bad semaphore s_n\n");
                Exit();
            }

            Printf("Reaction 1: N produced\n");
        }


        currReactions++;

    }

    // Signal the semaphore to tell the original process that we're done
    Printf("Reaction 1: PID %d is complete.\n", getpid());

    if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
        Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
        Exit();
    }
}
