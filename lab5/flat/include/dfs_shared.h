#ifndef __DFS_SHARED__
#define __DFS_SHARED__

typedef struct dfs_superblock {
  // STUDENT: put superblock internals here
  int valid; // If the super block is valid
  int dfs_disksize; // Total disk size
  int dfs_blocksize; // Virtual block size
  int dfs_blocknum;  // Total number of file system blocks
  int dfs_inode_start; // Virtual block inodes start at
  int dfs_num_inodes; // Number of available inodes in pool
  int dfs_fbv_start; // Starting virtual block number of free block vector
  int dfs_datablock_start; // Starting virtual block number of data blocks

} dfs_superblock;

#define DFS_BLOCKSIZE 512  // Must be an integer multiple of the disk blocksize

typedef struct dfs_block {
  char data[DFS_BLOCKSIZE];
} dfs_block;

typedef struct dfs_inode {
  // STUDENT: put inode structure internals here
  // IMPORTANT: sizeof(dfs_inode) MUST return 96 in order to fit in enough
  // inodes in the filesystem (and to make your life easier).  To do this, 
  // adjust the maximumm length of the filename until the size of the overall inode 
  // is 96 bytes.

  char inuse;
  uint32 max_size;
  char filename[FILE_MAX_FILENAME_LENGTH];
  uint32 vblock_table[10];
  uint32 vblock_index;

} dfs_inode;

#define DFS_MAX_FILESYSTEM_SIZE 0x1000000  // 16MB

#define DFS_FAIL -1
#define DFS_SUCCESS 1

/** STUDENT DEFINES **/
#define FDISK_NUM_INODES      192 
#define DISK_BLOCKSIZE  256
#define DFS_FBV_MAX_NUM_WORDS  DFS_MAX_FILESYSTEM_SIZE/ DFS_BLOCKSIZE/32

#define FDISK_FBV_BLOCK_START 1+ FDISK_NUM_INODES /(2*(DFS_BLOCKSIZE/DISK_BLOCKSIZE)) //STUDENT: define this
#endif
