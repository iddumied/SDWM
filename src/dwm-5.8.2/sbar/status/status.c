/*
#include <errno.h>
#include <locale.h>
#include <stdarg.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#include <X11/xpm.h>
static const int status_refresh_on  = 1;           // refresh zyklus in seconds if battery adapter is online
static const int status_refresh_of  = 60;          // if batteri is discharging
*/

#include <pthread.h>

pthread_mutex_t mutex;

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
//# void *update_status();
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
//    update_audio();

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
  // initializing
  ncpus = get_ncpus();
  unsigned int i, r, g;

  // reserving memory
  cpuloads = (double*)malloc((sizeof(double)*ncpus));
  last    = (unsigned long**)malloc((sizeof(unsigned long*)*ncpus));
  current = (unsigned long**)malloc((sizeof(unsigned long*)*ncpus));
  for(i = 0;i < ncpus;i++){
    last[i] = (unsigned long*)malloc((sizeof(unsigned long)*ncpus));
    current[i] = (unsigned long*)malloc((sizeof(unsigned long)*ncpus));
  }

  setup_cpu();
  
  statusstyle.cmem_used   = getcolor(memused_col);
  statusstyle.cmem_buffer = getcolor(membuff_col);
  statusstyle.cmem_cached = getcolor(memcach_col);
  statusstyle.rl          = getcolor("#010000");
  statusstyle.gl          = getcolor("#000100");

  
  if(!simply_style){
    // creating TBar pixmaps
    XGCValues gcv;
    GC gc = XCreateGC(dpy, root, 0, NULL);

    statusstyle.cpu_drawable = XCreatePixmap(dpy, root, cpu_length-1, (bh-distance_y*2 - 2), DefaultDepth(dpy, screen));
    for(i = 0;i < cpu_length-1;i++){
          gcv.foreground = (int)(((double)i / (double)cpu_length) * 255) * statusstyle.rl + 
            (255 - (int)(((double)i / (double)cpu_length) * 255)) * statusstyle.gl;

          XChangeGC(dpy, gc, GCForeground, &gcv);
          XDrawLine(dpy, statusstyle.cpu_drawable, gc, i, 0, i, (bh-distance_y*2 - 2));
    }

    XFreeGC(dpy, gc);
  }
}
