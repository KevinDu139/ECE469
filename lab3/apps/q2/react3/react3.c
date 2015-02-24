#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
    sem_t s_procs_completed;    // Semaphore to signal the original process that we're done
    mbox_t mb_n;                // Mailbox for n atom
    mbox_t mb_o2;               // Mailbox for o2 atom
    mbox_t mb_no2;              // Mailbox for no2 atom

    char data[3];

    Printf("Reaction 3: starting\n");

    if (argc != 5 ) { 
        Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_proc_semaphore> <handle_to_N_mailbox> <handle_to_O2_mailbox> <handle_to_NO2_mailbox>\n"); 
        Exit();
    } 

    // Convert the command-line strings into integers for use as handles
    s_procs_completed = dstrtol(argv[1], NULL, 10);
    mb_n = dstrtol(argv[2], NULL, 10);
    mb_o2 = dstrtol(argv[3], NULL, 10);
    mb_no2 = dstrtol(argv[4], NULL, 10);

    //open N Mailbox
    if(!mbox_open(mb_n)){
        Printf("Bad mailbox open mb_n\n");
        Exit();
    }

    //open O2 mailbox
    if(!mbox_open(mb_o2)){
        Printf("Bad mailbox open mb_o2\n");
        Exit();
    }

    //open NO2 mailbox
    if(!mbox_open(mb_no2)){
        Printf("Bad mailbox open mb_no2\n");
        Exit();
    }

    mbox_recv(mb_n, 1, (char *) data);
    Printf("Recv N from mb_n\n");

    mbox_recv(mb_o2, 2, (char *) data);
    Printf("Recv O2 from mb_o2\n");

    mbox_send(mb_no2, 3, (void *) "NO2");
    Printf("Sent NO2 to mb_no2\n");
    

    //close N Mailbox
    if(!mbox_close(mb_n)){
        Printf("Bad mailbox close mb_n\n");
        Exit();
    }

    //close O2 mailbox
    if(!mbox_close(mb_o2)){
        Printf("Bad mailbox close mb_o2\n");
        Exit();
    }

    //close NO2 mailbox
    if(!mbox_close(mb_no2)){
        Printf("Bad mailbox close mb_no2\n");
        Exit();
    }

    // Signal the semaphore to tell the original process that we're done
    Printf("Reaction 3: PID %d is complete.\n", getpid());

    if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
        Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
        Exit();
    }
}
