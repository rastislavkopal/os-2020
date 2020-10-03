#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char *
fmtname(char * path)
{
  static char buf[DIRSIZ+1];
  char *p;

  for(p =path+strlen(path); p>= path && *p != '/';p--)
    ;
  p++;

  if (strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
  return buf;
}

void readDir(char * path, char * fileName)
{
  
  int fd;
  //char buf[512], *p;
  struct stat st;
  struct dirent di;

  if ((fd = open(path,0)) < 0){
    //fprintf(2,"Find: Cannot open file: %s, strlen: %d\n", path,strlen(path));
    return;
  }

  if (fstat(fd,&st) != 0){
    fprintf(2,"Find: Cannot fstat\n");
    close(fd);
    return;
  }

  if (st.type == T_FILE){
    if (strcmp(fmtname(path),fileName)==0)
      printf("fileName\n");
  } else if (st.type == T_DIR){
    // check some error of file length..

    while(read(fd,&di,sizeof(di)) == sizeof(di)){
      if (strcmp(fileName,di.name) == 0)
	printf("%s%s\n",path,di.name);
      
      if (strcmp(di.name,".") != 0 && strcmp(di.name,"..") != 0 && strlen(di.name) != 0)
      {
	// concat path with fileName in di.name
        char * newPath;
	newPath = (char*)malloc((strlen(path)+strlen(di.name)+2) * sizeof(char));
	strcpy(newPath, path);
	strcpy(newPath + strlen(path),di.name);
	newPath[strlen(path)+strlen(di.name)] = '/';
	newPath[strlen(path)+strlen(di.name)+1] = '\0';
	//printf("Find: newpath: %s\n",newPath);

        readDir(newPath,fileName);
	free(newPath);
	}
    } 
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  char * path = "./";

  if (argc < 2 || argc > 3 ){
    printf("Wrong num of params, expected [path, filename]\n");
  } else{
    if (argc == 3 ){ // is custom directory
      if (strcmp(argv[1],".") != 0){
	memset(path,'\0',sizeof(argv[1]));
	strcpy(path,argv[1]);
	//readDir(path,argv[2]);
	if (path[strlen(path)-1] != '/'){ // last char should be /
	  char * newPath;
	  newPath = (char*)malloc((strlen(path)+2) * sizeof(char));
	  strcpy(newPath, path);
	  newPath[strlen(path)] = '/';
	  newPath[strlen(path)+1] = '\0';
	  readDir(newPath,argv[2]);
	  exit(0);
	}
      }else {
	readDir(path,argv[2]);
	exit(0);
      }
    } else
      readDir(path,argv[1]);
  }
  exit(0);
}
