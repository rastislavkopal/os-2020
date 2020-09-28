#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char * argv[]){
  
  if (argc != 2){
    printf("Wrong number of arguments, expects 1\n");
    exit(1);
  }
  int n = atoi(argv[1]);
  if (n == 0){
   printf("Invalid argument type, expects number\n");
   exit(1);
  }
  sleep(n);
  printf("yoooo wasssup there\n");
  exit(0);
}
