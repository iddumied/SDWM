void get_memory();

typedef struct {
  int total, free, buffer, cached, used, swaptotal, swapfree, swapused;
  double pfree, pbuffer, pcached, pused,pswapfree, pswapused;
} Memory;

static Memory memory;

void get_memory()
{
  // initializing
  FILE * fp;
  char * line = NULL;
  size_t len = 0;
  ssize_t read;
  int i, nline = 0;
  
  // open /proc/stat
  fp = fopen("/proc/meminfo", "r");
  if (fp == NULL){
      printf("\nfailed to read /proc/meminfo\n");
      return;
  }

  
  // reading line by line
  while ((read = getline(&line, &len, fp)) != -1) {
    // if line == design capacity
    switch(nline){ 
      case 0: // MemTotal:
        for(i = 9;i < len;i++){
          if(line[i] != ' '){
            pthread_mutex_lock (&mutex);
            memory.total = atoi(line+i);
            pthread_mutex_unlock (&mutex);
            break;
          }
        }
        break;
	
      case 1: // MemFree:
        for(i = 8;i < len;i++){
          if(line[i] != ' '){
            pthread_mutex_lock (&mutex);
            memory.free = atoi(line+i);
            pthread_mutex_unlock (&mutex);
            break;
          }
        }
        break;
	
      case 2: // Buffers:
        for(i = 8;i < len;i++){
          if(line[i] != ' '){
            pthread_mutex_lock (&mutex);
            memory.buffer = atoi(line+i);
            pthread_mutex_unlock (&mutex);
            break;
          }
        }
        break;
	
      case 3: // Cached:
        for(i = 7;i < len;i++){
          if(line[i] != ' '){
            pthread_mutex_lock (&mutex);
            memory.cached = atoi(line+i);
            pthread_mutex_unlock (&mutex);
            break;
          }
        }
        break;
	
      case 13: // SwapTotal:
        for(i = 10;i < len;i++){
          if(line[i] != ' '){
            pthread_mutex_lock (&mutex);
            memory.swaptotal = atoi(line+i);
            pthread_mutex_unlock (&mutex);
            break;
          }
        }
        break;
	
      case 14: // SwapFree:
        for(i = 9;i < len;i++){
          if(line[i] != ' '){
            pthread_mutex_lock (&mutex);
            memory.swapfree = atoi(line+i);
            pthread_mutex_unlock (&mutex);
            break;
          }
        }
        break;
	
      case 15: // Finish
        if (line) free(line);
        fclose(fp);
        // calcualting
        pthread_mutex_lock (&mutex);
        memory.used      = memory.total - memory.free - memory.buffer - memory.cached;
        memory.swapused  = memory.swaptotal - memory.swapfree;
        memory.pfree     = (double)memory.free     / (double)memory.total;
        memory.pbuffer   = (double)memory.buffer   / (double)memory.total;
        memory.pcached   = (double)memory.cached   / (double)memory.total;
        memory.pused     = (double)memory.used     / (double)memory.total;
        memory.pswapfree = (double)memory.swapfree / (double)memory.swaptotal;
        memory.pswapused = (double)memory.swapused / (double)memory.swaptotal;
        pthread_mutex_unlock (&mutex);
        
        return;   
    }
    nline++;
  }
}
