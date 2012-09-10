#include "utils.c"
#include "cpu.c"
#include "battery.c"
#include "mem.c"
#include "date.c"
#include "uptime.c"
#include "thermal.c"
#include "backlight.c"
#include "audio.c"
#include "net.c"
#include "disk.c"

// my netbook NF310 Fn-keys
#define XFnToggleMute 0x1008ff12
#define XFnAudioUp 0x1008ff13
#define XFnAudioDown 0x1008ff11
#define XfnToggleDisplay 0x1008ff41
#define XfnBacklightUp 0x1008FF02
#define XfnBacklightDown 0x1008FF03
#define XfnWlan 0x1008ff95

void setup_status();
void update_status();

void update_status()
{
    #ifdef DEBUG
    char buffer[256];
    sprintf(buffer, "Update Status: draw Memory %s", sbar_status_symbols[DrawMemory].active ? "yes" : "no");
    log_str(buffer, LOG_DEBUG);
    sprintf(buffer, "Update Status: draw Battery %s", sbar_status_symbols[DrawBattery].active ? "yes" : "no");
    log_str(buffer, LOG_DEBUG);
    sprintf(buffer, "Update Status: draw Uptime %s", sbar_status_symbols[DrawUptime].active ? "yes" : "no");
    log_str(buffer, LOG_DEBUG);
    sprintf(buffer, "Update Status: draw Termal %s", sbar_status_symbols[DrawTermal].active ? "yes" : "no");
    log_str(buffer, LOG_DEBUG);
    sprintf(buffer, "Update Status: draw Backlight %s", sbar_status_symbols[DrawBacklight].active ? "yes" : "no");
    log_str(buffer, LOG_DEBUG);
    sprintf(buffer, "Update Status: draw Net %s", sbar_status_symbols[DrawNet].active ? "yes" : "no");
    log_str(buffer, LOG_DEBUG);
    #endif
    
    if(sbar_status_symbols[DrawMemory].active) {
      get_memory();
      
      #ifdef DEBUG
      log_str("Sucessfully Updated Memory", LOG_DEBUG);
      #endif
    }
    
    if(sbar_status_symbols[DrawBattery].active) {
      check_battery();

      #ifdef DEBUG
      log_str("Sucessfully Updated Battery", LOG_DEBUG);
      #endif
    }

    cpu_usage();

    #ifdef DEBUG
    log_str("Sucessfully Updated CPU", LOG_DEBUG);
    #endif

    update_date();

    #ifdef DEBUG
    log_str("Sucessfully Updated Date", LOG_DEBUG);
    #endif

    if(sbar_status_symbols[DrawUptime].active) {
      update_uptime();

      #ifdef DEBUG
      log_str("Sucessfully Updated Uptime", LOG_DEBUG);
      #endif
    }

    if(sbar_status_symbols[DrawTermal].active) {
      get_thermal();

      #ifdef DEBUG
      log_str("Sucessfully Updated Termal", LOG_DEBUG);
      #endif
    }

    if(sbar_status_symbols[DrawBacklight].active) {
      update_backlight();

      #ifdef DEBUG
      log_str("Sucessfully Updated Backlight", LOG_DEBUG);
      #endif
    }

    if(sbar_status_symbols[DrawNet].active) {
      update_net();

      #ifdef DEBUG
      log_str("Sucessfully Updated Net", LOG_DEBUG);
      #endif
    }

    update_disk();

    #ifdef DEBUG
    log_str("Sucessfully Updated Disk", LOG_DEBUG);
    #endif

}

void setup_status()
{
  #ifdef INFO
  char buffer[256];
  sprintf(buffer, "Setup Status: draw Memory %s", sbar_status_symbols[DrawMemory].active ? "yes" : "no");
  log_str(buffer, LOG_DEBUG);
  sprintf(buffer, "Setup Status: draw Battery %s", sbar_status_symbols[DrawBattery].active ? "yes" : "no");
  log_str(buffer, LOG_DEBUG);
  sprintf(buffer, "Setup Status: draw Uptime %s", sbar_status_symbols[DrawUptime].active ? "yes" : "no");
  log_str(buffer, LOG_DEBUG);
  sprintf(buffer, "Setup Status: draw Termal %s", sbar_status_symbols[DrawTermal].active ? "yes" : "no");
  log_str(buffer, LOG_DEBUG);
  sprintf(buffer, "Setup Status: draw Backlight %s", sbar_status_symbols[DrawBacklight].active ? "yes" : "no");
  log_str(buffer, LOG_DEBUG);
  sprintf(buffer, "Setup Status: draw Net %s", sbar_status_symbols[DrawNet].active ? "yes" : "no");
  log_str(buffer, LOG_DEBUG);
  #endif

  // setup disks
  setup_disk();  

  #ifdef INFO
  log_str("Setup Status Disk", LOG_INFO);
  #endif

  // setup uptime
  if(sbar_status_symbols[DrawUptime].active) {
    setup_uptime();
    
    #ifdef INFO
    log_str("Setup Status Uptime", LOG_INFO);
    #endif
  }
  
  // setup net
  if(sbar_status_symbols[DrawNet].active) {
    setup_net();

    #ifdef INFO
    log_str("Setup Status Net", LOG_INFO);
    #endif
  }
  
  // setup audio
  if(sbar_status_symbols[DrawAudio].active) {
    setup_audio();

    #ifdef INFO
    log_str("Setup Status Audio", LOG_INFO);
    #endif
  }
  
  // setup backlight
  if(sbar_status_symbols[DrawBacklight].active) {
    backlight.on = True;

    #ifdef INFO
    log_str("Setup Status Backlight", LOG_INFO);
    #endif
  }

  // setup cpu
  setup_cpu();

  #ifdef INFO
  log_str("Setup Status CPU", LOG_INFO);
  #endif
}
