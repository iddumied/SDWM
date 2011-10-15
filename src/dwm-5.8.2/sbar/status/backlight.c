void update_backlight();
void get_backlight_max();
void get_backlight_cur();
void set_backlight(const Arg *arg);
void get_backlight_stat();
void toggle_backlight();

typedef struct {
  int max, cur;
  double per;
  Bool on;
} Backlight;

Backlight backlight;

void toggle_backlight()
{
  Bool on;
  Arg sarg;  
  
  get_backlight_stat();

  pthread_mutex_lock (&mutex);
  on = !backlight.on;
  backlight.on = on;
  pthread_mutex_unlock (&mutex);

  if(on)
    sarg.v = (const char*[]){ "/bin/sh", "-c",  "echo 1 > /proc/easy_backlight", NULL };
  else
    sarg.v = (const char*[]){ "/bin/sh", "-c",  "echo 0 > /proc/easy_backlight", NULL };

  spawn(&sarg);
}

void get_backlight_stat()
{
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
  
    // read actual_brightness
    fp = fopen("/proc/easy_backlight", "r");
    if (fp == NULL){
         printf("\nfailed to read /proc/easy_backlight\n");
         sbar_status_symbols[DrawBacklight].active = False;
         return;
    }

    while ((read = getline(&line, &len, fp)) != -1) {
      pthread_mutex_lock (&mutex);
      if(line[0] == '1') backlight.on = True;
      else backlight.on = False;
      pthread_mutex_unlock (&mutex);
      break;
    }
    
    if (line) free(line);
    fclose(fp);
}


void set_backlight(const Arg *arg)
{
  int max, cur, set;
  
  pthread_mutex_lock (&mutex);
  max = backlight.max;
  cur = backlight.cur;
  pthread_mutex_unlock (&mutex);
  
  set = cur + arg->i;
  if(set >= 0 && set <= max){
    Arg sarg;

    if(set > cur){
      sarg.v = (const char*[]){ "/bin/sh", "-c",  "xbacklight -inc 13", NULL };
    }else{
      sarg.v = (const char*[]){ "/bin/sh", "-c",  "xbacklight -dec 1", NULL };
    }

    spawn(&sarg);
  }
}


void update_backlight()
{
  get_backlight_max();
  get_backlight_cur();
  
  pthread_mutex_lock (&mutex);
  backlight.per = ((double)backlight.cur / (double)backlight.max);
  pthread_mutex_unlock (&mutex);
}

void get_backlight_cur()
{
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
  
    // read actual_brightness
    fp = fopen("/sys/class/backlight/samsung/actual_brightness", "r");
    if (fp == NULL){
         printf("\nfailed to read /sys/class/backlight/samsung/actual_brightness\n");
         sbar_status_symbols[DrawBacklight].active = False;
         return;
    }

    while ((read = getline(&line, &len, fp)) != -1) {
      pthread_mutex_lock (&mutex);
      backlight.cur = atoi(line);
      pthread_mutex_unlock (&mutex);
      break;
    }
    
    if (line) free(line);
    fclose(fp);
}

void get_backlight_max()
{
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    // read max_brightness
    fp = fopen("/sys/class/backlight/samsung/max_brightness", "r");
    if (fp == NULL){
         printf("\nfailed to read /sys/class/backlight/samsung/max_brightness\n");
         sbar_status_symbols[DrawBacklight].active = False;
         return;
    }

    while ((read = getline(&line, &len, fp)) != -1) {
      pthread_mutex_lock (&mutex);
      backlight.max = atoi(line);
      pthread_mutex_unlock (&mutex);
      break;
    }
    
    if (line) free(line);
    line = NULL;
    fclose(fp);
}
