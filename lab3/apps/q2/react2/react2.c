#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
    sem_t s_procs_completed;    // Semaphore to signal the original process that we're done
    mbox_t mb_h2o;              // Mailbox for h20 atom
    mbox_t mb_h2;               // Mailbox for h2 atom
    mbox_t mb_o2;               // Mailbox for o2 atom

    char data[3];

    Printf("Reaction 2: starting - PID %d \n", getpid());

    if (argc != 6 ) { 
        Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_proc_semaphore> <handle_to_H2O_mailbox> <handle_to_H2_mailbox> <handle_to_O2_mailbox>\n"); 
        Exit();
    } 

    // Convert the command-line strings into integers for use as handles
    s_procs_completed = dstrtol(argv[1], NULL, 10);
    mb_h2o = dstrtol(argv[2], NULL, 10);
    mb_h2 = dstrtol(argv[3], NULL, 10);
    mb_o2 = dstrtol(argv[4], NULL, 10);

    //open H2O Mailbox
    if(!mbox_open(mb_h2o)){
        Printf("Bad mailbox open mb_h2o\n");
        Exit();
    }

    //open H2 mailbox
    if(!mbox_open(mb_h2)){
        Printf("Bad mailbox open mb_h2\n");
        Exit();
    }

    //open O2 mailbox
    if(!mbox_open(mb_o2)){
        Printf("Bad mailbox open mb_o2\n");
        Exit();
    }

    mbox_recv(mb_h2o, 3, (char *) data);
    Printf("Recv H2O from mb_h2o\n");
    mbox_recv(mb_h2o, 3, (char *) data);
    Printf("Recv H2O from mb_h2o\n");

    mbox_send(mb_h2, 2, (void *) "H2");
    Printf("Sent H2 to mb_h2\n");
    mbox_send(mb_h2, 2, (void *) "H2");
    Printf("Sent H2 to mb_h2\n");
    
    mbox_send(mb_o2, 2, (void *) "O2");
    Printf("Sent O2 to mb_o2\n");

    //close H2O Mailbox
    if(!mbox_close(mb_h2o)){
        Printf("Bad mailbox close mb_h2o\n");
        Exit();
    }

    //close H2 mailbox
    if(!mbox_close(mb_h2)){
        Printf("Bad mailbox close mb_h2\n");
        Exit();
    }

    //close O2 mailbox
    if(!mbox_close(mb_o2)){
        Printf("Bad mailbox close mb_o2\n");
        Exit();
    }

    // Signal the semaphore to tell the original process that we're done
    Printf("Reaction 2: PID %d is complete.\n", getpid());

    if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
        Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
        Exit();
    }
}
