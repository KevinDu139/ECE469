#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "dfs.h"
#include "files.h"
#include "synch.h"

// You have already been told about the most likely places where you should use locks. You may use 
// additional locks if it is really necessary.

// STUDENT: put your file-level functions here
int FileOpen(char *filename, char *mode){

  //file modes - r, w, rw
  //max write is 4096 bytes

  //use locks when opening new file descriptor

  return 0;
}

int FileClose(int handle){
  return 0;
}

int FileRead(int handle, void *mem, int num_bytes){
  return 0;
}

int FileWrite(int handle, void *mem, int num_bytes){
  return 0;
}

int FileSeek(int handle, int num_bytes, int from_where){
  return 0;
}

int FileDelete(char *filename){
  return 0;
}

