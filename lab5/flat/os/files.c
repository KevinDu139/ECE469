#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "dfs.h"
#include "files.h"
#include "synch.h"

/* File descriptor pool */
static file_descriptor descriptors[FILE_MAX_OPEN_FILES];

/* File Descriptor pool lock */
static lock_t descriptor_lock;

// You have already been told about the most likely places where you should use locks. You may use 
// additional locks if it is really necessary.

// STUDENT: put your file-level functions here
//------------------------------------------------------------
// Open the given filename with one of three possible modes:
// "r", "w", or "rw". Return FILE_FAIL on failure 
// (e.g., when a process tries to open a file that is 
// already open for another process), and the handle of a 
// file descriptor on success. Remember to use locks whenever 
// you allocate a new file descriptor. (NEEDS A LOCK)
//------------------------------------------------------------
int FileOpen(char *filename, char *mode){
  int i;
  //file modes - r, w, rw
  //max write is 4096 bytes

  // Lock before accessing file descriptor pool
  if(LockHandleAcquire(descriptor_lock) != SYNC_SUCCESS) {
    printf("FileOpen bad lock acquire!\n");
    return FILE_FAIL;
  }

  // Check if the file is already open
  if(IsFileOpen(filename)) {
    printf("FileOpen found file (%s) is already open! Failed to open file!\n", filename);
    return FILE_FAIL;
  }

  // Find an available file descriptor
  for(i = 0; i < FILE_MAX_OPEN_FILES; i++) {
    // Found an available file descriptor
    if(descriptors[i].inuse == 0) {
      // Mark as inuse
      descriptors[i].inuse = 1;

      // Release lock from descriptor pool
      if(LockHandleRelease(descriptor_lock) != SYNC_SUCCESS) {
	printf("FileOpen bad lock release!\n");
	return FILE_FAIL;
      }

      // Initialize descriptor properties
      dstrcpy(filename, descriptors[i].filename); // Sets filename
      // Sets inode handle
      if((descriptors[i].inode_handle = DfsInodeOpen(filename)) == DFS_FAIL) {
	printf("FileOpen failed to find if a file name exists: %s\n", filename);
	return FILE_FAIL;
      }
      // Zeros current byte position
      descriptors[i].current_byte_position = 0;
      // Sets to false
      descriptors[i].eof_flag = 0;
      // Initialze both modes to zero
      descriptors[i].mode_read = 0;
      descriptors[i].mode_write = 0;
      // Set file modes to arguments
      if((int)dstrstr(mode, "r") != 0) {
	// Set mode read to true
	descriptors[i].mode_read = 1;
      }
      if((int)dstrstr(mode, "w") != 0) {
	// Set mode write to true
	descriptors[i].mode_write = 1;
      }
      // Zero permissions
      descriptors[i].permissions = 0;
      // Set file descriptor return handle
      return i;
    }
  }

  // Release lock from descriptor pool
  if(LockHandleRelease(descriptor_lock) != SYNC_SUCCESS) {
    printf("FileOpen bad lock release!\n");
    return FILE_FAIL;
  }

  
      
  return FILE_FAIL;
}

//------------------------------------------------------------
// Close the given file descriptor handle. Return FILE_FAIL 
// on failure, and FILE_SUCCESS on success.
//------------------------------------------------------------
int FileClose(int handle){
  descriptors[handle].inuse = 0;
  return FILE_SUCCESS;
}

//------------------------------------------------------------
// Read num_bytes from the open file descriptor identified by 
// handle. Return FILE_FAIL on failure or upon reaching end 
// of file, and the number of bytes read on success. If end
// of file is reached, the end-of-file flag in the file
// descriptor should be set.
//------------------------------------------------------------
int FileRead(int handle, void *mem, int num_bytes){
  int filesize;

  // Check if read mode is set
  if(!descriptors[handle].mode_read) {
    printf("FileRead attempt to read a file not set for reading!\n");
    return FILE_FAIL;
  }

  // Check if EOF is already occurred
  if(descriptors[handle].eof_flag) {
    printf("End of File previously reached! No bytes read!\n");
    return FILE_FAIL;
  }

  // Check num_bytes is within 4096 byte limit
  if(num_bytes > 4096) {
    printf("FileRead cannot read more than 4096 bytes at once, given %d bytes\n", num_bytes);
    return FILE_FAIL;
  }

  // Get filesize of file
  filesize = DfsInodeFilesize(descriptors[handle].inode_handle);

  // Check remaining bytes to be read
  if(descriptors[handle].current_byte_position + num_bytes > filesize) {
    printf("Reading to end of file\n");
    num_bytes = filesize - descriptors[handle].current_byte_position;
    descriptors[handle].eof_flag = 1;
  }

  // Read bytes from inode into mem buffer
  if(DfsInodeReadBytes(descriptors[handle].inode_handle,
		       mem,
		       descriptors[handle].current_byte_position,
		       num_bytes) == DFS_FAIL) {
    printf("FileRead failed to read from inode data!\n");
    return FILE_FAIL;
  }

  // Update current byte position
  descriptors[handle].current_byte_position += num_bytes;

  // Return eof or num_bytes
  if(descriptors[handle].eof_flag) { return FILE_EOF; }
  return num_bytes;
}

