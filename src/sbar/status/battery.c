#define CHARGING 0
#define DISCHARGING 1
#define CHARGED 2
#include <stdlib.h>
#include <stdio.h>

void check_battery();
void check_adapter();
void get_capacity();
void check_stat();

typedef struct {
  int h;
  int m;
} Remain;

typedef struct {
  Bool adapter; // True means online
  double stat;
  Remain remain;
  int mode, capacity, rate, current;
} Battery;

static Battery battery;

void check_battery()
{
  #ifdef DEBUG
  log_str("Update Battery", LOG_DEBUG);
  #endif
  
  check_adapter();
  
  #ifdef DEBUG
  log_str("Sucessfully Update Battery Adapter", LOG_DEBUG);
  #endif
  
  get_capacity();
  
  #ifdef DEBUG
  log_str("Sucessfully Update Battery Capacity", LOG_DEBUG);
  #endif
  
  check_stat();

  #ifdef DEBUG
  log_str("Sucessfully Update Battery Stat", LOG_DEBUG);
  #endif
}

void check_adapter()
{
  #ifdef DEBUG
  log_str("Battery Check Status: [BCS]", LOG_DEBUG);
  #endif

  // initializing
  FILE * fp;
  char * line = NULL;
  size_t len = 0;
  ssize_t read;
  int i;

  fp = fopen("/sys/class/power_supply/ADP1/online", "r");
  if (fp == NULL){
      log_str("failed to read /sys/class/power_supply/ADP1/online", LOG_WARNING);
  }else{
    
    #ifdef DEBUG
    log_str("[BCS] Sucessfuly opend /sys/class/power_supply/ADP1/online", LOG_DEBUG);
    #endif
    
    if((read = getline(&line, &len, fp)) != -1)
      battery.adapter = line[0] == '1' ? True : False;
    
    if(line) free(line);
    fclose(fp);

    #ifdef DEBUG
    log_str("[BCS] Sucesfully readed /sys/class/power_supply/ADP1/online", LOG_DEBUG);
    #endif

    return;
  }
  
  // open /proc/stat
  fp = fopen("/proc/acpi/ac_adapter/ADP1/state", "r");
  if (fp == NULL){
      log_str("failed to read /proc/acpi/ac_adapter/ADP1/state", LOG_WARNING);
      sbar_status_symbols[DrawBattery].active = False;
      return;
  }
  #ifdef DEBUG
  log_str("[BCS] Sucesfully opend /proc/acpi/ac_adapter/ADP1/state", LOG_DEBUG);
  #endif
  
  // reading line by line
  while ((read = getline(&line, &len, fp)) != -1) {
    for(i = 0;i < len;i++){
      // chech if adapter is online
      if(line[i] == 'o'){
        battery.adapter = line[i+1] == 'n' ? True : False;
        break;
      }
    }
    break;
  }

  #ifdef DEBUG
  log_str("[BCS] Sucesfully readed /proc/acpi/ac_adapter/ADP1/state", LOG_DEBUG);
  #endif

  if (line) free(line);
  fclose(fp);

  #ifdef DEBUG
  log_str("Sucessfully finished [BCS]", LOG_DEBUG);
  #endif
}

