#include "usertraps.h"

#define FILE_SEEK_SET 1
#define FILE_SEEK_END 2
#define FILE_SEEK_CUR 3


char big_text[] = "A file system is simply a means of organizing a large set of bytes using a small amount of overhead, both in terms of size and access speed. There is no restriction as to what type of hardware a file system runs on: it could be Read-Only memory like a DVD-ROM, it could be a traditional magnetic or solid-state hard drive, it could be a RAM-based file system that is entirely in volatile memory, or it could even be a virtual disk that resides in a file (as what you will be implementing in this lab). In this way, the file system can shield the low-level details of hardware disk manipulation from higher-level programs. Recall that with paging, you needed to store the page table and other information about a processs virtual address and physical page usage. You stored this information in a page table, and the pointer to the page table is stored process control block (PCB) structure. For file systems, we need a similar type of structure to store the information about a file: things such as the filename, the block translation table, length of the file, etc. This structure is called an index node or inode for short. An inode is the metadata associated with a file, meaning that each file has one inode, and each inode represents one file.In lab 4, you kept track of which pages were free and which were in use through the use of a bitmap where each bit corresponded to 1 page. This was called the freemap. We will use the same concept in this lab to keep track of which file system blocks are in use and which are free, except that we will call it the free block vector in this lab instead of the freemap.The table below summarizes the relationship between paging terms and file system termsIt is easy in this lab to get confused between disk operations, file system operations, and file operations. Disk operations are performed by the actual physical disk hardware. These operations include reading and writing blocks, and reporting disk information such as the total size and the block size.File system operations (a.k.a. DFS operations) are performed by the file system driver in the operating system. DFS operations include opening the file system (i.e. loading it into memory), closing the file system (i.e. writing it back to the disk), allocating and freeing DFS blocks, reading and writing DFS blocks, reading and writing virtual blocks (through inodes), and block caching.It is very tempting to #define all the relevant information about the file system. For instance, the user program that formats the file system can #define the block size. It would be tempting to #define this in the include/dfs_shared.h header file, and use the same constant in the operating system. However, since the block size is configurable, if I use your file system driver on a filesystem that I created with a different block size, it should still work. If you use your #define-d constant, you will be using the wrong block size. Recall that the block size (and all other configurable file system options) are written in the superblock structure. Your driver therefore should use the values in the superblock structure after it is read from the disk, rather than a constant.You may ask, however, then how are you supposed to know how large to make your dfs_block stuctures and inode and free block vector arrays at compile time, if you wont know how big any of them are until you read the superblock at run time? The answer is that you must assign a #define-d maximum value to each of these items, and then use however many items/bytes that the superblock requires at runtime.When grading, we will take off points for using any #define-d constants where the superblock information could be used instead. ";

void main (int argc, char *argv[]){
  int handle;
  char data[] = "testtest";
  char read_data[10] = {0};

  handle = file_open("test.txt", "rw");
  
  file_write(handle, data, 8);

  file_seek(handle, -8, FILE_SEEK_CUR);
  file_read(handle, read_data, 8);

  Printf("read data: %s\n",read_data);

  file_close(handle);


  handle = file_open("test.txt", "r");
  
  Printf("read data: %s\n",read_data);

  file_write(handle, data, 8);

  file_close(handle);

  handle = file_open("big.txt", "rw");

  file_write(handle, big_text, sizeof(big_text));
  
  file_write(handle, big_text, sizeof(big_text));

  file_seek(handle, -11, FILE_SEEK_END);

  file_read(handle, read_data, 10);
  
  Printf("read data: %s\n",read_data);

  file_close(handle);


}

