void cpu_usage();
int get_ncpus();
void setup_cpu();

typedef struct {
  int total, running, blocked;
} Processes;

typedef struct {
  unsigned long **last;
  unsigned long **current;
  char *name, *herz;
  double *cpuloads;
  int ncpus;
  Processes processes
} CPUInfo;

static CPUInfo cpuinfo;

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
  cpuinfo.ncpus = get_ncpus();

  // reserving memory
  cpuinfo.cpuloads = (double*)malloc((sizeof(double)*cpuinfo.ncpus));
  cpuinfo.last    = (unsigned long**)malloc((sizeof(unsigned long*)*cpuinfo.ncpus));
  cpuinfo.current = (unsigned long**)malloc((sizeof(unsigned long*)*cpuinfo.ncpus));
  for(i = 0;i < cpuinfo.ncpus;i++){
    cpuinfo.last[i] = (unsigned long*)malloc((sizeof(unsigned long)*cpuinfo.ncpus));
    cpuinfo.current[i] = (unsigned long*)malloc((sizeof(unsigned long)*cpuinfo.ncpus));
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
          cpuinfo.last[cpu][usg] = atoi(line+i+1);
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
  setup_cpu_name();
}

void setup_cpu_name() {
  // creating values
  FILE * fp;
  char * line = NULL;
  size_t len = 0;
  ssize_t read;
  int i, j;

 
  fp = popen("cat /proc/cpuinfo |grep \"model name\"", "r");
  if (fp == NULL){
      printf("\nfailed to read /proc/cpuinfo\n");
      return;
  }
  
  // reading line by line
  if ((read = getline(&line, &len, fp)) != -1) {
    for (i = 0; i < len; i++){
      if (line[i] == ':'){
        cpuinfo.name = (char *) malloc(sizeof(char) * len - i);
          
        for (j = i+2; j < len && line[j+1] != '@'; j++) {
          cpuinfo.name[j-i-2] = line[j];
        }
        cpuinfo.name[j-i-2] = '\x00';

      } else if (line[i] == '@') {
        cpuinfo.herz = (char *) malloc(sizeof(char) * len - i);

        for (j = i+2; j < len && line[j+1] != ' '; j++) {
          cpuinfo.herz[j-i-2] = line[j];
        }
        cpuinfo.herz[j-i-2] = '\x00';
      }
    }
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
            cpuinfo.current[cpu][usg] = atoi(line+i+1);
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
              cpuinfo.processes.total = atoi(line+i+1);
              break;
            }
          }
        }else if(line[6] == 'r'){ // processes running
          for(i = 4;i < len;i++){
            if(line[i] == ' '){
              cpuinfo.processes.running = atoi(line+i+1);
              break;
            }
          }
        }else if(line[6] == 'b'){ // processes blocked
          for(i = 4;i < len;i++){
            if(line[i] == ' '){
              cpuinfo.processes.blocked = atoi(line+i+1);
              break;
            }
          }
        }
      }
      cpu++;
    }
    if (line) free(line);
    fclose(fp);
 
    
    for( i = 0;i < cpuinfo.ncpus; i++){
      cpuinfo.cpuloads[i] = 1 - (double)(cpuinfo.current[i][3] - cpuinfo.last[i][3]) / (double)(cpuinfo.current[i][0] + cpuinfo.current[i][1] + cpuinfo.current[i][2] + cpuinfo.current[i][3] - 
                                  cpuinfo.last[i][0] - cpuinfo.last[i][1] - cpuinfo.last[i][2] - cpuinfo.last[i][3]);
      for(j = 0;j < 4;j++)
        cpuinfo.last[i][j] = cpuinfo.current[i][j];
    }
}

