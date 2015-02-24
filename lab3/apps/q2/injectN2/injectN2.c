#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
    sem_t s_procs_completed;    // Semaphore to signal the original process that we're done
    mbox_t mb_n2;                // mailbox for H2O atom

    Printf("Starting N2 Injection\n");

    if (argc != 3 ) { 
        Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_proc_semaphore> <handle_to_N2_mailbox>\n"); 
        Exit();
    } 

    // Convert the command-line strings into integers for use as handles
    s_procs_completed = dstrtol(argv[1], NULL, 10);
    mb_n2 = dstrtol(argv[2], NULL, 10);
   
    if(!mbox_open(mb_n2)){
        Printf("Bad mailbox open mb_n2\n");
        Exit();
    }

    mbox_send(mb_n2, 2, (void *) "N2");
    Printf("Sent N2 to mb_n2\n");

    if(!mbox_close(mb_n2)){
        Printf("Bad mailbox open mb_n2\n");
        Exit();
    }

    // Signal the semaphore to tell the original process that we're done
    Printf("Inject N2: PID %d is complete.\n", getpid());

    if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
        Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
        Exit();
    }
}
