#include "usertraps.h"
#include "misc.h"

  int *test1;
  int *test2;
  int *test3;
  int *test4;


void main (int argc, char *argv[])
{


  Printf("starting test code\n");

  test1 = (int*) malloc(200);
  test2 = (int*) malloc(500);
  test3 = (int*) malloc(50);
  test4 = (int*) malloc(50);

  mfree(test1);
  mfree(test4);
  mfree(test3);
  mfree(test2);


}
