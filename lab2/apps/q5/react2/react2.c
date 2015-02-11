
#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
    sem_t s_procs_completed;    // Semaphore to signal the original process that we're done
    sem_t s_h2o;                // Semaphore for H2O atom
    sem_t s_h2;                 // Semaphore for H2 atom
    sem_t s_o2;                 // Semaphore for O2 atom

    int numReactions =0 ;           // Number of reactions to complete
    int currReactions =0;          // Number of currently completed reactions

    int i;                      // Loop counter

    Printf("Reaction 2: starting\n");

    if (argc != 6 ) { 
        Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_proc_semaphore> <handle_to_H2O_semaphore> <handle_to_H2_semaphore> <handle_to_O2_semaphore> <number of reactions to complete> \n"); 
        Exit();
    } 

    // Convert the command-line strings into integers for use as handles
    s_procs_completed = dstrtol(argv[1], NULL, 10);
    s_h2o = dstrtol(argv[2], NULL, 10);
    s_h2 = dstrtol(argv[3], NULL, 10);
    s_o2 = dstrtol(argv[4], NULL, 10);
    numReactions = dstrtol(argv[5], NULL, 10);


    while(currReactions < (numReactions/2)) {
           
        //Wait until there is 2x H2O available
        for(i=0; i < 2; i++) {
            if(sem_wait(s_h2o) != SYNC_SUCCESS) {
                Printf("Bad Semaphore s_h2o\n");
                Exit();
            }
        }

        //Split 2x H2O into 2x H2 and O2
        for(i=0; i < 2; i++) {
            //Signal additional H2
            if(sem_signal(s_h2) != SYNC_SUCCESS) {
                Printf("Bad semaphore s_h2\n");
                Exit();
            }

            Printf("Reaction 2: H2 produced\n");
        }

        if(sem_signal(s_o2) != SYNC_SUCCESS) {
            Printf("Bad semaphore s_o2\n");
            Exit();
        }

        Printf("Reaction 2: O2 produced\n");

        currReactions++;

    }

    // Signal the semaphore to tell the original process that we're done
    Printf("Reaction 2: PID %d is complete.\n", getpid());

    if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
        Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
        Exit();
    }
}
