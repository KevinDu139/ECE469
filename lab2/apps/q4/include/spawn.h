#ifndef __USERPROG__
#define __USERPROG__


typedef struct buffer {
  char buffer[BUFFERSIZE];
  int currptr;
  int endptr;
} buffer_l;


#define FILENAME_PRODUCER "bin/producer.dlx.obj"
#define FILENAME_CONSUMER "bin/consumer.dlx.obj"

#endif
