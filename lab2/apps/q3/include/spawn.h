#ifndef __USERPROG__
#define __USERPROG__


typedef struct buffer {
  char buffer[BUFFERSIZE];
  int currptr;
  int endptr;
} buffer_l;


#define FILENAME_PRODUCER "producer.dlx.obj"
#define FILENAME_CONSUMER "consumer.dlx.obj"

#endif
