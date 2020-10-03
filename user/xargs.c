#include "kernel/types.h"
#include "kernel/param.h" // MAXARG
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
  int argv_len;
  char buf[128] = {0};
  char *max_args[MAXARG];

  for (int i = 1; i < argc; i++) {
    max_args[i - 1] = argv[i];
  }
//for each line terminated by endLn character
  while (gets(buf, sizeof(buf))) {
    int buf_len = strlen(buf);
    if (buf_len < 1)
      break;
    buf[buf_len - 1] = 0;

    argv_len = argc - 1;
    char *p = buf;

    while (*p) {
      while (*p && (*p == ' '))
        *p++ = 0;
      if (*p)
        max_args[argv_len++] = p;
      while (*p && (*p != ' '))
        p++;
    }

    if (argv_len >= MAXARG) {
      printf("Too many args\n");
      exit(1);
    }

    if (argv_len < 1) {
      printf("Too few args\n");
      exit(1);
    }

    max_args[argv_len] = 0;

    if (fork()) {
      wait(0);
    } else {
      exec(max_args[0], max_args);
      exit(0);
    }
  }
  exit(0);
}
