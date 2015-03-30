#include "usertraps.h"
#include "misc.h"

#define HELLO_WORLD "hello_world.dlx.obj"
#define prog1   "1.dlx.obj"
#define prog2   "2.dlx.obj"
#define prog3   "3.dlx.obj"
#define prog4   "4.dlx.obj"
#define prog5   "5.dlx.obj"
#define prog6   "6.dlx.obj"


void main (int argc, char *argv[])
{
    int i;                               // Loop index variable
    sem_t s_procs_completed;             // Semaphore used to wait until all spawned processes have completed
    char s_procs_completed_str[10];      // Used as command-line argument to pass page_mapped handle to new processes
    int program;                        // used to figure out which program to run

    if (argc != 2) {
        Printf("Usage: %s <which program to run (1-6) or all of them (0) \n", argv[0]);
        Exit();
    }

    // Convert string from ascii command line argument to integer number
    program = dstrtol(argv[1], NULL, 10); // the "10" means base 10

    // Create semaphore to not exit this process until all other processes 
    // have signalled that they are complete.
    if ((s_procs_completed = sem_create(0)) == SYNC_FAIL) {
        Printf("makeprocs (%d): Bad sem_create\n", getpid());
        Exit();
    }

    // Setup the command-line arguments for the new processes.  We're going to
    // pass the handles to the semaphore as strings
    // on the command line, so we must first convert them from ints to strings.
    ditoa(s_procs_completed, s_procs_completed_str);

    if(program == 1 || program == 0){
        // part 1 - create single hello world process
        Printf("-------------------------------------------------------------------------------------\n");

        Printf("makeprocs (%d): PART 1\n", getpid());
        Printf("makeprocs (%d): Creating a single hello world\n", getpid());
        process_create(prog1, s_procs_completed_str, NULL);
        if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
            Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
            Exit();
        }

        Printf("-------------------------------------------------------------------------------------\n");
    }

    if(program == 2 || program == 0){
    // part 2 - accessing memory beyond the max virtual address
    Printf("-------------------------------------------------------------------------------------\n");

    Printf("makeprocs (%d): PART 2\n", getpid());
    Printf("makeprocs (%d): Accessing memory beyond the max virtual address\n", getpid());
    process_create(prog2, s_procs_completed_str, NULL);
    if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
        Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
        Exit();
    }

    Printf("-------------------------------------------------------------------------------------\n");
    }


    if(program == 3 || program == 0){
    // part 3 - access memory inside the virtual address space but outside currently allocated space

    Printf("-------------------------------------------------------------------------------------\n");

    Printf("makeprocs (%d): PART 3\n", getpid());
    Printf("makeprocs (%d): Accessing memory inside the virtual address space but outside currently allocated space\n", getpid());
    process_create(prog3, s_procs_completed_str, NULL);
    if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
        Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
        Exit();
    }

    Printf("-------------------------------------------------------------------------------------\n");
    }


    if(program == 4 || program == 0){
    // part 4 - Causing user call stack to grow larger than one page
    Printf("-------------------------------------------------------------------------------------\n");

    Printf("makeprocs (%d): PART 4\n", getpid());
    Printf("makeprocs (%d): Causing user call stack to grow larger than one page\n", getpid());
    process_create(prog4, s_procs_completed_str, NULL);
    if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
        Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
        Exit();
    }

    Printf("-------------------------------------------------------------------------------------\n");
    }


    if(program == 5 || program == 0){
    // part 5 - Caling hello world 100 times
    Printf("-------------------------------------------------------------------------------------\n");

    Printf("makeprocs (%d): PART 5\n", getpid());
    Printf("makeprocs (%d): Calling hello world 100 times\n", getpid());

    for(i=0; i<100; i++) {
        Printf("makeprocs (%d): Creating hello world #%d\n", getpid(), i);
        process_create(prog5, s_procs_completed_str, NULL);
        if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
            Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
            Exit();
        }
    }

    Printf("-------------------------------------------------------------------------------------\n");
    }


    if(program == 6 || program == 0){
    // part 6 - 30 Process memory test
    Printf("-------------------------------------------------------------------------------------\n");

    Printf("makeprocs (%d): PART 6\n", getpid());
    Printf("makeprocs (%d): 30 Process memory stress test\n", getpid());

    for(i=0; i<30; i++) {
        Printf("makeprocs (%d): Creating memory stress test program #%d\n", getpid(), i);
        process_create(prog6, s_procs_completed_str, NULL);
    }
    if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
        Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
        Exit();
    }

    Printf("-------------------------------------------------------------------------------------\n");
    }


    Printf("makeprocs (%d): All other processes completed, exiting main process.\n", getpid());

}
