#include "usertraps.h"
#include "misc.h"

#include "fdisk.h"

dfs_superblock sb;
dfs_inode inodes[DFS_INODE_MAX_NUM];
uint32 fbv[DFS_FBV_MAX_NUM_WORDS];

int diskblocksize = 0; // These are global in order to speed things up
int disksize = 0;      // (i.e. fewer traps to OS to get the same number)

int FdiskWriteBlock(uint32 blocknum, dfs_block *b); //You can use your own function. This function 
//calls disk_write_block() to write physical blocks to disk

void main (int argc, char *argv[])
{
  // STUDENT: put your code here. Follow the guidelines below. They are just the main steps. 
  // You need to think of the finer details. You can use bzero() to zero out bytes in memory

  //Initializations and argc check
  if (argc != 1) {
    Printf("Usage: "); Printf(argv[0]); Printf("Please edit fdisk.h to change the #defines.\n");
    Exit();
  }

  // Need to invalidate filesystem before writing to it to make sure that the OS
  // doesn't wipe out what we do here with the old version in memory
  // You can use dfs_invalidate(); but it will be implemented in Problem 2. You can just do 
  sb.valid = 0

  //add checking to be sure fs size is less than max size, same with disk
  sb.dfs_disksize = MY_DFS_FILESYSTEM_SIZE; 
  sb.dfs_blocksize = MY_DFS_BLOCKSIZE; 
  sb.dfs_blocknum = MY_DFS_NUMBLOCKS; 

  sb.dfs_inode_start = FDISK_INODE_BLOCK_START;
  sb.dfs_num_inodes = FDISK_NUM_INODES;
  sb.dfs_fbv_start = FDISK_FBV_BLOCK_START;

  // Make sure the disk exists before doing anything else
  if(DiskCreate() == DISK_FAIL) {
    Printf("Failed to create disk!\n");
    Exit();
  }

  // Write all inodes as not in use and empty (all zeros)
  for(i = 0; i < DFS_INODE_MAX_NUM; i++) {
    inodes[i].inuse = 0; // Mark as not in use
    inodex[i].max_size = 0; // Clear max size
    inodes[i].vblock_index = 0; // Clear index value
    for(j = 0; j < 10; j++) {
      inodes[i].vblock_table[j] = 0; // Zero table entries
    }
  }
  // Next, setup free block vector (fbv) and write free block vector to the disk
  // Finally, setup superblock as valid filesystem and write superblock and boot record to disk: 
  // boot record is all zeros in the first physical block, and superblock structure goes into the second physical block
  Printf("fdisk (%d): Formatted DFS disk for %d bytes.\n", getpid(), disksize);
}

int FdiskWriteBlock(uint32 blocknum, dfs_block *b) {
  // STUDENT: put your code here
}
