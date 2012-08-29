#include <sys/vfs.h>
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#define LENGTH(X) (sizeof X / sizeof X[0])
#ifndef MAXPARTITIONS
#define MAXPARTITIONS 20
#endif

typedef struct {
  unsigned long long read, write, readtime, writetime, ios_in_process, iostime;
} DiskActions;

typedef struct {
  char *path, realname[256], *mountpoint;
  Bool active;
  unsigned long long free, total, avil, used;
  long double pfree, pavil, pused;
  DiskActions last,now,between;
} Disk;

static Disk disks[MAXPARTITIONS];
static char mountpoints[MAXPARTITIONS][512];
static char paths[MAXPARTITIONS][512];
static int mounted_volumes;

void setup_disk();
void get_disk_stat();
void update_mounts();
void update_stats();
void merge_mount_path();
void update_disk();

#ifdef DISK_MAIN
int main()
{
  setup_disk();
  update_disk();
  sleep(1);
  update_disk();

}
#endif

void update_disk()
{
  update_stats();
  update_mounts();
  merge_mount_path();
  get_disk_stat();
}

void merge_mount_path()
{
  FILE * fp;
  char * line = NULL;
  size_t len = 0;
  ssize_t read;
  int i,j;
  char buf[256], *linebreak;

  for(i = 0; i < MAXPARTITIONS;i++){
    if(!disks[i].active) break;

    if(disks[i].realname[0] == 's' && disks[i].realname[1] == 'd' && disks[i].realname[3] != '\x00'){
      // find mountpoint and path who belong to the disk 
      for(j = 0;j < MAXPARTITIONS;j++){
        if(strstr(paths[j],disks[i].realname)){
          disks[i].path = &paths[j];
          disks[i].mountpoint = &mountpoints[j];
          break;
        }
      }     
      if(j == MAXPARTITIONS){
        for(j = 0; j < MAXPARTITIONS; j++)          
          if(paths[j][0] == '\x00') break;
  
        sprintf(paths[j],"/dev/%s",disks[i].realname);

        disks[i].path = &paths[j];
        disks[i].mountpoint = NULL;
      }
    }else if(disks[i].realname[0] == 's' && disks[i].realname[1] == 'd' && disks[i].realname[3] == '\x00'){
      for(j = 0; j < MAXPARTITIONS; j++)          
        if(paths[j][0] == '\x00') break;
  
      sprintf(paths[j],"/dev/%s",disks[i].realname);

      disks[i].path = &paths[j];
      disks[i].mountpoint = NULL;


    }else if(disks[i].realname[0] == 'd'){ // if /dev/mapper read the name from   
      sprintf(buf,"/sys/block/%s/dm/name",disks[i].realname);

      fp = fopen(buf, "r");
      if (fp == NULL){
        char buffer[300];
        sprintf(buffer, "failed to read %s",buf);
        log_str(buffer, LOG_WARNING);
        return;
      }
  
      // reading line by line
      if((read = getline(&line, &len, fp)) != -1) {
        for(j = 0; j < len; j++){
          if(line[j] == '\n') line[j] = '\x00';
          if(line[j] == '\x00') break;
        }

        for(j = 0;j < MAXPARTITIONS;j++){
          if(!strcmp(paths[j]+12,line) && paths[j][5] != 's'){
            disks[i].path = &paths[j];
            if(paths[j][0] == '\x00'){
              paths[j][0] = '/';
              break;
            }
              
            disks[i].mountpoint = &mountpoints[j];
            break;
          }
        }  
        if(j == MAXPARTITIONS){
          for(j = 0; j < MAXPARTITIONS; j++)          
            if(paths[j][0] == '\x00') break;
  
          sprintf(paths[j],"/dev/mapper/%s",line);

          disks[i].path = &paths[j];
          disks[i].mountpoint = NULL;
        }
      }   

      fclose(fp);
    }else { // volume not mounted
      disks[i].path = NULL;
      disks[i].mountpoint = NULL;
    }
  }   
  if (line) free(line);
}

