#include "ostraps.h"
#include "dlxos.h"
#include "traps.h"
#include "queue.h"
#include "disk.h"
#include "dfs.h"
#include "synch.h"

static dfs_inode inodes[FDISK_NUM_INODES]; // all inodes
static dfs_superblock sb; // superblock
static uint32 fbv[DFS_FBV_MAX_NUM_WORDS]; // Free block vector

static uint32 negativeone = 0xFFFFFFFF;
static inline uint32 invert(uint32 n) { return n ^ negativeone; }

static uint32 fs_is_open = 0; // Flag to mark when file system has been opened

static lock_t fbv_lock;
static lock_t inode_lock;


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

  // Also allocate a lock for later file system operations
  fbv_lock = LockCreate();
  inode_lock = LockCreate();
  CreateFileLock();
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
  bzero(buffer_block.data, DISK_BLOCKSIZE);
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
  if(DiskReadBlock(1, &buffer_block) == DISK_FAIL) {
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
  for(i=0; i < DFS_FBV_MAX_NUM_WORDS / (DISK_BLOCKSIZE / 4); i++) {
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
  if(DiskWriteBlock(1, &buffer_block) == DISK_FAIL) {
    printf("DfsOpenFileSystem Failed to Write Super Block Back To Disk!\n");
    return DFS_FAIL;
  }
  sb.valid = 1; // Re-validate in memory

  // Mark file system as open
  fs_is_open = 1;

  return DFS_SUCCESS;
}


//-------------------------------------------------------------------
// DfsCloseFileSystem writes the current memory version of the
// filesystem metadata to the disk, and invalidates the memory's 
// version.
//-------------------------------------------------------------------

int DfsCloseFileSystem() {
  disk_block buffer_block;
  int i;
  bzero(buffer_block.data, DISK_BLOCKSIZE);
  //Basic steps:
  // Check that filesystem is not already open
  if(!fs_is_open || !sb.valid) {
    printf("DfsCloseFileSystem tried to close a file system that is not open or invalid!\n");
    return DFS_FAIL;
  }

  // Mark file system as closed
  fs_is_open = 0;

  // Read superblock from disk.  Note this is using the disk read rather 
  // than the DFS read function because the DFS read requires a valid 
  // filesystem in memory already, and the filesystem cannot be valid 
  // until we read the superblock. Also, we don't know the block size 
  // until we read the superblock, either.
  bcopy((char*)&sb, buffer_block.data, sizeof(dfs_superblock));
  if(DiskWriteBlock(1, &buffer_block) == DISK_FAIL) {
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
  for(i=0; i < DFS_FBV_MAX_NUM_WORDS / (DISK_BLOCKSIZE / 4); i++) {
    bcopy((char*)(fbv+i), buffer_block.data, sizeof(uint32)); // Copy free block vector to system resource
    // Write free block vector from disk
    if(DiskWriteBlock(2+FDISK_NUM_INODES+i, &buffer_block) == DISK_FAIL) {
      printf("DfsCloseFileSystem Failed to write to disk!\n");
      return DFS_FAIL;
    }
  }

  // Invalidate file system in memory
  DfsInvalidate();

  return DFS_SUCCESS;
}


//-----------------------------------------------------------------
// DfsAllocateBlock allocates a DFS block for use. Remember to use 
// locks where necessary.
//-----------------------------------------------------------------

uint32 DfsAllocateBlock() {
  int i, j;
  int block_offset = sb.dfs_datablock_start;
  // Check that file system has been validly loaded into memory
  if(!fs_is_open || !sb.valid) {
    printf("DfsAllocateBlock detected that a file system is not open or is invalid!\n");
    return DFS_FAIL;
  }

  // Lock free block vector for searching
  if(LockHandleAcquire(fbv_lock) != SYNC_SUCCESS) {
    printf("DfsAllocateBlock bad lock acquire!\n");
    return DFS_FAIL;
  }

  // Find the first free block using the free block vector (FBV), mark it in use
  for(i = 0; i < DFS_FBV_MAX_NUM_WORDS; i++) {
    // Loop over each bit of the current int (32 bits) in the array
    for(j = 0; j < 32; j++) {
      // Search for a free block (0 => Empty Block, 1 => Used Block)
      // (0 & 1) = 0 (i.e. unused block), (1 & 1) = 1 (i.e. used block)
      if(!(fbv[i] & (0x1 << j))) {
        // Mark the block as used
        fbv[i] |= (0x1 << j);
        // Release lock before returning
        if(LockHandleRelease(fbv_lock) != SYNC_SUCCESS) {
          printf("DfsAllocateBlock bad lock release!\n");
          return DFS_FAIL;
        }
        // return the block number
        return (32 * i + j) + block_offset;
      }
    }
  }

  // Release lock before returning
  if(LockHandleRelease(fbv_lock) != SYNC_SUCCESS) {
    printf("DfsAllocateBlock bad lock release!\n");
    return DFS_FAIL;
  }

  // Return failed if this part is reach
  return DFS_FAIL;
}


//-----------------------------------------------------------------
// DfsFreeBlock deallocates a DFS block.
//-----------------------------------------------------------------

int DfsFreeBlock(uint32 blocknum) {
  int block_offset = sb.dfs_datablock_start;
  uint32 fbv_index = (blocknum - block_offset) / 32;
  uint32 fbv_bit = (blocknum - block_offset) - (32 * fbv_index);

  // Verify file system is open
  if(!fs_is_open || !sb.valid) {
    printf("DfsFreeBlock detected that a file system is not open or invalid!\n");
    return DFS_FAIL;
  }

  // Lock free block vector
  if(LockHandleAcquire(fbv_lock) != SYNC_SUCCESS) {
    printf("DfsFreeBlock bad lock acquire!\n");
    return DFS_FAIL;
  }

  // Free block in fbv
  fbv[fbv_index] ^= (0x1 << fbv_bit);

  // Release lock before returning
  if(LockHandleRelease(fbv_lock) != SYNC_SUCCESS) {
    printf("DfsFreeBlock bad lock release!\n");
    return DFS_FAIL;
  }

  // Return Success
  return DFS_SUCCESS;
}


//-----------------------------------------------------------------
// DfsReadBlock reads an allocated DFS block from the disk
// (which could span multiple physical disk blocks).  The block
// must be allocated in order to read from it.  Returns DFS_FAIL
// on failure, and the number of bytes read on success.  
//-----------------------------------------------------------------

int DfsReadBlock(uint32 blocknum, dfs_block *b) {
  int block_offset = sb.dfs_datablock_start;
  uint32 fbv_index = (blocknum - block_offset) / 32;
  uint32 fbv_bit = (blocknum - block_offset) - (32 * fbv_index);
  uint32 physical_blocknum;
  disk_block temp;

  if(fbv[fbv_index] & (0x1 << (fbv_bit-1))){

    physical_blocknum = blocknum * (sb.dfs_blocksize / DISK_BLOCKSIZE);

    //if true read entire block 
    //put it in dfs_block *b
    if(DiskReadBlock(physical_blocknum, &temp) != sb.dfs_blocksize){
      bcopy(temp.data, b->data, DISK_BLOCKSIZE);

      DiskReadBlock(physical_blocknum+1, &temp);
      bcopy(temp.data, b->data+DISK_BLOCKSIZE, DISK_BLOCKSIZE);

    }else{
      bcopy(temp.data, b->data, DISK_BLOCKSIZE);
    }

    //return bytes read on success (block size??)
    return sb.dfs_blocksize;

  }

  return DFS_FAIL;
}



//-----------------------------------------------------------------
// DfsWriteBlock writes to an allocated DFS block on the disk
// (which could span multiple physical disk blocks).  The block
// must be allocated in order to write to it.  Returns DFS_FAIL
// on failure, and the number of bytes written on success.  
//-----------------------------------------------------------------

int DfsWriteBlock(uint32 blocknum, dfs_block *b){
  int block_offset = sb.dfs_datablock_start;
  uint32 fbv_index = (blocknum - block_offset) / 32;
  uint32 fbv_bit = (blocknum - block_offset) - (32 * fbv_index);
  uint32 physical_blocknum;
  disk_block temp;

  if(fbv[fbv_index] & (0x1 << fbv_bit)){

    physical_blocknum = blocknum * (sb.dfs_blocksize / DISK_BLOCKSIZE);

    //if true read entire block 
    //put it in dfs_block *b

    bcopy(b->data, temp.data, DISK_BLOCKSIZE);
    if(DiskWriteBlock(physical_blocknum, &temp) != sb.dfs_blocksize){

      bcopy(b->data+DISK_BLOCKSIZE, temp.data, DISK_BLOCKSIZE);
      DiskWriteBlock(physical_blocknum+1, &temp);

    }

    //return bytes read on success (block size??)
    return sb.dfs_blocksize;

  }

  return DFS_FAIL;
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
  uint32 i;
  // Loop through every inode
  for(i = 0; i < FDISK_NUM_INODES; i++) {
    // Check if it is in inuse inode
    if(inodes[i].inuse) {
      if(dstrncmp(inodes[i].filename, filename, dstrlen(filename))) {
        // If filename exists, return handle
        return i;
      }
    }
  }
  // Else, return DFS_FAIL
  return DFS_FAIL;
}


//-----------------------------------------------------------------
// DfsInodeOpen: search the list of all inuse inodes for the 
// specified filename. If the filename exists, return the handle 
// of the inode. If it does not, allocate a new inode for this 
// filename and return its handle. Return DFS_FAIL on failure. 
// Remember to use locks whenever you allocate a new inode.
//-----------------------------------------------------------------

uint32 DfsInodeOpen(char *filename) {
  uint32 handle;



  /* Check if file exists */
  if((handle = DfsInodeFilenameExists(filename)) != DFS_FAIL) {
    return handle;
  }

  /* Otherwise, allocate a new inode and return its handle */
  // Make sure lock is secure before allocating an inode
  if(LockHandleAcquire(inode_lock) != SYNC_SUCCESS) {
    printf("DfsFreeBlock bad lock acquire!\n");
    return DFS_FAIL;
  }

  // Allocate the new inode
  handle = AllocateInode();


  // Release lock
  if(LockHandleRelease(inode_lock) != SYNC_SUCCESS) {
    printf("DfsFreeBlock bad lock release!\n");
    return DFS_FAIL;
  }


  printf("starting max size (should be 0)%d\n", inodes[handle].max_size);
  // Check if allocate succeeded
  if(handle == DFS_FAIL) {
    printf("DfsInodeOpen no inode could be allocated!\n");
    return DFS_FAIL;
  }

  // Load filename into inode
  dstrcpy(inodes[handle].filename, filename);

  // Return file handle
  return handle;
}


//-----------------------------------------------------------------
// DfsInodeDelete de-allocates any data blocks used by this inode, 
// including the indirect addressing block if necessary, then mark 
// the inode as no longer in use. Use locks when modifying the 
// "inuse" flag in an inode.Return DFS_FAIL on failure, and 
// DFS_SUCCESS on success.
//-----------------------------------------------------------------

int DfsInodeDelete(uint32 handle) {
  int i = 0;
  uint32 indirect_table[sb.dfs_blocksize/4];
  dfs_block indirect_table_block;

  if(!inodes[handle].inuse) { printf("Invalid inode handle!\n"); return DFS_FAIL; }

  // Free block in indirect table
  if(DfsReadBlock(inodes[handle].vblock_index, &indirect_table_block) == DFS_FAIL) {
    printf("DfsInodeDelete could not read indirect table block!\n");
    return DFS_FAIL;
  }
  // Move data in block to a local array
  bcopy(indirect_table_block.data, (char*)indirect_table, sb.dfs_blocksize);
  // Free all blocks referenced in indirect table
  while(indirect_table[i] != 0) {
    if(DfsFreeBlock(indirect_table[i]) == DFS_FAIL) {
      printf("DfsInodeDelete could not free an address in indirect table!\n");
      return DFS_FAIL;
    }
    // Move to next address
    i++;
  }

  if(DfsFreeBlock(inodes[handle].vblock_index) == DFS_FAIL) {
    printf("DfsInodeDelete could not free  indirect table!\n");
    return DFS_FAIL;
  }


  // Clear indirect address block number
  inodes[handle].vblock_index = 0;

  // Free every data block in table
  for(i = 0; i < 10; i++) {
    if(DfsFreeBlock(inodes[handle].vblock_table[i]) == DFS_FAIL) {
      printf("DfsInode could not free an address data block table!\n");
      return DFS_FAIL;
    }
    // Zero table entry
    inodes[handle].vblock_table[i] = 0;
  }

  // Clear max size
  inodes[handle].max_size = 0;

  // Clear filename
  bzero(inodes[handle].filename, FILE_MAX_FILENAME_LENGTH);

  // Make sure lock is secure before freeing an inode
  if(LockHandleAcquire(inode_lock) != SYNC_SUCCESS) {
    printf("DfsFreeBlock bad lock acquire!\n");
    return DFS_FAIL;
  }

  // Mark inode as not inuse
  inodes[handle].inuse = 0;

  // Release lock
  if(LockHandleRelease(inode_lock) != SYNC_SUCCESS) {
    printf("DfsFreeBlock bad lock release!\n");
    return DFS_FAIL;
  }

  return DFS_SUCCESS;
}


//-----------------------------------------------------------------
// DfsInodeReadBytes reads num_bytes from the file represented by 
// the inode handle, starting at virtual byte start_byte, copying 
// the data to the address pointed to by mem. Return DFS_FAIL on 
// failure, and the number of bytes read on success.
//-----------------------------------------------------------------

int DfsInodeReadBytes(uint32 handle, void *mem, int start_byte, int num_bytes) {
  uint32 i;
  dfs_block temp_block;
  uint32 blocknum;
  int start = start_byte - (start_byte / sb.dfs_blocksize) * sb.dfs_blocksize;
  int bytes_read = 0;

  // Zero temp_block
  bzero(temp_block.data, DFS_BLOCKSIZE);

  if(!inodes[handle].inuse) { printf("Invalid inode handle!\n"); return DFS_FAIL; }

  for(i = start_byte / sb.dfs_blocksize; i <= (start_byte + num_bytes) / sb.dfs_blocksize; i++) {
    // Get a data block from table
    blocknum = DfsInodeTranslateVirtualToFilesys(handle, i);

    // Get a data block from table
    if(DfsReadBlock(blocknum, &temp_block) == DFS_FAIL) {
      printf("DfsInodeReadBytes failed to read a block: %d\n", i);
      return DFS_FAIL;
    }

    // Copy bytes to buffer
    if((sb.dfs_blocksize - start) < (num_bytes - bytes_read)) {
      // Read from start byte to end of block
      bcopy((temp_block.data + start), (mem + bytes_read), (sb.dfs_blocksize - start));
      // Update number of bytes read
      bytes_read += (sb.dfs_blocksize - start);
    } else {
      // Read from start byte to end of block
      bcopy((temp_block.data + start), (mem + bytes_read), (num_bytes - bytes_read));
      // Update number of bytes read
      bytes_read += (num_bytes - bytes_read);
    }

    // Set start to beginning of a block
    start = 0;
  }

  return num_bytes;
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
  uint32 i;
  dfs_block temp_block;
  int start = start_byte - (start_byte / sb.dfs_blocksize) * sb.dfs_blocksize;
  int bytes_written = 0;
  int blocknum;
  int block_bytes;

  // Zero temp block
  bzero(temp_block.data, DFS_BLOCKSIZE);

  if(!inodes[handle].inuse) { printf("Invalid inode handle!\n"); return DFS_FAIL; }

  // Allocate all blocks need in write that are not allocated
  for(i = 0; i <=( (start_byte + num_bytes) / sb.dfs_blocksize); i++) {
    // Block translation
    blocknum = DfsInodeTranslateVirtualToFilesys(handle, i);
    // If translation is 0, then allocate a block
    if(blocknum == 0) {
      if(DfsInodeAllocateVirtualBlock(handle, i) == DFS_FAIL) {
        printf("DfsInodeWriteBytes failed to allocate a new block!\n");
        return DFS_FAIL;
      }
    }
  }

  for(i = start_byte / sb.dfs_blocksize; i <= (start_byte + num_bytes) / sb.dfs_blocksize; i++) {
    // Print bytes written compared to number of bytes
    //printf("Block: %d, Bytes written: %d, num_bytes: %d\n", i, bytes_written, num_bytes);

    // Get a data block from table
    blocknum = DfsInodeTranslateVirtualToFilesys(handle, i);

    // Copy bytes to buffer
    if(start != 0) {
      // Decide how many bytes to write
      if((start_byte / sb.dfs_blocksize) == ((start_byte + num_bytes) / sb.dfs_blocksize)) {
        // Writing completely inside a single block
        block_bytes = num_bytes;
      } else {
        // Writing to until end of block
        block_bytes = sb.dfs_blocksize - start;
      }
      // Read block from disk, modify it, and write back
      // Read block from disk
      if(DfsReadBlock(blocknum, &temp_block) == DFS_FAIL) {
        printf("DfsInodeWriteBytes failed to read a block: blocknum\n");
        return DFS_FAIL;
      }
      // Modify the block
      bcopy((char*)(mem + bytes_written), (temp_block.data + start), block_bytes);
      // Write block back to disk
      if(DfsWriteBlock(blocknum, &temp_block) == DFS_FAIL) {
        printf("DfsInodeWriteBytes failed to write a block: blocknum\n");
        return DFS_FAIL;
      }
      // Increment bytes written
      //bytes_written += block_bytes;
    } else {
      // Write from beginning to end of block
      if((num_bytes - bytes_written) > sb.dfs_blocksize) {
        // Copy data from buffer
        bcopy((char*)(mem + bytes_written), temp_block.data, sb.dfs_blocksize);
        // Write entire block
        if(DfsWriteBlock(blocknum, &temp_block) == DFS_FAIL) {
          printf("DfsInodeWriteBytes failed to write a block: blocknum\n");
          return DFS_FAIL;
        }
        // Increment bytes written
        //bytes_written += block_bytes;
	block_bytes = sb.dfs_blocksize;
      } else {
        // Write until some point
        // Read block from disk, modify it, and write back
        // Read block from disk
        if(DfsReadBlock(blocknum, &temp_block) == DFS_FAIL) {
          printf("DfsInodeWriteBytes failed to read a block: blocknum\n");
          return DFS_FAIL;
        }
        // Modify the block
        bcopy((char*)(mem + bytes_written), temp_block.data, (num_bytes - bytes_written));
        // Write block back to disk
        if(DfsWriteBlock(blocknum, &temp_block) == DFS_FAIL) {
          printf("DfsInodeWriteBytes failed to write a block: blocknum\n");
          return DFS_FAIL;
        }
        // Increment bytes written
        //bytes_written += block_bytes;
	block_bytes = num_bytes - bytes_written;
      }
    }
    // Increment bytes written
    bytes_written += block_bytes;
    // Set start to beginning of a block
    start = 0;
  }
  // Update inode file size tracker
  if(inodes[handle].max_size < start_byte + num_bytes) {
    inodes[handle].max_size = start_byte + num_bytes;
  }

  return num_bytes;
}


//-----------------------------------------------------------------
// DfsInodeFilesize simply returns the size of an inode's file. 
// This is defined as the maximum virtual byte number that has 
// been written to the inode thus far. Return DFS_FAIL on failure.
//-----------------------------------------------------------------

uint32 DfsInodeFilesize(uint32 handle) {
  if(!inodes[handle].inuse) { printf("Invalid inode handle!\n"); return DFS_FAIL; }
  return inodes[handle].max_size;
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
  dfs_block indirect_block;
  uint32 indirect_table[sb.dfs_blocksize/4];
  uint32 return_blocknum;
  int i;
  bzero(indirect_block.data, DFS_BLOCKSIZE);

  // Decide if direct or indirect space
  if(virtual_blocknum < 10) {
    // Normal table allocation operation
    if((inodes[handle].vblock_table[virtual_blocknum] = DfsAllocateBlock()) == DFS_FAIL) {
      printf("DfsInodeAllocateVirtualBlock failed to allocate a new block!\n");
      return DFS_FAIL;
    }
    // Set return value
    return_blocknum = inodes[handle].vblock_table[virtual_blocknum];
  } else {
    // Indirect table allocation operation
    // Check if indirect block is allocated
    if(inodes[handle].vblock_index == 0) {
      // Allocate a block for indirect indexing
      if((inodes[handle].vblock_index = DfsAllocateBlock()) == DFS_FAIL) {
        printf("DfsInodeAllocateVirtualBlock failed to allocate a new block!\n");
        return DFS_FAIL;
      }
      // Zero the block for indirect addressing
      if(DfsWriteBlock(inodes[handle].vblock_index, &indirect_block) == DFS_FAIL) {
	printf("DfsInodeAllocateVirtualBlock failed to write a block: %d\n", inodes[handle].vblock_index);
	return DFS_FAIL;
      } 
    }

    // Read indirect indexing block from disk
    if(DfsReadBlock(inodes[handle].vblock_index, &indirect_block) == DFS_FAIL) {
      printf("DfsInodeAllocateVirtualBlock failed to read a block: %d\n", inodes[handle].vblock_index);
      return DFS_FAIL;
    }
    // Copy block into array
    bcopy(indirect_block.data, (char*)indirect_table, sb.dfs_blocksize);
    // Allocate a block and store in indirect table
    if((indirect_table[virtual_blocknum - 10] = DfsAllocateBlock()) == DFS_FAIL) {
      printf("DfsInodeAllocateVirtualBlock failed to allocate a new block!\n");
      return DFS_FAIL;
    }
    // Copy back into block structure
    bcopy((char*)indirect_table, indirect_block.data, sb.dfs_blocksize);
    // Write back to disk
    if(DfsWriteBlock(inodes[handle].vblock_index, &indirect_block) == DFS_FAIL) {
      printf("DfsInodeAllocateVirtualBlock failed to write a block: %d\n", inodes[handle].vblock_index);
      return DFS_FAIL;
    } 
    // Set return value
    return_blocknum = indirect_table[virtual_blocknum - 10];
  }

  return return_blocknum;
}



//-----------------------------------------------------------------
// DfsInodeTranslateVirtualToFilesys translates the 
// virtual_blocknum to the corresponding file system block using 
// the inode identified by handle. Return DFS_FAIL on failure.
//-----------------------------------------------------------------

uint32 DfsInodeTranslateVirtualToFilesys(uint32 handle, uint32 virtual_blocknum) {
  dfs_block indirect_block;
  uint32 indirect_table[sb.dfs_blocksize/4];

  // Zero indirect_block
  bzero(indirect_block.data, DFS_BLOCKSIZE);

  if(!inodes[handle].inuse) { printf("Invalid inode handle!\n"); return DFS_FAIL; }

  // Decide which action to take
  if(virtual_blocknum < 10) {
    return inodes[handle].vblock_table[virtual_blocknum];
  }
  // Check if indirect space has been allocated
  if(inodes[handle].vblock_index == 0) { return 0; }

  // Read indirect block
  DfsReadBlock(inodes[handle].vblock_index, &indirect_block);
  // Copy into array
  bcopy(indirect_block.data, (char*)indirect_table, sb.dfs_blocksize/4);
  // Return block number
  return indirect_table[virtual_blocknum-10];
}

/* Helper function for allocating a new inode */
uint32 AllocateInode() {
  uint32 i, j;
  // Find a free inode and return its handle
  for(i = 0; i < FDISK_NUM_INODES; i++) {
    // Check if inodes is inuse
    if(!inodes[i].inuse) {
      // Mark as now inuse
      inodes[i].inuse = 1;
      inodes[i].max_size =0;
      inodes[i].vblock_index = 0;
      // Zero direct translation table
      for(j = 0; j < 10; j++) {
        inodes[i].vblock_table[j] = 0;
      }
      // Return the handle
      return i;
    }
  }
  // Error if no free inode is found
  return DFS_FAIL;
}


void InodeTests(){
  uint32 handle;
  int i; 
  char test_memory[]="test data";
  uint32 fsblock; 

  char read_mem[10] = {0};


  printf("FBV %x\n", fbv[0]);
  /* Step 1: Open a new inode */
  printf("=1=> Opening a new inode named: test1\n");
  handle = DfsInodeOpen("test1");

  printf("FBV %x\n", fbv[0]);

  /* 
     printf("=2=> allocating new virtual block: \n");
     fsblock = DfsInodeAllocateVirtualBlock(handle, 0);
     fsblock = DfsInodeAllocateVirtualBlock(handle, 1);
     fsblock = DfsInodeAllocateVirtualBlock(handle, 2);
     fsblock = DfsInodeAllocateVirtualBlock(handle, 3);
     fsblock = DfsInodeAllocateVirtualBlock(handle, 4);
     fsblock = DfsInodeAllocateVirtualBlock(handle, 5);
     fsblock = DfsInodeAllocateVirtualBlock(handle, 6);
     fsblock = DfsInodeAllocateVirtualBlock(handle, 7);
     fsblock = DfsInodeAllocateVirtualBlock(handle, 8);
     fsblock = DfsInodeAllocateVirtualBlock(handle, 9);
     fsblock = DfsInodeAllocateVirtualBlock(handle, 10);
     fsblock = DfsInodeAllocateVirtualBlock(handle, 11);
     fsblock = DfsInodeAllocateVirtualBlock(handle, 12);

*/

  /* Step 3: Write to new inode */
  printf("=3=> Writing to new inode data: %s\n", test_memory);
  DfsInodeWriteBytes(handle, (void*)test_memory, 0, sizeof(test_memory));

  printf("=3.1=> filesize of current data: %d\n:", DfsInodeFilesize(handle));


  /* Step 4: Writing to same block, different offset */
  dstrcpy(test_memory, "dead beef");
  printf("=4=> Writing to new inode data: %s\n", test_memory);
  DfsInodeWriteBytes(handle, (void*)test_memory, 40, sizeof(test_memory));

  /* Step 5: Writing to same block, different offset */
  dstrcpy(test_memory, "potatoes");
  printf("=5=> Writing to new inode data: %s\n", test_memory);
  DfsInodeWriteBytes(handle, (void*)test_memory, 254, sizeof(test_memory));

  /* Step 6: Writing across blocks */
  dstrcpy(test_memory, "carrots");
  printf("=6=> Writing to new inode data: %s\n", test_memory);
  DfsInodeWriteBytes(handle, (void*)test_memory, 510, sizeof(test_memory));

  /* Step 7: Writing to indirect space!!! */
  dstrcpy(test_memory, "broth");
  printf("=7=> Writing indirect space!!: %s\n", test_memory);
  DfsInodeWriteBytes(handle, (void*)test_memory, 5632, sizeof(test_memory));


  /* Step 8: Writing to indirect space!!! */
  printf("=8=> reading memory!!: \n");
  DfsInodeReadBytes(handle, (void*)read_mem, 0, 10);
  printf("=8=> Read!!: %s\n", read_mem);


  /* Step 8: Writing to indirect space!!! */
  printf("=9=> reading memory!!: \n");
  DfsInodeReadBytes(handle, (void*)read_mem, 510, 10);
  printf("=9=> Read!!: %s\n", read_mem);


  printf("FBV %x\n", fbv[0]);

  DfsInodeDelete(handle);  

  printf("=10=> filesize of current data: %d\n:", DfsInodeFilesize(handle));


  printf("FBV %x\n", fbv[0]);

  printf("=FINAL=> End of Inode test\n");

}


/* Helper function for testing various dfs functions */
void DfsTests() {
  uint32 blocknum;
  dfs_block test_block;
  dfs_block read_block;
  char data[] = "CAFE69DEADBEEF1337";
  int i;

  // Allocate a file system block to write to
  blocknum = DfsAllocateBlock();
  printf("=1=> Allocated block number: %d\n", blocknum);

  // Write data
  dstrcpy(test_block.data, data);
  //  test_block.data = 'CAFE69DEADBEEF1337';
  printf("=2=> Wrote data in a block: %s\n", test_block.data);

  // Write data to block
  if(DfsWriteBlock(blocknum, &test_block) == DFS_FAIL) {
    printf("MuddleFileSystem failed to write a block to disk!\n");
    return;
  }
  printf("=3=> Write a block to disk\n");

  // Read the block from disk
  if(DfsReadBlock(blocknum, &read_block) == DFS_FAIL) {
    printf("MuddleFileSystem failed to read a block from disk!\n");
    return;
  }
  printf("=4=> Read a block from disk\n");

  // Print read block
  printf("=5=> Displaying data from disk block: %s\n", read_block.data);

  // Print free block vector
  for(i = 0; i < DFS_FBV_MAX_NUM_WORDS; i++) {
    printf("word %d: %X // ", i, fbv[i]);
    if(!(i % 5)) { printf("\n"); }
  }
  printf("\n");

  // Free allocated block
  DfsFreeBlock(blocknum);
  printf("=6=> Freed block number: %d\n", blocknum);

}
