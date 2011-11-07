void cpu_usage();
int get_ncpus();
void setup_cpu();

static unsigned long **last;
static unsigned long **current;

static double *cpuloads;
static int ncpus;

typedef struct {
  int total, running, blocked;
} Processes;

Processes processes;

void setup_cpu()
{
  // creating values
  FILE * fp;
  char * line = NULL;
  size_t len = 0;
  ssize_t read;
  int i, j, cpu, usg;
  cpu = 0;


  // initializing
  ncpus = get_ncpus();

  // reserving memory
  cpuloads = (double*)malloc((sizeof(double)*ncpus));
  last    = (unsigned long**)malloc((sizeof(unsigned long*)*ncpus));
  current = (unsigned long**)malloc((sizeof(unsigned long*)*ncpus));
  for(i = 0;i < ncpus;i++){
    last[i] = (unsigned long*)malloc((sizeof(unsigned long)*ncpus));
    current[i] = (unsigned long*)malloc((sizeof(unsigned long)*ncpus));
  }

 
  // open /proc/stat
  fp = fopen("/proc/stat", "r");
  if (fp == NULL){
      printf("\nfailed to read /proc/stat\n");
      return;
  }
  
  // reading line by line
  while ((read = getline(&line, &len, fp)) != -1) {
    if(line[0] == 'c' && line[3] != ' '){
      usg = 0;
      // geting values vor user, nice, system, idle  
      for(i = 4;i < len;i++){
        if(line[i] == ' '){
          last[cpu][usg] = atoi(line+i+1);
          usg++;
          if(usg == 4) break;
        }
      }
    }else if(line[3] == ' ') continue;
    else break;
    cpu++;
  }

  if (line) free(line);
  fclose(fp);
}



int get_ncpus()
{
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int i, cpu;
    cpu = 0;


    fp = fopen("/proc/stat", "r");
    if (fp == NULL){
         printf("\nfailed to read /proc/stat\n");
         return;
    }

    while ((read = getline(&line, &len, fp)) != -1) {
      if(line[0] == 'c' && line[3] != ' ') cpu++;
      else if(line[3] == ' ') continue;
      else break;
    }

    if (line) free(line);
    fclose(fp);
    return cpu;
}


void cpu_usage()
{
   FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int i, j, cpu, usg;
    cpu = 0;

    fp = fopen("/proc/stat", "r");
    if (fp == NULL){
      printf("\nfailed to read /proc/stat\n");
      return;
    }

    while ((read = getline(&line, &len, fp)) != -1) {
      if(line[0] == 'c' && line[3] != ' ' && line[3] != 't'){
        usg = 0;
        for(i = 4;i < len;i++){
          if(line[i] == ' '){
            current[cpu][usg] = atoi(line+i+1);
            usg++;
            if(usg == 4) break;
          }
        }
      }else if(line[3] == ' ' || line[3] == 't') continue;
      else if(line[0] == 's') break;
      else if(line[0] == 'p'){
        if(line[4] == 'e'){ // processes total
          for(i = 4;i < len;i++){
            if(line[i] == ' '){
              processes.total = atoi(line+i+1);
              break;
            }
          }
        }else if(line[6] == 'r'){ // processes running
          for(i = 4;i < len;i++){
            if(line[i] == ' '){
              processes.running = atoi(line+i+1);
              break;
            }
          }
        }else if(line[6] == 'b'){ // processes blocked
          for(i = 4;i < len;i++){
            if(line[i] == ' '){
              processes.blocked = atoi(line+i+1);
              break;
            }
          }
        }
      }
      cpu++;
    }
    if (line) free(line);
    fclose(fp);
 
    
    for( i = 0;i < ncpus; i++){
      cpuloads[i] = 1 - (double)(current[i][3] - last[i][3]) / (double)(current[i][0] + current[i][1] + current[i][2] + current[i][3] - 
                                  last[i][0] - last[i][1] - last[i][2] - last[i][3]);
      for(j = 0;j < 4;j++)
        last[i][j] = current[i][j];
    }
}

