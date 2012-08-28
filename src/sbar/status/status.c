#include <pthread.h>

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


#define XFnToggleMute 0x1008ff12
#define XFnAudioUp 0x1008ff13
#define XFnAudioDown 0x1008ff11
#define XfnToggleDisplay 0x1008ff41
#define XfnBacklightUp 0x1008FF02
#define XfnBacklightDown 0x1008FF03
#define XfnWlan 0x1008ff95



void setup_status();
void update_status();


typedef struct {
  Pixmap cpu_drawable;
  unsigned long cmem_used, cmem_buffer, cmem_cached, rl, gl;
  unsigned long cpu_line, cpu_point;
} StatusStyle;

static StatusStyle statusstyle;

void update_status()
{
    
    if(sbar_status_symbols[DrawMemory].active)
      get_memory();
    
    if(sbar_status_symbols[DrawBattery].active)
      check_battery();

    cpu_usage();
    update_date();

    if(sbar_status_symbols[DrawUptime].active)
      update_uptime();

    if(sbar_status_symbols[DrawTermal].active)
      get_thermal();

    if(sbar_status_symbols[DrawBacklight].active)
      update_backlight();

    if(sbar_status_symbols[DrawNet].active)
      update_net();

    update_disk();

}

void setup_status()
{
  // setup disks
  setup_disk();  

  // setup uptime
  if(sbar_status_symbols[DrawUptime].active)
    setup_uptime();
  
  // setup net
  if(sbar_status_symbols[DrawNet].active)
    setup_net();
  
  // setup audio
  if(sbar_status_symbols[DrawAudio].active)
    setup_audio();
  
  // setup backlight
  if(sbar_status_symbols[DrawBacklight].active)
    backlight.on = True;

  // setup cpu
  setup_cpu();
}
