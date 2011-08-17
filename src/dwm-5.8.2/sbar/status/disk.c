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
  const char *name, *realname, *mountpoint;
  Bool mounted;
  unsigned long free, total;
  DiskActions last,now,between;
} Disk;

static Disk *disks;

void setup_disk();
void get_disk_stat();
void update_media();

int main()
{
  setup_disk();
}

void update_media()
{
  
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
  disks = (Disk*)malloc(sizeof(Disk)*length);  

  for(i = 1; i < length; i++)
  {
    disks[i].name = part_names[i];
    disks[i].realname = partitions[i];
    disks[i].mountpoint = mount_points[i];

    disks[i].mounted = ((disks[i].mountpoint == NULL) ? False : True);
  }

  get_disk_stat();
}