void update_mounts()
{
  // initializing
  FILE * fp;
  char * line = NULL;
  size_t len = 0;
  ssize_t read;
  int i, j, k;
  mounted_volumes = i = 0;
  
  // open /proc/stat
  fp = fopen("/proc/mounts", "r");
  if (fp == NULL){
      log_str("failed to read /proc/mounts", LOG_WARNING);
      return;
  }
  
  // reading line by line
  while ((read = getline(&line, &len, fp)) != -1) {
    if(strstr(line, "/dev/sd") || strstr(line, "/dev/mapper/")){ //TODO make a string arry for lvm volumes
      if(i == MAXPARTITIONS) break;        
      mounted_volumes++;

      for(j = 0;j < len;j++){
        if(line[j] == ' ') break;
        paths[i][j] = line[j]; 
      }
      paths[i][j] = '\x00';

      for(k = 0, j++;j < len;j++,k++){
        if(line[j] == ' ') break;
        mountpoints[i][k] = line[j]; 
      }
      mountpoints[i][j] = '\x00';

      i++;
    }
  }

  if (line) free(line);
  fclose(fp);

  for(;i < MAXPARTITIONS;i++){
    paths[i][0] = '\x00';
    mountpoints[i][0] = '\x00';
  }

}

void update_stats()
{
  // initializing
  FILE * fp;
  char * line = NULL;
  size_t len = 0;
  ssize_t read;
  int i, j, k, l, m;
  i = 0;
  
  // open /proc/stat
  fp = fopen("/proc/diskstats", "r");
  if (fp == NULL){
      log_str("failed to read /proc/diskstats", LOG_WARNING);
      return;
  }
  
  // reading line by line
  while ((read = getline(&line, &len, fp)) != -1) {
    disks[i].active = True;

    if(line[16] == ' ') k = j = 2; // = disk
    else k = j = 3;
    
    // saveing partition name
    disks[i].realname[k+1] = '\x00';  
    for(;k >= 0;k--)
      disks[i].realname[k] = line[13+k];  

    l = 0 ;
    for(j = 13+j+1; j < len; j++){
      switch(l){
        case 3:
          disks[i].now.read = (atoll(line+j) * 512);
          break;

        case 4:
          disks[i].now.readtime = atoll(line+j);
          break;

        case 7:
          disks[i].now.write = (atoll(line+j) * 512);
          break;
    
        case 8:
          disks[i].now.writetime = atoll(line+j);
          break;

        case 9:
          disks[i].now.ios_in_process = atoi(line+j);
          break;
          
        case 10:
          disks[i].now.iostime = atoll(line+j);
          break;

        case 11:
           j = len;
           break;
      }
      l++;
      
      for(; j < len; j++){
        if(line[j] == ' ') break;
      }
    }    
    i++;
  }
  disks[i].active = False; // no more dis from now on  

  if (line) free(line);
  fclose(fp);
  
  for(j = 0;j < MAXPARTITIONS;j++){
    if(j == i) break;
    disks[j].between.read           = (disks[j].now.read           - disks[j].last.read);
    disks[j].between.write          = (disks[j].now.write          - disks[j].last.write);
    disks[j].between.readtime       = (disks[j].now.readtime       - disks[j].last.readtime);
    disks[j].between.writetime      = (disks[j].now.writetime      - disks[j].last.writetime);
    disks[j].between.ios_in_process = (disks[j].now.ios_in_process - disks[j].last.ios_in_process);
    disks[j].between.iostime        = (disks[j].now.iostime        - disks[j].last.iostime);

    disks[j].last.read           = disks[j].now.read;
    disks[j].last.write          = disks[j].now.write;
    disks[j].last.readtime       = disks[j].now.readtime;
    disks[j].last.writetime      = disks[j].now.writetime;
    disks[j].last.ios_in_process = disks[j].now.ios_in_process;
    disks[j].last.iostime        = disks[j].now.iostime;
  }
}

void get_disk_stat()
{
  struct statfs fs;
  unsigned long long bfree, bsize, btotal, bavil;
  char buf[30];
  int i;
  

  for(i = 1; i < MAXPARTITIONS; i++)
  {
    if(!disks[i].active) break;

    if(disks[i].mountpoint != NULL){
      statfs(disks[i].mountpoint, &fs);
      bfree = fs.f_bfree;
      bavil = fs.f_bavail;
      bsize = fs.f_bsize;
      btotal = fs.f_blocks;

      disks[i].free = bfree * bsize;
      disks[i].avil = bavil * bsize;
      disks[i].used = (btotal - bfree) * bsize;
      disks[i].total = btotal * bsize;

      disks[i].pfree = ((long double)disks[i].free / (long double)disks[i].total);
      disks[i].pused = ((long double)disks[i].used / (long double)disks[i].total);
      disks[i].pavil = ((long double)disks[i].avil / (long double)disks[i].total);
    }
  }
}

void setup_disk()
{
  update_stats();
}
