#include <sys/vfs.h>
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#define LENGTH(X)               (sizeof X / sizeof X[0])

#define MAXPARTITIONS 20                    // max number of posible partitions mounted if you mount more it will crash

typedef struct {
  char name[5];
  unsigned long long bytes;
  Bool used;
} Partition;

typedef struct {
  unsigned long long read, write, readtime, writetime, ios_in_process, iostime;
} DiskActions;

typedef struct {
  char path[512], realname[256], mountpoint[512];
  Bool mounted, active;
  unsigned long free, total, avil, used;
  DiskActions last,now,between;
} Disk;

static Disk disks[MAXPARTITIONS];
static char mountpoints[MAXPARTITIONS][512];
static char paths[MAXPARTITIONS][512];
static Partition partitions[MAXPARTITIONS];

void setup_disk();
void get_disk_stat();
void update_mounts();
void update_stats();
void merge_mount_path();

int main()
{
  update_stats();
  update_mounts();
  while(1==1){
  sleep(1);
  update_stats();
  update_mounts();

  printf("| write | read | write time | read time | I/Os | I/Os time || write | read | write time | read time | I/Os | I/Os time |\n");
  

  int i;
  for(i = 0;i < MAXPARTITIONS;i++){
    if(!disks[i].active) break;

    printf("%llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu -- %s %s %s  --  %s %s\n",disks[i].between.write,disks[i].between.read,disks[i].between.writetime,disks[i].between.readtime,disks[i].between.ios_in_process,disks[i].between.iostime,disks[i].now.write,disks[i].now.read,disks[i].now.writetime,disks[i].now.readtime,disks[i].now.ios_in_process,disks[i].now.iostime, disks[i].realname, disks[i].path, disks[i].mountpoint, mountpoints[i], paths[i]);
    
  }
    
}
}

void merge_mount_path()
{
    
}

void update_mounts()
{
  // initializing
  FILE * fp;
  char * line = NULL;
  size_t len = 0;
  ssize_t read;
  int i, j, k;
  i = 1;
  
  // open /proc/stat
  fp = fopen("/proc/mounts", "r");
  if (fp == NULL){
      printf("\nfailed to read /proc/mounts\n");
      return;
  }
  
  // reading line by line
  while ((read = getline(&line, &len, fp)) != -1) {
    if(strstr(line, "/dev/sd") || strstr(line, "/dev/mapper/")){ //TODO make a string arry for lvm volumes
      if(i == MAXPARTITIONS) break;        

      for(j = 0;j < len;j++){
        if(line[j] == ' ') break;
        paths[i][j] = line[j]; 
      }

      for(k = 0, j++;j < len;j++,k++){
        if(line[j] == ' ') break;
        mountpoints[i][k] = line[j]; 
      }
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
      printf("\nfailed to read /proc/diskstats\n");
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
          disks[i].now.write = atoll(line+j);
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
    if(disks[i].mounted){
      statfs(disks[i].mountpoint, &fs);
      bfree = fs.f_bfree;
      bavil = fs.f_bavail;
      bsize = fs.f_bsize;
      btotal = fs.f_blocks;

      disks[i].free = bfree * bsize;
      disks[i].avil = bavil * bsize;
      disks[i].used = (btotal - bfree) * bsize;
      disks[i].total = btotal * bsize;
    //  printf("%s, %s, %s: %llu bytes left of %llu - %Lf      bsize %d, bfree %d\n", disks[i].mountpoint, disks[i].realname, disks[i].name, disks[i].free, disks[i].total, ((long double)disks[i].free / (long double)disks[i].total), bsize, bfree);
    }
  }
}

void setup_disk()
{
  get_disk_stat();
}
