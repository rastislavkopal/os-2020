#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"

int endCharAtPos(char * arr){
  int i = strlen(arr);
  char * p = &arr[strlen(arr)];

  while (*p-- != '\n')
    i--;
  return i;
}

/*
 * Returns whole input that was received in stdin
 */
char * readFromStdIn(){
  char buf[512];
  int i = 0;

  read(0,&buf[i++],sizeof(char));
  while(buf[i-1] != '\n'){
    read(0,&buf[i++],1);
  }
  buf[i] = '\0';
  char * arr = (char*)malloc(strlen(buf)*sizeof(char));
  memcpy(arr,&buf[0],strlen(buf));
  return &arr[0];
}
/*
 * Find EndLine character in range startPos -> till end of char
 */
int findEndLine(char * str, int startPos){
  int j = 0;
  while(1)
    if (str[startPos+j] == '\n')
      return startPos+j;
    else
      ++j;

  return -1;
}
/*
 * Append to argv array strings from input line
 */
void concatArgv(char * argv[], int argvPos, char * toAdd)
{
 int startIndex = 0,findIndex = 0;
 while(1){
   if (toAdd[findIndex] == '\n' || toAdd[findIndex] == '\0'){ // add last
    argv[argvPos] = (char*)malloc((findIndex-startIndex)*sizeof(char));
    memcpy(argv[argvPos],&toAdd[startIndex],findIndex-startIndex);  
    return;
   }
   if (toAdd[findIndex] == ' '){
    argv[argvPos] = (char*)malloc((findIndex-startIndex)*sizeof(char));
    memcpy(argv[argvPos],&toAdd[startIndex],findIndex-startIndex);  
    argvPos++;
    startIndex = findIndex;
   }
   findIndex++;
 }
}

int
main (int argc,char * argv[]){
  char * in = readFromStdIn();

  int startPos = 0;
  char * argv_exec[MAXARG];

  if (argc<2){
    fprintf(2,"Xargs: Not enough args");
    exit(1);
  }

  for(int i = 1;i < argc;i++){
     argv_exec[i-1] = argv[i];
  }
  while(1){
    int endLnPos = findEndLine(in,startPos);
    int fd = fork();
    if (fd == 0){ // child
      char * argStr = (char*) malloc((endLnPos-startPos)*sizeof(char));
      memcpy(&argStr[0],&in[startPos],endLnPos-startPos);
     // argv_exec[argc-1] = argStr;
      concatArgv(argv_exec,argc-1,argStr);
      exec(argv[1],argv_exec);
    } else if (fd > 0){ // parent
      int status;
      wait(&status);
      if (status < 0){ // error while waiting
	fprintf(2,"Xargs, problem while wait()");
	exit(1);
      }
      close(fd);
    } else { //error while forking
      fprintf(2,"Xargs: Couldnt fork\n");
      exit(1);
    }
  
    startPos =  endLnPos;
  
    if (endLnPos == endCharAtPos(in))
      break;
  }

  exit(0);
}
