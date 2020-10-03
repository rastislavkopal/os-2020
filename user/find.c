#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

int match(char*, char*);

void
find(char *dir, char *para)
{
    char buf[512], *p;
    int fd;

    struct dirent de;

    struct stat st;

    //printf("dir: %s\n", dir);

    if((fd = open(dir, 0)) < 0){
        fprintf(2, "find: cannot open %s\n", dir);
        return;
    }

    if(fstat(fd, &st) < 0){
        fprintf(2, "find: cannot stat %s\n", dir);
        close(fd);
        return;
    }

    switch(st.type){
    case T_FILE:
        fprintf(2, "find: %s is a file, not directory\n", dir);
        break;
    case T_DIR:
        if(strlen(dir) + 1 + DIRSIZ + 1 > sizeof(buf)){
            fprintf(2, "find: path too long\n");
            break;
        }
        strcpy(buf, dir);

        //printf("buf: %s\n", buf);

        p = buf + strlen(buf);
        *p++ = '/';
        while(read(fd, &de, sizeof(de))){
            //printf("de.name: %s %d\n", de.name, strcmp(de.name, "."));
            if(de.inum == 0)
                continue;
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;
            if(stat(buf, &st) < 0){
                fprintf(2, "find: cannot stat %s\n", buf);
                continue;
            }
            switch(st.type){
            case T_FILE:
                if(match(para, de.name)){
                    printf("%s\n", buf);
                }
                break;
            case T_DIR:
                if(strcmp(de.name, ".") && strcmp(de.name, "..") ) find(buf, para);
                break;
            }
        }
        break;
    }
}

int
main(int argc, char *argv[])
{
    // printf("%d\n", argc);
    if(argc == 2){
        find(".", argv[1]);
    } else if(argc == 3){
        find(argv[1], argv[2]);
    } else {
        printf("find: argument error\n");
    }
    exit(0);
}

// Regexp matcher from Kernighan & Pike,
// The Practice of Programming, Chapter 9.

int matchhere(char*, char*);
int matchstar(int, char*, char*);

int
match(char *re, char *text)
{
  if(re[0] == '^')
    return matchhere(re+1, text);
  do{  // must look at empty string
    if(matchhere(re, text))
      return 1;
  }while(*text++ != '\0');
  return 0;
}

// matchhere: search for re at beginning of text
int matchhere(char *re, char *text)
{
  if(re[0] == '\0')
    return 1;
  if(re[1] == '*')
    return matchstar(re[0], re+2, text);
  if(re[0] == '$' && re[1] == '\0')
    return *text == '\0';
  if(*text!='\0' && (re[0]=='.' || re[0]==*text))
    return matchhere(re+1, text+1);
  return 0;
}

// matchstar: search for c*re at beginning of text
int matchstar(int c, char *re, char *text)
{
  do{  // a * matches zero or more instances
    if(matchhere(re, text))
      return 1;
  }while(*text!='\0' && (*text++==c || c=='.'));
  return 0;
}

