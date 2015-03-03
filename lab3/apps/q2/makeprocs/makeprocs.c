#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
  //int numprocs = 0;               // Used to store number of processes to create
  //int i;                          // Loop index variable
  int pnice = 0;
  int pinfo = 1;

  // Number of molecules to spawn and react
  int num_n2;
  int num_h2o;
  int num_limit;
  int num_molecules; // Process creator control variable

  // Completed processes semaphore
  sem_t s_procs_completed;

  // Molecule mailboxes
  mbox_t mb_n2;
  mbox_t mb_n;
  mbox_t mb_h2o;
  mbox_t mb_h2;
  mbox_t mb_o2;
  mbox_t mb_no2;

  // Number of molecule strings
  //char num_n2_str[10];
  //char num_h2o_str[10];
  //char num_limit_str[10];

  // Semaphore argument strings
  char s_procs_completed_str[10]; // Used as command-line argument to pass page_mapped handle to new processes
  char mb_n2_str[10];
  char mb_h2o_str[10];
  char mb_h2_str[10];
  char mb_n_str[10];
  char mb_o2_str[10];
  char mb_no2_str[10];
	
  if (argc != 3) {
    Printf("Usage: "); Printf(argv[0]); Printf(" <number of N2 molecules> <number of H2O molecules\n");
    Exit();
  }


  // Convert string from ascii command line argument to integer number
  //numprocs = dstrtol(argv[1], NULL, 10); // the "10" means base 10
  num_n2 = dstrtol(argv[1], NULL, 10);
  num_h2o = dstrtol(argv[2], NULL, 10);
  Printf("Creating processes\n");

  // Calculate number of molecules that will be generated for the
  // reactor processes to know when to stop
  if((num_n2*2) < (num_h2o/2)) {
    num_limit = num_n2*2;
  } else {
    num_limit = num_h2o/2;
  }

  // Allocate space for a shared memory page, which is exactly 64KB
  // Note that it doesn't matter how much memory we actually need: we 
  // always get 64KB
  //if ((h_mem = shmget()) == 0) {
  //Printf("ERROR: could not allocate shared memory page in "); Printf(argv[0]); Printf(", exiting...\n");
  //Exit();
  //}

  // Map shared memory page into this process's memory space
  //if ((mc = (missile_code *)shmat(h_mem)) == NULL) {
  //if ((ring_buffer = (buffer_l *)shmat(h_mem)) == NULL) {
  //Printf("Could not map the shared page to virtual address in "); Printf(argv[0]); Printf(", exiting..\n");
  //Exit();
  //}

  // Put some values in the shared memory, to be read by other processes
  //ring_buffer->currptr = 0;
  //ring_buffer->endptr = 0;

  // Create semaphore to not exit this process until all other processes 
  // have signalled that they are complete.  To do this, we will initialize
  // the semaphore to (-1) * (number of signals), where "number of signals"
  // should be equal to the number of processes we're spawning - 1.  Once 
  // each of the processes has signaled, the semaphore should be back to
  // zero and the final sem_wait below will return.
  //// Total processes = num_n2_created + num_n2_used
  ////                    + num_h2o_created + num_h2o_used/2
  ////                    + limiting_num
  if ((s_procs_completed = sem_create(-(((num_n2*2)+(num_h2o)+(num_h2o/2)+(num_limit))-1))) == SYNC_FAIL) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }

  // Molecule mailbox creation
  if((mb_n2 = mbox_create()) == MBOX_FAIL) {
    Printf("Bad mbox_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if((mb_h2o = mbox_create()) == MBOX_FAIL) {
    Printf("Bad mbox_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if((mb_h2 = mbox_create()) == MBOX_FAIL) {
    Printf("Bad mbox_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if((mb_n = mbox_create()) == MBOX_FAIL) {
    Printf("Bad mbox_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if((mb_o2 = mbox_create()) == MBOX_FAIL) {
    Printf("Bad mbox_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if((mb_no2 = mbox_create()) == MBOX_FAIL) {
    Printf("Bad mbox_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }


  // Open each mailbox after being created
  if(mbox_open(mb_n2) == MBOX_FAIL) {
    Printf("Bad mbox_open in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if(mbox_open(mb_h2o) == MBOX_FAIL) {
    Printf("Bad mbox_open in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if(mbox_open(mb_h2) == MBOX_FAIL) {
    Printf("Bad mbox_open in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if(mbox_open(mb_n) == MBOX_FAIL) {
    Printf("Bad mbox_open in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if(mbox_open(mb_o2) == MBOX_FAIL) {
    Printf("Bad mbox_open in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if(mbox_open(mb_no2) == MBOX_FAIL) {
    Printf("Bad mbox_open in "); Printf(argv[0]); Printf("\n");
    Exit();
  }


  // Setup the command-line arguments for the new process.  We're going to
  // pass the handles to the shared memory page and the semaphore as strings
  // on the command line, so we must first convert them from ints to strings.
  ditoa(s_procs_completed, s_procs_completed_str);
  ditoa(mb_n2, mb_n2_str);
  ditoa(mb_h2o, mb_h2o_str);
  ditoa(mb_h2, mb_h2_str);
  ditoa(mb_n, mb_n_str);
  ditoa(mb_o2, mb_o2_str);
  ditoa(mb_no2, mb_no2_str);

  //ditoa(num_n2, num_n2_str);
  //ditoa(num_h2o, num_h2o_str);
  //ditoa(num_limit, num_limit_str);

  // Now we can create the processes.  Note that you MUST end your call to
  // process_create with a NULL argument so that the operating system
  // knows how many arguments you are sending.
  /* Create a process for each N2 generation and consumption */
  for(num_molecules = 0; num_molecules < num_n2; num_molecules++) {
    // N2 Generator
    process_create(FILENAME_INJECTN2, pnice, pinfo, s_procs_completed_str, mb_n2_str);
    // N Reactor
    process_create(FILENAME_REACT1, pnice, pinfo, s_procs_completed_str, mb_n2_str, mb_n_str);
  }
  /* Create a process for each H2O generation and consumption */
  for(num_molecules = 0; num_molecules < num_h2o; num_molecules++) {
    // H2O Generator
    process_create(FILENAME_INJECTH2O, pnice, pinfo, s_procs_completed_str, mb_h2o_str);
  }
  for(num_molecules = 0; num_molecules < num_h2o/2; num_molecules++) {
    // H2O Reactor
    process_create(FILENAME_REACT2, pnice, pinfo, s_procs_completed_str, mb_h2o_str, mb_h2_str, mb_o2_str);
  }
  /* Create a process for each NO2 Reaction */
  for(num_molecules = 0; num_molecules < num_limit; num_molecules++) {
    // N2O Reactor
    process_create(FILENAME_REACT3, pnice, pinfo, s_procs_completed_str, mb_n_str, mb_o2_str, mb_no2_str);
  }
  Printf("Processes created\n");

  // And finally, wait until all spawned processes have finished.
  if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf("\n");
    Exit();
  }

  // Close mailboxes once all processes have finished
  if(mbox_close(mb_n2) == MBOX_FAIL) {
    Printf("Bad mbox_close in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if(mbox_close(mb_h2o) == MBOX_FAIL) {
    Printf("Bad mbox_close in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if(mbox_close(mb_n) == MBOX_FAIL) {
    Printf("Bad mbox_close in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if(mbox_close(mb_o2) == MBOX_FAIL) {
    Printf("Bad mbox_close in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if(mbox_close(mb_no2) == MBOX_FAIL) {
    Printf("Bad mbox_close in "); Printf(argv[0]); Printf("\n");
    Exit();
  }

  // Exit message
  Printf("All other processes completed, exiting main process.\n");
}