void get_capacity()
{
  #ifdef DEBUG
  log_str("Update Battery Capacity: [UBC]", LOG_DEBUG);
  #endif


  // initializing
  FILE * fp;
  char * line = NULL;
  size_t len = 0;
  ssize_t read;
  int i;
  
  fp = fopen("/sys/class/power_supply/BAT1/charge_full", "r");
  if (fp == NULL){
      log_str("failed to read /sys/class/power_supply/BAT1/charge_full", LOG_WARNING);
  }else{
    
    #ifdef DEBUG
    log_str("[UBC] Sucessfully opend /sys/class/power_supply/BAT1/charge_full", LOG_DEBUG);
    #endif

    if((read = getline(&line, &len, fp)) != -1)
      battery.capacity = atoi(line);
    
    if(line) free(line);
    fclose(fp);

    #ifdef DEBUG
    log_str("[UBC] Sucessfully readed /sys/class/power_supply/BAT1/charge_full", LOG_DEBUG);
    #endif
    
    return;
  }

  // try proc file


  // open /proc/stat
  fp = fopen("/proc/acpi/battery/BAT1/info", "r");
  if (fp == NULL){
      log_str("failed to read /proc/acpi/battery/BAT1/info", LOG_WARNING);
      sbar_status_symbols[DrawBattery].active = False;
      return;
  }
    
  #ifdef DEBUG
  log_str("[UBC] Sucessfully opend /proc/acpi/battery/BAT1/info", LOG_DEBUG);
  #endif
  
  // reading line by line
  while ((read = getline(&line, &len, fp)) != -1) {
    // if line == design capacity
    if(line[0] == 'd'){
      for(i = 16;i < len;i++){
        if(line[i] != ' '){
          battery.capacity = atoi(line+i);
          break;
        }
      }
      break;
    }
  }
  if (line) free(line);
  fclose(fp);
    
  #ifdef DEBUG
  log_str("[UBC] Sucessfully readed /proc/acpi/battery/BAT1/info", LOG_DEBUG);
  #endif
}

