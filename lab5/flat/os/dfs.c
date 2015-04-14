#include "ostraps.h"
#include "dlxos.h"
#include "traps.h"
#include "queue.h"
#include "disk.h"
#include "dfs.h"
#include "synch.h"

dfs_inode inodes[FDISK_NUM_INODES]; // all inodes
dfs_superblock sb; // superblock
static uint32 fbv[DFS_FBV_MAX_NUM_WORDS]; // Free block vector

static uint32 negativeone = 0xFFFFFFFF;
static inline uint32 invert(uint32 n) { return n ^ negativeone; }

static uint32 fs_is_open = 0; // Flag to mark when file system has been opened

// You have already been told about the most likely places where you should use locks. You may use 
// additional locks if it is really necessary.

// STUDENT: put your file system level functions below.
// Some skeletons are provided. You can implement additional functions.

///////////////////////////////////////////////////////////////////
// Non-inode functions first
///////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------
// DfsModuleInit is called at boot time to initialize things and
// open the file system for use.
//-----------------------------------------------------------------

void DfsModuleInit() {
// You essentially set the file system as invalid and then open 
// using DfsOpenFileSystem().
  DfsInvalidate();
  DfsOpenFileSystem();
}

//-----------------------------------------------------------------
// DfsInavlidate marks the current version of the filesystem in
// memory as invalid.  This is really only useful when formatting
// the disk, to prevent the current memory version from overwriting
// what you already have on the disk when the OS exits.
//-----------------------------------------------------------------

void DfsInvalidate() {
// This is just a one-line function which sets the valid bit of the 
// superblock to 0.
  sb.valid = 0;
}

//-------------------------------------------------------------------
// DfsOpenFileSystem loads the file system metadata from the disk
// into memory.  Returns DFS_SUCCESS on success, and DFS_FAIL on 
// failure.
//-------------------------------------------------------------------

int DfsOpenFileSystem() {
  disk_block buffer_block;
  uint32 virtual_block_size;
  int i;
//Basic steps:
// Check that filesystem is not already open
  if(fs_is_open) {
    printf("DfsOpenFileSystem tried to open file system that already is open!\n");
    return DFS_FAIL;
  }

// Read superblock from disk.  Note this is using the disk read rather 
// than the DFS read function because the DFS read requires a valid 
// filesystem in memory already, and the filesystem cannot be valid 
// until we read the superblock. Also, we don't know the block size 
// until we read the superblock, either.
  if(DiskReadBlock(1, buffer_block.data) == DISK_FAIL) {
    printf("DfsOpenFileSystem Failed to read super block from disk!\n");
    return DFS_FAIL;
  }

// Copy the data from the block we just read into the superblock in memory
  bcopy(buffer_block.data, (char*)&sb, sizeof(dfs_superblock));
  virtual_block_size = sb.dfs_blocksize;

// All other blocks are sized by virtual block size:
// Read inodes
  for(i=0; i < FDISK_NUM_INODES; i+=2) {
    // Read inodes from disk
    if(DiskReadBlock(2+(i/2), &buffer_block) == DISK_FAIL) {
      printf("DfsOpenFileSystem Failed to read from disk!\n");
      return DFS_FAIL;
    }
    bcopy(buffer_block.data, (char*)(inodes+i), 2*sizeof(dfs_inode)); // Copy inodes to system pool
  }

// Read free block vector
  for(i=0; i < DFS_FBV_MAX_NUM_WORDS; i++) {
    // Read free block vector from disk
    if(DiskReadBlock(2+FDISK_NUM_INODES+i, &buffer_block) == DISK_FAIL) {
      printf("DfsOpenFileSystem Failed to read from disk!\n");
      return DFS_FAIL;
    }
    bcopy(buffer_block.data, (char*)(fbv+i), sizeof(uint32)); // Copy free block vector to system resource
  }

// Change superblock to be invalid, write back to disk, then change 
// it back to be valid in memory
  DfsInvalidate(); // Invalidate memory to write back to disk
  bcopy((char*)&sb, buffer_block.data, sizeof(dfs_superblock));
  if(DiskWriteBlock(1, buffer_block.data) == DISK_FAIL) {
    printf("DfsOpenFileSystem Failed to Write Super Block Back To Disk!\n");
    return DFS_FAIL;
  }
  sb.valid = 1; // Re-validate in memory

  return DFS_SUCCESS;
}


//-------------------------------------------------------------------
// DfsCloseFileSystem writes the current memory version of the
// filesystem metadata to the disk, and invalidates the memory's 
// version.
//-------------------------------------------------------------------

int DfsCloseFileSystem() {
  disk_block buffer_block;
  uint32 virtual_block_size;
  int i;
//Basic steps:
// Check that filesystem is not already open
  if(fs_is_open) {
    printf("DfsCloseFileSystem tried to open file system that already is open!\n");
    return DFS_FAIL;
  }

// Read superblock from disk.  Note this is using the disk read rather 
// than the DFS read function because the DFS read requires a valid 
// filesystem in memory already, and the filesystem cannot be valid 
// until we read the superblock. Also, we don't know the block size 
// until we read the superblock, either.
    bcopy((char*)&sb, buffer_block.data, sizeof(dfs_superblock));
  if(DiskWriteBlock(1, buffer_block.data) == DISK_FAIL) {
    printf("DfsCloseFileSystem Failed to read super block from disk!\n");
    return DFS_FAIL;
  }

// All other blocks are sized by virtual block size:
// Write inodes
  for(i=0; i < FDISK_NUM_INODES; i+=2) {
    bcopy((char*)(inodes+i), buffer_block.data, 2*sizeof(dfs_inode)); // Copy inodes to system pool
    // Write inodes to disk
    if(DiskWriteBlock(2+(i/2), &buffer_block) == DISK_FAIL) {
      printf("DfsCloseFileSystem Failed to write to disk!\n");
      return DFS_FAIL;
    }

  }

// Write free block vector
  for(i=0; i < DFS_FBV_MAX_NUM_WORDS; i++) {
    bcopy((char*)(fbv+i), buffer_block.data, sizeof(uint32)); // Copy free block vector to system resource
    // Write free block vector from disk
    if(DiskWriteBlock(2+FDISK_NUM_INODES+i, &buffer_block) == DISK_FAIL) {
      printf("DfsCloseFileSystem Failed to write to disk!\n");
      return DFS_FAIL;
    }
  }

  // Invalidate file system in memory
  DfsInvalidate();

  // Mark file system as closed
  fs_is_open = 0;

  return DFS_SUCCESS;
}


