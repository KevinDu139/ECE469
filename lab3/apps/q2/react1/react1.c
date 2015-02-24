#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
    sem_t s_procs_completed;    // Semaphore to signal the original process that we're done
    mbox_t mb_n2;               // Mailbox for N2 atom
    mbox_t mb_n;                // Mailbox for N atom

    char data[3];

    Printf("Reaction 1: starting\n");

    if (argc != 4 ) { 
        Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_proc_semaphore> <handle_to_N2_mailbox> <handle_to_N_mailbox>\n"); 
        Exit();
    } 

    // Convert the command-line strings into integers for use as handles
    s_procs_completed = dstrtol(argv[1], NULL, 10);
    mb_n2 = dstrtol(argv[2], NULL, 10);
    mb_n = dstrtol(argv[3], NULL, 10);

    //open N2 Mailbox
    if(!mbox_open(mb_n2)){
        Printf("Bad mailbox open mb_n2\n");
        Exit();
    }

    //open N mailbox
    if(!mbox_open(mb_n)){
        Printf("Bad mailbox open mb_n\n");
        Exit();
    }

    mbox_recv(mb_n2, 2, (char *) data);
    Printf("Recv N2 from mb_n2\n");

    mbox_send(mb_n, 1, (void *) "N");
    mbox_send(mb_n, 1, (void *) "N");

    Printf("Sent N to mb_n\n");

    if(!mbox_close(mb_n2)){
        Printf("Bad mailbox open mb_n2\n");
        Exit();
    }

    if(!mbox_close(mb_n)){
        Printf("Bad mailbox open mb_n\n");
        Exit();
    }

    // Signal the semaphore to tell the original process that we're done
    Printf("Reaction 1: PID %d is complete.\n", getpid());

    if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
        Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
        Exit();
    }
}