void check_stat()
{

  #ifdef DEBUG
  log_str("Update Battery Stat: [UBS]", LOG_DEBUG);
  #endif

  // initializing
  FILE * fp;
  char * line = NULL;
  size_t len = 0;
  ssize_t read;
  int i;



  fp = fopen("/sys/class/power_supply/BAT1/status", "r");
  if (fp == NULL){
    log_str("failed to open /sys/class/power_supply/BAT1/status", LOG_WARNING);
  }else{

    #ifdef DEBUG
    log_str("[UBS] Sucessfully opend /sys/class/power_supply/BAT1/status", LOG_DEBUG);
    #endif

    if((read = getline(&line, &len, fp)) != -1){
      if(line[0] == 'D') battery.mode = DISCHARGING;
      else if(line[0] == 'C') battery.mode = CHARGING;
      else battery.mode = CHARGED;
    }

    #ifdef DEBUG
    log_str("[UBS] Sucessfully readed /sys/class/power_supply/BAT1/status", LOG_DEBUG);
    #endif

    fclose(fp);
    
    fp = fopen("/sys/class/power_supply/BAT1/charge_now", "r");
    if (fp == NULL){
      log_str("failed to open /sys/class/power_supply/BAT1/charge_now", LOG_WARNING);
    }else{

      #ifdef DEBUG
      log_str("[UBS] Sucessfully opend /sys/class/power_supply/BAT1/charge_now", LOG_DEBUG);
      #endif

      if((read = getline(&line, &len, fp)) != -1)
        battery.current = atoi(line);

      #ifdef DEBUG
      log_str("[UBS] Sucessfully readed /sys/class/power_supply/BAT1/charge_now", LOG_DEBUG);
      #endif

      fclose(fp);

      fp = fopen("/sys/class/power_supply/BAT1/current_now", "r");
      if (fp == NULL){
        log_str("failed to open /sys/class/power_supply/BAT1/current_now", LOG_WARNING);
      }else{

        #ifdef DEBUG
        log_str("[UBS] Sucessfully opend /sys/class/power_supply/BAT1/current_now", LOG_DEBUG);
        #endif

        if((read = getline(&line, &len, fp)) != -1)
          battery.rate = atoi(line);

        // avoid divide with zero
        if (battery.rate == 0) battery.mode = CHARGED;

        #ifdef DEBUG
        log_str("[UBS] Sucessfully readed /sys/class/power_supply/BAT1/current_now", LOG_DEBUG);
        char logbuf[256];
        sprintf(logbuf, "Battery current %d, rate %d, capacity %d remain hours %d", battery.current, battery.rate, battery.capacity, battery.remain.h);
        log_str(logbuf, LOG_DEBUG);
        #endif

        if(battery.mode == DISCHARGING){
          battery.remain.h = battery.current / battery.rate;
          battery.remain.m = (int)(((double)battery.current / 
                              (double)battery.rate - battery.remain.h) * 60);
          battery.stat = (double)battery.current / (double)battery.capacity;
        }else if(battery.mode == CHARGING){
          battery.remain.h = (battery.capacity - battery.current) / battery.rate;
          battery.remain.m = (int)(((double)(battery.capacity - battery.current) / 
                              (double)battery.rate - battery.remain.h) * 60);
          battery.stat = (double)battery.current / (double)battery.capacity;
        }else{ //charged
          battery.stat = 1.0;
          battery.remain.h = 0;
          battery.remain.m = 0;
        }
        #ifdef DEBUG
        log_str("[UBS] Sucessfully updated changes from /sys/class/power_supply/BAT1/current_now", LOG_DEBUG);
        #endif
      
        if(line) free(line);

        #ifdef DEBUG
        log_str("[UBS] Sucessfully freed resauces from /sys/class/power_supply/BAT1/current_now", LOG_DEBUG);
        #endif

        fclose(fp);

        #ifdef DEBUG
        log_str("[UBS] Sucessfully closed /sys/class/power_supply/BAT1/current_now", LOG_DEBUG);
        #endif
        return;
      }
    }
  }

  // try proc file

  // open /proc/stat
  fp = fopen("/proc/acpi/battery/BAT1/state", "r");
  if (fp == NULL){
    log_str("failed to open/proc/acpi/battery/BAT1/state", LOG_WARNING);
    sbar_status_symbols[DrawBattery].active = False;
    return;
  }

  #ifdef DEBUG
  log_str("[UBS] Sucessfully opend /proc/acpi/battery/BAT1/state", LOG_DEBUG);
  #endif

  // reading line by line
  while ((read = getline(&line, &len, fp)) != -1) {
    if(line[0] == 'p' && line[7] == ':') continue;
    else if(line[0] == 'c' && line[1] == 'a') continue;
    else if(line[0] == 'c' && line[1] == 'h'){ // if line == charging state:
      for(i = 15;i < len;i++){
        if(line[i] != ' '){
          if(line[i] == 'd') battery.mode = DISCHARGING;
          else if(line[i+6] == 'd') battery.mode = CHARGED;
          else if(line[i+6] == 'n') battery.mode = CHARGING;
          break;
        }
      }
    }else if(line[0] == 'p' && line[8] == 'r'){ //if line = present rate: 
      for(i = 13;i < len;i++){
        if(line[i] != ' '){
          battery.rate = atoi(line+i);
          break;
        }
      }
    }else if(line[0] == 'r'){ // if line = remaining capacity:
      for(i = 19;i < len;i++){
        if(line[i] != ' '){
          battery.current = atoi(line+i);
          break;
        }
      }
    }else break;
  }
  
  if(battery.mode == DISCHARGING){
    battery.remain.h = battery.current / battery.rate;
    battery.remain.m = (int)(((double)battery.current / 
        (double)battery.rate - battery.remain.h) * 60);
    battery.stat = (double)battery.current / (double)battery.capacity;
  }else if(battery.mode == CHARGING){
    battery.remain.h = (battery.capacity - battery.current) / battery.rate;
    battery.remain.m = (int)(((double)(battery.capacity - battery.current) / 
        (double)battery.rate - battery.remain.h) * 60);
    battery.stat = (double)battery.current / (double)battery.capacity;
  }else{ //charged
    battery.stat = 1.0;
    battery.remain.h = 0;
    battery.remain.m = 0;
  }
  
  if (line) free(line);
  fclose(fp);

  #ifdef DEBUG
  log_str("[UBS] Sucessfully readed /proc/acpi/battery/BAT1/state", LOG_DEBUG);
  #endif
}

