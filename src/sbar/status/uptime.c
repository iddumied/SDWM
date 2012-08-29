void update_uptime();
void get_uptime();
void setup_uptime();


typedef struct {
  char uptime[35], since[35];
  int s,m,h,d, len;
} TBarUptime;

static TBarUptime tbar_uptime;

void setup_uptime()
{
  update_uptime();
  
  time_t t = time(NULL);
  char buffer[30];
  int sec_total = 0;

  sec_total += tbar_uptime.d*60*60*24;
  sec_total += tbar_uptime.h*60*60;
  sec_total += tbar_uptime.m*60;
  sec_total += tbar_uptime.s;  
  
  t -= (time_t)sec_total;
  struct tm *ts = localtime(&t);
  

  strftime(buffer, 80, "%d.%m. %Y - %H:%M:%S", ts);

  
  sprintf(tbar_uptime.since, "%s %s", weekdays[ts->tm_wday], buffer);
}


void update_uptime()
{
  get_uptime();
  
  if(utime_seconds){
    if(tbar_uptime.d > 0)
      sprintf(tbar_uptime.uptime, "\x0a %02d:%02d:%02d:%02d", tbar_uptime.d, tbar_uptime.h, tbar_uptime.m, tbar_uptime.s);
    else if(tbar_uptime.h > 0)
       sprintf(tbar_uptime.uptime, "\x0a %02d:%02d:%02d", tbar_uptime.h, tbar_uptime.m, tbar_uptime.s);
    else if(tbar_uptime.m > 0)
       sprintf(tbar_uptime.uptime, "\x0a %02d:%02d", tbar_uptime.m, tbar_uptime.s);
    else if(tbar_uptime.s > 0)
       sprintf(tbar_uptime.uptime, "\x0a %02d", tbar_uptime.s);
    
  }else{
    if(tbar_uptime.d > 0)
      sprintf(tbar_uptime.uptime, "\x0a %02d:%02d:%02d", tbar_uptime.d, tbar_uptime.h, tbar_uptime.m);
    else if(tbar_uptime.h > 0)
       sprintf(tbar_uptime.uptime, "\x0a %02d:%02d", tbar_uptime.h, tbar_uptime.m);
    else if(tbar_uptime.m > 0)
       sprintf(tbar_uptime.uptime, "\x0a %02d", tbar_uptime.m);
  }

  tbar_uptime.len =  strlen(tbar_uptime.uptime);
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
    
    tbar_uptime.s = total_seconds%60;
    tbar_uptime.m = (total_seconds/60)%60;
    tbar_uptime.h = (total_seconds/60/60)%24;
    tbar_uptime.d = (total_seconds/60/60/24);
    
  }

  if (line) free(line);
  fclose(fp);
}
