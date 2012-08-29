void update_uptime();
void get_uptime();
void setup_uptime();


typedef struct {
  char uptime[35], since[35], symbol;
  int s,m,h,d, len, total_seconds;
} Uptime;

static Uptime uptime;

void setup_uptime()
{
  update_uptime();
  
  time_t t = time(NULL);
  char buffer[30];
  int sec_total = 0;

  sec_total += uptime.d*60*60*24;
  sec_total += uptime.h*60*60;
  sec_total += uptime.m*60;
  sec_total += uptime.s;  
  
  t -= (time_t)sec_total;
  struct tm *ts = localtime(&t);
  

  strftime(buffer, 80, "%d.%m. %Y - %H:%M:%S", ts);

  
  sprintf(uptime.since, "%s %s", weekdays[ts->tm_wday], buffer);
}


void update_uptime()
{
  get_uptime();

  if (uptime.total_seconds < uptime_good)
    uptime.symbol = 'ü';
  else if (uptime.total_seconds < uptime_lazy)
    uptime.symbol = 'Ü';
  else if (uptime.total_seconds < uptime_sad)
    uptime.symbol = 'û';
  else if (uptime.total_seconds < uptime_angry)
    uptime.symbol = 'ú';
  else if (uptime.total_seconds < uptime_berserk)
    uptime.symbol = 'ù';
  else
    uptime.symbol = 'é';
  
  
  if(utime_seconds){
    if(uptime.d > 0)
      sprintf(uptime.uptime, "ü Ü û ú ù é %02d:%02d:%02d:%02d", uptime.d, uptime.h, uptime.m, uptime.s);
    else if(uptime.h > 0)
       sprintf(uptime.uptime, "ü Ü û ú ù é %02d:%02d:%02d", uptime.h, uptime.m, uptime.s);
    else if(uptime.m > 0)
       sprintf(uptime.uptime, "ü Ü û ú ù é %02d:%02d", uptime.m, uptime.s);
    else if(uptime.s > 0)
       sprintf(uptime.uptime, "ü Ü û ú ù é %02d", uptime.s);
    
  }else{
    if(uptime.d > 0)
      sprintf(uptime.uptime, "ü Ü û ú ù é %02d:%02d:%02d", uptime.d, uptime.h, uptime.m);
    else if(uptime.h > 0)
       sprintf(uptime.uptime, "ü Ü û ú ù é %02d:%02d", uptime.h, uptime.m);
    else if(uptime.m > 0)
       sprintf(uptime.uptime, "ü Ü û ú ù é %02d", uptime.m);
  }

  printf("[DEBUG] %x %x %x\n", (int *)uptime.uptime, ((int *)uptime.uptime) + 1, ((int *)uptime.uptime) + 2); 

  uptime.len =  strlen(uptime.uptime);
}

void get_uptime()
{
  // initializing
  FILE * fp;
  char * line = NULL;
  size_t len = 0;
  ssize_t read;
  int i, j, total_seconds;
  
  // open /proc/stat
  fp = fopen("/proc/uptime", "r");
  if (fp == NULL){
      log_str("failed to read /proc/uptime", LOG_WARNING);
      sbar_status_symbols[DrawUptime].active = False;
      return;
  }
  
  // reading line by line
  if((read = getline(&line, &len, fp)) != -1){
    total_seconds = atoi(line);
    
    uptime.total_seconds = total_seconds;
    uptime.s = total_seconds%60;
    uptime.m = (total_seconds/60)%60;
    uptime.h = (total_seconds/60/60)%24;
    uptime.d = (total_seconds/60/60/24);
    
  }

  if (line) free(line);
  fclose(fp);
}
