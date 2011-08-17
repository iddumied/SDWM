#include <sys/vfs.h>
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#define LENGTH(X)               (sizeof X / sizeof X[0])

static const char *partitions[]     = { "sda","sda1","sda3", "dm-1","dm-2","dm-3" }; // partitons to show informations of
static const char *part_names[]     = { "all","boot","Dings","swap","root","home" }; // name of the partitons
static const char *mount_points[]   = { NULL,"/boot","/media/Dings/",NULL,"/","/home" }; // mountpoints of the partitions 
#define MAXPARTITIONS 10                    // max number of posible partitions mounted if you mount more it will crash

typedef struct {
  unsigned long long read, write, readtime, writetime, ios_in_process, iostime;
} DiskActions;

typedef struct {
  char name[256], realname[256], mountpoint[512];
  Bool mounted, active;
  unsigned long free, total;
  DiskActions last,now,between;
} Disk;

static Disk disks[MAXPARTITIONS];

void setup_disk();
void get_disk_stat();
void update_mounts();
void update_stats();

int main()
{
  update_stats();
  while(1==1){
  sleep(1);
  update_stats();

  printf("| write | read | write time | read time | I/Os | I/Os time || write | read | write time | read time | I/Os | I/Os time |\n");
  

  int i;
  for(i = 0;i < MAXPARTITIONS;i++){
  if(!disks[i].active) break;

    printf("%llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu \n",disks[i].between.write,disks[i].between.read,disks[i].between.writetime,disks[i].between.readtime,disks[i].between.ios_in_process,disks[i].between.iostime,disks[i].now.write,disks[i].now.read,disks[i].now.writetime,disks[i].now.readtime,disks[i].now.ios_in_process,disks[i].now.iostime);

  }
}
}

void update_mounts()
{
  // initializing
  FILE * fp;
  char * line = NULL;
  size_t len = 0;
  ssize_t read;
  int i, j;
  
  // open /proc/stat
  fp = fopen("/proc/mounts", "r");
  if (fp == NULL){
      printf("\nfailed to read /proc/mounts\n");
      return;
  }
  
  // reading line by line
  while ((read = getline(&line, &len, fp)) != -1) {

  }

  if (line) free(line);
  fclose(fp);

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
  unsigned long long bfree, bsize, btotal;
  char buf[30];
  int i;


  for(i = 1; i < LENGTH(partitions); i++)
  {
    if(disks[i].mounted){
      statfs(disks[i].mountpoint, &fs);
      bfree = fs.f_bfree;
      bsize = fs.f_bsize;
      btotal = fs.f_blocks;

      disks[i].free = bfree * bsize;
      disks[i].total = btotal * bsize;
      printf("%s, %s, %s: %llu bytes left of %llu - %Lf      bsize %d, bfree %d\n", disks[i].mountpoint, disks[i].realname, disks[i].name, disks[i].free, disks[i].total, ((long double)disks[i].free / (long double)disks[i].total), bsize, bfree);
    }
  }
}

void setup_disk()
{
  int i,j,length = LENGTH(partitions);

  get_disk_stat();
}
