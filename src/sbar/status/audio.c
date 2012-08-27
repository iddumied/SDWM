#include <stdio.h>
void set_audio(const Arg *arg);
void toggle_mute();


typedef struct {
  int percent;
  Bool mute;
} Audio;

Audio audio;

void set_audio(const Arg* arg)
{
  int set;
  char cmd[25];
  
  set = audio.percent + arg->i;
  
  if(set > 100) set = 100;
  if(set < 0) set = 0;
  
  sprintf(cmd,"amixer set Master %d%c",set,'%');
  
  FILE *fp;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  int i, mute, clip;

  fp = popen(cmd, "r");
  if (fp == NULL){
        printf("\nfailed to read output of amixer\n");
        return;
  }

  for (i = 0; i < 5 && (read = getline(&line, &len, fp)) != -1; i++);
  
  if (read != -1) {
    i = 0;
    clip = 0;

    for (i = 0; i < read; i++) {
      
      if (line[i] == '[') {
        clip++;
        switch (clip) {
          case 1:
            audio.percent = atoi(line + i + 1);
            break;
          case 3:                      //[on]
            audio.mute = (line[i+2] == 'n') ? False : True;
            break;
        }
      }
    }
  }

  if (line) free(line);
  fclose(fp);
}


void toggle_mute()
{
  FILE *fp;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  int i, mute, clip;

  audio.mute = !audio.mute;
  
  if(audio.mute)
    fp = popen("amixer set Master mute", "r");
  else
    fp = popen("amixer set Master unmute", "r");
  
  if (fp == NULL){
        printf("\nfailed to read output of amixer\n");
        return;
  }

  for (i = 0; i < 5 && (read = getline(&line, &len, fp)) != -1; i++);
  
  if (read != -1) {
    i = 0;
    clip = 0;

    for (i = 0; i < read; i++) {
      
      if (line[i] == '[') {
        clip++;
        switch (clip) {
          case 1:
            audio.percent = atoi(line + i + 1);
            break;
          case 3:                      //[on]
            audio.mute = (line[i+2] == 'n') ? False : True;
            break;
        }
      }
    }
  }

  if (line) free(line);
  fclose(fp);
}


void setup_audio()
{
  FILE *fp;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  int i, mute, clip;

  fp = popen("amixer | grep Master -A 4", "r");
  if (fp == NULL){
        printf("\nfailed to read output of amixer\n");
        return;
  }

  for (i = 0; i < 5 && (read = getline(&line, &len, fp)) != -1; i++);
  
  if (read != -1) {
    i = 0;
    clip = 0;

    for (i = 0; i < read; i++) {
      
      if (line[i] == '[') {
        clip++;
        switch (clip) {
          case 1:
            audio.percent = atoi(line + i + 1);
            break;
          case 3:                      //[on]
            audio.mute = (line[i+2] == 'n') ? False : True;
            break;
        }
      }
    }
  }

  if (line) free(line);
  fclose(fp);
}
