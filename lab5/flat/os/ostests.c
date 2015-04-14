#include "ostraps.h"
#include "dlxos.h"
#include "traps.h"
#include "disk.h"
#include "dfs.h"

void RunOSTests() {
  int i;

  // Open the file system into system memory
  DfsModuleInit();

  // Modify some values before writing back
  MuddleFileSystem();

  // Close file system and write back to disk
  DfsCloseFileSystem();
}

