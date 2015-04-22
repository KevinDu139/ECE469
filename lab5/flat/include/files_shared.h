#ifndef __FILES_SHARED__
#define __FILES_SHARED__

#define FILE_SEEK_SET 1
#define FILE_SEEK_END 2
#define FILE_SEEK_CUR 3

#define FILE_MAX_FILENAME_LENGTH 44

#define FILE_MAX_READWRITE_BYTES 4096

//-----------------------------------------------
// A file descriptor structure that stores
// relevant information about an open file such
// as the filename, the inode handle, the current
// position in the file, an end-of-file flag, and
// the mode
//-----------------------------------------------
typedef struct file_descriptor {
  // STUDENT: put file descriptor info here
  char inuse;
  char filename[FILE_MAX_FILENAME_LENGTH];
  uint32 inode_handle;
  uint32 current_byte_position;
  char eof_flag;
  char mode_read;
  char mode_write;
  char permissions;
} file_descriptor;

#define FILE_FAIL -1
#define FILE_EOF -1
#define FILE_SUCCESS 1

#endif
