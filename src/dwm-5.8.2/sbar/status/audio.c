#include <stdio.h>
void *update_audio();
void set_audio(const Arg *arg);
void toggle_mute();


typedef struct {
  int max, cur, realper;
  double percent;
  Bool mute, headphones;
} Audio;

Audio audio;

void set_audio(const Arg* arg)
{
  double per;
  int set;
  char cmd[25];
  
  pthread_mutex_lock (&mutex);
  per = audio.realper;
  pthread_mutex_unlock (&mutex);
  
  set = per + arg->i;
  
  if(set > 100) set = 100;
  if(set < 0) set = 0;
  
  printf("\namixer set Master %d%c\n",set,'%');
  
  sprintf(cmd,"amixer set Master %d%c",set,'%');
  
  Arg sarg;
  sarg.v = (const char*[]){ "/bin/sh", "-c",  cmd, NULL };

  spawn(&sarg);
  
  pthread_mutex_lock (&mutex);
  audio.realper = set;
  pthread_mutex_unlock (&mutex);
  
  // creating refresh Thread and init mutex
  pthread_t refresh;
  pthread_mutex_init (&mutex, NULL);
  
  // start thread
  pthread_create (&refresh, NULL, update_audio, NULL);
}


void toggle_mute()
{
  pthread_mutex_lock (&mutex);
  Bool mute = !audio.mute;
  audio.mute = mute;
  pthread_mutex_unlock (&mutex);
  
  Arg sarg;
  
  if(mute)
    sarg.v = (const char*[]){ "/bin/sh", "-c",  "amixer set Master mute", NULL };
  else
    sarg.v = (const char*[]){ "/bin/sh", "-c",  "amixer set Master unmute", NULL };
  
  spawn(&sarg);

  
  // creating refresh Thread and init mutex
  pthread_t refresh;
  pthread_mutex_init (&mutex, NULL);
  
  // start thread
  pthread_create (&refresh, NULL, update_audio, NULL);
}


void *update_audio()
{
  sleep(1);
  
  FILE * fp;
  char * line = NULL;
  size_t len = 0;
  ssize_t read;
  int i, mute;
  i = 0;

  fp = fopen("/proc/asound/card0/codec#0", "r");
  if (fp == NULL){
        printf("\nfailed to read /proc/asound/card0/codec#0\n");
        return;
  }

  while ((read = getline(&line, &len, fp)) != -1) {
    if(i == 21){ // current stat
      pthread_mutex_lock (&mutex);
      audio.cur = xtoi(line+20,2);
      pthread_mutex_unlock (&mutex);
    }else if(i == 113){
      mute = atoi(line+20);
      if(mute > 0){
        pthread_mutex_lock (&mutex);
        audio.mute = True;
        pthread_mutex_unlock (&mutex);
      }else{
        pthread_mutex_lock (&mutex);
        audio.mute = False;
        pthread_mutex_unlock (&mutex);
      }
    }else if(i == 120){
      mute = atoi(line+14);
      if(mute > 0){
        pthread_mutex_lock (&mutex);
        audio.headphones = False;
        pthread_mutex_unlock (&mutex);
      }else{
        pthread_mutex_lock (&mutex);
        audio.headphones = True;
        pthread_mutex_unlock (&mutex);
      }
    }
    i++;
  }

  if (line) free(line);
  fclose(fp);

  pthread_mutex_lock (&mutex);
  audio.percent = (double)audio.cur / (double)audio.max;
  pthread_mutex_unlock (&mutex);
}

void setup_audio()
{
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int i = 0;


    fp = fopen("/proc/asound/card0/codec#0", "r");
    if (fp == NULL){
         printf("\nfailed to read /proc/asound/card0/codec#0\n");
         return;
    }

    while ((read = getline(&line, &len, fp)) != -1) {
      if(i == 20){
        pthread_mutex_lock (&mutex);
        audio.max = xtoi(line+35, 2);
        pthread_mutex_unlock (&mutex);
      }else if(i == 21) break;      
      i++;
    }

    if (line) free(line);
    fclose(fp);
    
    update_audio();
    
    pthread_mutex_lock (&mutex);
    audio.realper = ((int)(audio.percent*10))*10;
    pthread_mutex_unlock (&mutex);
}