//-----------------------------------------------------------------
// DfsAllocateBlock allocates a DFS block for use. Remember to use 
// locks where necessary.
//-----------------------------------------------------------------

uint32 DfsAllocateBlock() {
// Check that file system has been validly loaded into memory
// Find the first free block using the free block vector (FBV), mark it in use
// Return handle to block

}


//-----------------------------------------------------------------
// DfsFreeBlock deallocates a DFS block.
//-----------------------------------------------------------------

int DfsFreeBlock(uint32 blocknum) {

}


//-----------------------------------------------------------------
// DfsReadBlock reads an allocated DFS block from the disk
// (which could span multiple physical disk blocks).  The block
// must be allocated in order to read from it.  Returns DFS_FAIL
// on failure, and the number of bytes read on success.  
//-----------------------------------------------------------------

int DfsReadBlock(uint32 blocknum, dfs_block *b) {


}


//-----------------------------------------------------------------
// DfsWriteBlock writes to an allocated DFS block on the disk
// (which could span multiple physical disk blocks).  The block
// must be allocated in order to write to it.  Returns DFS_FAIL
// on failure, and the number of bytes written on success.  
//-----------------------------------------------------------------

int DfsWriteBlock(uint32 blocknum, dfs_block *b){

}


////////////////////////////////////////////////////////////////////////////////
// Inode-based functions
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------
// DfsInodeFilenameExists looks through all the inuse inodes for 
// the given filename. If the filename is found, return the handle 
// of the inode. If it is not found, return DFS_FAIL.
//-----------------------------------------------------------------

uint32 DfsInodeFilenameExists(char *filename) {

}


//-----------------------------------------------------------------
// DfsInodeOpen: search the list of all inuse inodes for the 
// specified filename. If the filename exists, return the handle 
// of the inode. If it does not, allocate a new inode for this 
// filename and return its handle. Return DFS_FAIL on failure. 
// Remember to use locks whenever you allocate a new inode.
//-----------------------------------------------------------------

uint32 DfsInodeOpen(char *filename) {

}


//-----------------------------------------------------------------
// DfsInodeDelete de-allocates any data blocks used by this inode, 
// including the indirect addressing block if necessary, then mark 
// the inode as no longer in use. Use locks when modifying the 
// "inuse" flag in an inode.Return DFS_FAIL on failure, and 
// DFS_SUCCESS on success.
//-----------------------------------------------------------------

int DfsInodeDelete(uint32 handle) {

}


//-----------------------------------------------------------------
// DfsInodeReadBytes reads num_bytes from the file represented by 
// the inode handle, starting at virtual byte start_byte, copying 
// the data to the address pointed to by mem. Return DFS_FAIL on 
// failure, and the number of bytes read on success.
//-----------------------------------------------------------------

int DfsInodeReadBytes(uint32 handle, void *mem, int start_byte, int num_bytes) {

}


//-----------------------------------------------------------------
// DfsInodeWriteBytes writes num_bytes from the memory pointed to 
// by mem to the file represented by the inode handle, starting at 
// virtual byte start_byte. Note that if you are only writing part 
// of a given file system block, you'll need to read that block 
// from the disk first. Return DFS_FAIL on failure and the number 
// of bytes written on success.
//-----------------------------------------------------------------

int DfsInodeWriteBytes(uint32 handle, void *mem, int start_byte, int num_bytes) {


}


//-----------------------------------------------------------------
// DfsInodeFilesize simply returns the size of an inode's file. 
// This is defined as the maximum virtual byte number that has 
// been written to the inode thus far. Return DFS_FAIL on failure.
//-----------------------------------------------------------------

uint32 DfsInodeFilesize(uint32 handle) {

}


//-----------------------------------------------------------------
// DfsInodeAllocateVirtualBlock allocates a new filesystem block 
// for the given inode, storing its blocknumber at index 
// virtual_blocknumber in the translation table. If the 
// virtual_blocknumber resides in the indirect address space, and 
// there is not an allocated indirect addressing table, allocate it. 
// Return DFS_FAIL on failure, and the newly allocated file system 
// block number on success.
//-----------------------------------------------------------------

uint32 DfsInodeAllocateVirtualBlock(uint32 handle, uint32 virtual_blocknum) {


}



//-----------------------------------------------------------------
// DfsInodeTranslateVirtualToFilesys translates the 
// virtual_blocknum to the corresponding file system block using 
// the inode identified by handle. Return DFS_FAIL on failure.
//-----------------------------------------------------------------

uint32 DfsInodeTranslateVirtualToFilesys(uint32 handle, uint32 virtual_blocknum) {

}
