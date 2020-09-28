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
  char fullPath[512]; // TODO remove after test
  strcpy(fullPath,path);
  strcpy(fullPath +strlen(path),fileName);
  fullPath[strlen(path)+strlen(fileName)] = '\0';

  if ((fd = open(path,0)) < 1){
    fprintf(2,"Find: Cannot open file: %s\n", path);
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
      if (strcmp(fileName,di.name) == 0){
      printf("%s%s\n",path,di.name);
      }
      
      if (strcmp(di.name,".") != 0 && strcmp(di.name,"..") != 0 && strlen(di.name) != 0)
      {
	// concat path with fileName in di.name
        char * newPath;
	newPath = malloc(strlen(path)+strlen(di.name)+2);
	strcpy(newPath, path);
	strcpy(newPath + strlen(path),di.name);
	newPath[strlen(path)+strlen(di.name)] = '/';
	newPath[strlen(path)+strlen(di.name)+1] = '\0';
	printf("Find: newpath: %s\n",newPath);

        readDir(newPath,fileName);
      }else{
	printf("Well, that aint right\n");
      }
    }
  } else{
    printf("Find: Opened smthing else\n");
  }
}

int
main(int argc, char *argv[])
{
  char * path = "./";

  if (argc < 2){
    printf("Wrong num of params, expected [path, filename]\n");
  }
    
  if (argc == 3 ){
    // is custom directory
    memset(path,'\0',sizeof(argv[1]));
    strcpy(path,argv[1]);
  }

  // printf("%s\n",path);
  readDir(path,argv[2]);
  exit(0);
}