//------------------------------------------------------------
// Write num_bytes to the open file descriptor identified by
// handle. Return FILE_FAIL on failure, and the number of 
// bytes written on success.
//------------------------------------------------------------
int FileWrite(int handle, void *mem, int num_bytes){
  int filesize;

  // Check if write mode is set
  if(!descriptors[handle].mode_write) {
    printf("FileWrite attempt to write a file not set for reading!\n");
    return FILE_FAIL;
  }

  // Check num_bytes is within 4096 byte limit
  if(num_bytes > 4096) {
    printf("FileWrite cannot write more than 4096 bytes at once, given %d bytes\n", num_bytes);
    return FILE_FAIL;
  }

  // Get filesize of file
  filesize = DfsInodeFilesize(descriptors[handle].inode_handle);

  // Write data
  if(DfsInodeWriteBytes(descriptors[handle].inode_handle,
			mem,
			descriptors[handle].current_byte_position,
			num_bytes) == DFS_FAIL) {
    printf("FileWrite failed to write to inode data!\n");
    return FILE_FAIL;
  }

  // Set EOF if written to end or past end of file
  if(descriptors[handle].current_byte_position + num_bytes >= filesize) {
    descriptors[handle].eof_flag = 1;
  }

  // Update current byte position
  descriptors[handle].current_byte_position += num_bytes;

  return num_bytes;
}

//------------------------------------------------------------
// Seek num_bytes within the file descriptor identified by
// handle, from the location specified by from_where. There
// are three possible values for from_where: FILE_SEEK_CUR
// (seek relative to the current position), FILE_SEEK_SET
// (seek relative to the beginning of the file), and
// FILE_SEEK_END (seek relative to the end of the file). Any 
// seek operation will clear the eof flag.
//------------------------------------------------------------
int FileSeek(int handle, int num_bytes, int from_where){
  int filesize;
  
  // Check that seek option is valid
  if(from_where != FILE_SEEK_SET && from_where != FILE_SEEK_END && from_where != FILE_SEEK_CUR) {
    printf("FileSeek invalid seek option!\n");
    return FILE_FAIL;
  }

  // Unset end of file flag
  descriptors[handle].eof_flag = 0;

  // Get file descriptor inode file size
  filesize = DfsInodeFilesize(descriptors[handle].inode_handle);

  // File seek cur option
  if(from_where == FILE_SEEK_CUR) {
    // Check upper and lower limits
    if(num_bytes + descriptors[handle].current_byte_position > filesize ||
       num_bytes + descriptors[handle].current_byte_position < 0) {
      printf("FileSeek out of file bounds!\n");
      return FILE_FAIL;
    }

    // Shift current byte position
    descriptors[handle].current_byte_position += num_bytes;

  } else if(from_where == FILE_SEEK_SET) { // File seek set option
    // Check upper and lower limits
    if(num_bytes > filesize || num_bytes < 0) {
      printf("FileSeek out of file bounds!\n");
      return FILE_FAIL;
    }

    // Shift current byte position relative to position 0
    descriptors[handle].current_byte_position = num_bytes;

  } else if(from_where == FILE_SEEK_END) { // File seek end option
    // Check upper and lower limits
    if(num_bytes + filesize < 0 || num_bytes > 0) {
      printf("FileSeek out of file bounds!\n");
      return FILE_FAIL;
    }
    
    // Shift current byte position relative to end of file
    descriptors[handle].current_byte_position = num_bytes + filesize;
  }

  // Check for end of file
  if(descriptors[handle].current_byte_position == filesize) {
    descriptors[handle].eof_flag = 1;
  }

  return FILE_SUCCESS;
}

//------------------------------------------------------------
// Delete the file specified by filename. Return FILE_FAIL on 
// failure, and FILE_SUCCESS on success.
//------------------------------------------------------------
int FileDelete(char *filename){
  uint32 inode_handle = DfsInodeFilenameExists(filename);

  // Check if filename exists failed
  if(inode_handle == DFS_FAIL) {
    printf("FileDelete attempted to delete a file that does not exist!\n");
    return FILE_FAIL;
  }
  
  // Attempt to delete file inode
  if(DfsInodeDelete(inode_handle) == DFS_FAIL) {
    printf("FileDelete could not delete file inode!\n");
    return FILE_FAIL;
  }
  return FILE_SUCCESS;
}


//------------------------------------------------------------
// Helper Functions
//------------------------------------------------------------

/* create a lock for file descriptor pool */
void CreateFileLock() {
  descriptor_lock = LockCreate();
}

/* Check that a descriptor is not currently being used for a filename */
int IsFileOpen(char* filename) {
  int i;
  // Loop through file descriptors
  for(i = 0; i < FILE_MAX_OPEN_FILES; i++) {
    // Only check inuse file descriptors
    if(descriptors[i].inuse) {
      // Check if file names match
      if(dstrncmp(descriptors[i].filename, filename, FILE_MAX_FILENAME_LENGTH)) {
	// return 1 (true) if compare is a match
	return 1;
      }
    }
  }
  // return 0 (false) if loop completes
  return 0;
}
