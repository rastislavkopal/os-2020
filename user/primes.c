#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define MAX_PRIME 35

static int *pipeLine;

void incPipeLine(int n){
  if (n == 0){
    pipeLine = (int*) malloc(2*sizeof(int));
    return;
  }
  int* tmpArr = (int*) malloc((2*n+2)*sizeof(int));
  memcpy(tmpArr,&pipeLine[0],n*2+2);
  pipeLine = tmpArr;
}

void primesProc(int counter){
  int prime,x;
  read(pipeLine[2*counter],&prime,sizeof(int));
  printf("%d,",prime);

  if (prime > MAX_PRIME && prime == 0)// if prime > MAX_PRIME, end
  {
    // close all that should be close TODO
    return;
  }

  int fd = fork();
  incPipeLine(counter+1);

  if (fd == 0){ // child
    primesProc(counter+1);
    close(2*counter);
  } 
  else { // parent
    while(read(pipeLine[2*(counter)],&x,sizeof(int)))
      if (x % prime != 0) {
	write(pipeLine[2*counter+1],&x,sizeof(int));    
    close(2*counter+1);
    }
  }
}

int main(int argc,char * argv[]){
  incPipeLine(0);
  int fd;
  pipe(&pipeLine[0]); // get parent pipe

  fd = fork();

  if (fd < 0){
    fprintf(2,"Primes: could not pipe");
  }
  else if (fd == 0){ // child
    primesProc(0);
  }
  else { // parent
    for(int i=2;i<=MAX_PRIME;i++)
      write(pipeLine[1],&i,sizeof(i)); 

    close(pipeLine[1]);
  }
  exit(0);
}
