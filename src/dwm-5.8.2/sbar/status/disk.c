#include <sys/vfs.h>
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#define LENGTH(X)               (sizeof X / sizeof X[0])

static const char *partitions[]     = { "sda","sda1","sda3", "dm-1","dm-2","dm-3" }; // partitons to show informations of
static const char *part_names[]     = { "all","boot","Dings","swap","root","home" }; // name of the partitons
static const char *mount_points[]   = { NULL,"/boot","/media/Dings/",NULL,"/","/home" }; // mountpoints of the partitions 

typedef struct {
  unsigned long long read, write, readtime, writetime, ios_in_process, iostime;
} DiskActions;

typedef struct {
  char *name, *realname, *mountpoint;
  Bool mounted, active;
  unsigned long free, total;
  DiskActions last,now,between;
} Disk;

static Disk disks[20];

void setup_disk();
void get_disk_stat();
void update_media();
void update_stats();

int main()
{
  setup_disk();
  update_stats();
}

void update_stats()
{
  // initializing
  FILE * fp;
  char * line = NULL;
  size_t len = 0;
  ssize_t read;
  int i, j, k, l, m;
  
  // open /proc/stat
  fp = fopen("/proc/diskstats", "r");
  if (fp == NULL){
      printf("\nfailed to read /proc/diskstats\n");
      return;
  }
  
  // reading line by line
  while ((read = getline(&line, &len, fp)) != -1) {
    disks[i].active = True;

    if(line[16] == " ") k = 2; // = disk
    else k = 3;
    
    // saveing partition name
    disks[i].realname[k+1] = "\x00";  
    for(;k >= 0;k--)
      disks[i].realname[k] = line[j+k];  

    l = 1;
    for(j = 13+k+1; j < len; j++){
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
        if(line[j] == " ") break;
      }
    }    
    i++;
  }
  disks[i].active = False; // no more dis from now on  

  if (line) free(line);
  fclose(fp);
 
  disks[i].between.read           = disks[i].now.read           - disks[i].last.read;
  disks[i].between.write          = disks[i].now.write          - disks[i].last.write;
  disks[i].between.readtime       = disks[i].now.readtime       - disks[i].last.readtime;
  disks[i].between.writetime      = disks[i].now.writetime      - disks[i].last.writetime;
  disks[i].between.ios_in_process = disks[i].now.ios_in_process - disks[i].last.ios_in_process;
  disks[i].between.iostime        = disks[i].now.iostime        - disks[i].last.iostime;

  disks[i].last.read           = disks[i].now.read;
  disks[i].last.write          = disks[i].now.write;
  disks[i].last.readtime       = disks[i].now.readtime;
  disks[i].last.writetime      = disks[i].now.writetime;
  disks[i].last.ios_in_process = disks[i].now.ios_in_process;
  disks[i].last.iostime        = disks[i].now.iostime;
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

  for(i = 1; i < length; i++)
  {
    disks[i].name = part_names[i];
    disks[i].realname = partitions[i];
    disks[i].mountpoint = mount_points[i];

    disks[i].mounted = ((disks[i].mountpoint == NULL) ? False : True);
  }

  get_disk_stat();
}
