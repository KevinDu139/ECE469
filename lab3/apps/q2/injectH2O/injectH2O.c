#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
    sem_t s_procs_completed;    // Semaphore to signal the original process that we're done
    mbox_t mb_h2o;                // mailbox for H2O atom

    Printf("Starting H2O Injection - PID %d \n", getpid());

    if (argc > 4 ) { 
        Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_proc_semaphore> <handle_to_H2O_mailbox>\n"); 
        Exit();
    } 

    // Convert the command-line strings into integers for use as handles
    s_procs_completed = dstrtol(argv[1], NULL, 10);
    mb_h2o = dstrtol(argv[2], NULL, 10);
   
    if(!mbox_open(mb_h2o)){
        Printf("Bad mailbox open mb_h2o\n");
        Exit();
    }

    mbox_send(mb_h2o, 3, (void *) "H2O");
    Printf("Sent H2O to mb_h2o\n");

    if(!mbox_close(mb_h2o)){
        Printf("Bad mailbox open mb_h2o\n");
        Exit();
    }

    // Signal the semaphore to tell the original process that we're done
    Printf("Inject H2O: PID %d is complete.\n", getpid());

    if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
        Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
        Exit();
    }
}
