#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char * argv[]){

  int fd_to_child[2];
  int fd_to_parent[2];
  char a = 'x';

  if (pipe(fd_to_child) == -1 || pipe(fd_to_parent) == -1){
    printf("An error occured while creating pipe\n");
    exit(1);
  }

  int pid = fork();

  if (pid < 0 ){
    printf("An error occured while forking\n");
    exit(1);
  } else if (pid == 0){
    // child's process
    read(fd_to_child[0],&a,1);
    printf("%d: received ping\n",getpid());
    write(fd_to_parent[1],"b",1);
  } else {
    // parent's process
    write(fd_to_child[1],"a",1);
    int status;
    wait(&status);
    // if (status is err)
    read(fd_to_parent[0],&a,1);
    printf("%d: received pong\n",getpid());
  }

  exit(0);
}

