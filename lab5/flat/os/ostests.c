#include "ostraps.h"
#include "dlxos.h"
#include "traps.h"
#include "disk.h"
#include "dfs.h"

void RunOSTests() {
  int i;

  // Open the file system into system memory
  DfsModuleInit();

  // Modify super block value
  sb.dfs_blocksize = 0xDEAD;
  sb.dfs_blocknum = 0xBEEF;

  // Modify inodes 
  for(i = 0; i < FDISK_NUM_INODES; i++) {
    inodes[i].max_size = 0xCAFE0000 + i;
  }

  // Close file system and write back to disk
  DfsCloseFileSystem();
}

