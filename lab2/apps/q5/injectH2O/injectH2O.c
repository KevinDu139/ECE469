
#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
    sem_t s_procs_completed;    // Semaphore to signal the original process that we're done
    sem_t s_h2o;                // Semaphore for H2O atom

    int h2o_count;
    int i;

    Printf("Starting H2O Injection\n");

    if (argc != 4 ) { 
        Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_proc_semaphore> <handle_to_H2O_semaphore> <Number_of_H2O_to_inject>\n"); 
        Exit();
    } 

    // Convert the command-line strings into integers for use as handles
    s_procs_completed = dstrtol(argv[1], NULL, 10);
    s_h2o = dstrtol(argv[2], NULL, 10);
    h2o_count = dstrtol(argv[3], NULL, 10);
    
    for(i = 0; i < h2o_count; i++){

        //Signal we have injected N2
        if(sem_signal(s_h2o) != SYNC_SUCCESS) {
            Printf("Bad semaphore s_h2o\n");
            Exit();
        }
        Printf("We have Injected H2O\n");
    }

    // Signal the semaphore to tell the original process that we're done
    Printf("Inject H2O: PID %d is complete.\n", getpid());

    if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
        Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
        Exit();
    }
}
