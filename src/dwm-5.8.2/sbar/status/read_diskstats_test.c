#include <sys/vfs.h>
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <stdlib.h> 
#include <sys/types.h> 
#include <sys/stat.h> 
#include <dirent.h> 
#define LENGTH(X)               (sizeof X / sizeof X[0])



int main()
{
  // initializing
  FILE * fp;
  char * line = NULL;
  size_t len = 0;
  ssize_t read;
  int i, j, cpu, usg;
  cpu = 0;
  
  // open /proc/stat
  fp = fopen("/proc/diskstats", "r");
  if (fp == NULL){
      printf("\nfailed to read /proc/diskstats\n");
      return;
  }
  
  // reading line by line
  while ((read = getline(&line, &len, fp)) != -1) {
    printf("%s",line);
  }

  if (line) free(line);
  fclose(fp);
}
