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

//# void *update_status()
void update_status()
{
  //# while(True){  
    //# sleep(status_refresh);
    
    get_memory();
    check_battery();
    cpu_usage();
    update_date();
    update_uptime();
    get_thermal();
    update_backlight();
//    update_audio();
    update_net();


    XExposeEvent ev;

    ev.type = Expose;
    ev.display = dpy;
    ev.window = selmon->barwin;
  	ev.x = 0;
  	ev.y = 0;
  	ev.width = 1366;
  	ev.height = bh;
    ev.count = 0;    

/*  	if(XSendEvent(dpy, selmon->barwin, False, NoEventMask, (XEvent *)&ev) != 0){

      pthread_mutex_lock (&mutex);
      XFlush(dpy);
      pthread_mutex_unlock (&mutex);
    }
    else
      printf("\nfaile");
*/

  //# }
}

/*

int main()
{

  setup_status();
  
  printf("\n");
  
  while(True){
    sleep(1);

    
    char *batmod;
    
    if(pthread_mutex_trylock (&mutex) != EBUSY){ // locking mutex
        
      system("clear");
      
      switch(battery.mode){
        case DISCHARGING:
          batmod = "discharging";
          break;
        case CHARGING:
          batmod = "charging";
          break;
        case CHARGED:
          batmod = "charged";
          break;
      }
      
      printf("\nBattery:\n Adapter: %s\n capacity: %d mAh\n mode: %s\n rate: %d mA\n remain: %d mAh\n ramain time: %02d:%02d\n\nCPU:\n", 
      battery.adapter ? "onLine" : "offLine", battery.capacity, batmod, battery.rate, battery.current, battery.remain.h, battery.remain.m);
      
      int i = 0;
      for(;i < ncpus;i++)
        printf(" cpu%d: %f  \n", i, cpuloads[i]);    
      
      printf("\nMemory:\n total: %d\n used: %d\n buffer: %d\n chached: %d\n free: %d\n swap total: %d\n swap free: %d\n", memory.total, memory.used, memory.buffer, 
              memory.cached, memory.free, memory.swaptotal, memory.swapfree);
    
      printf("\nMemory:\n used: %f\n buffer: %f\n chached: %f\n free: %f\n swap free: %f\n swap used: %f\n",memory.pused, memory.pbuffer, memory.pcached, memory.pfree, 
              memory.pswapfree, memory.pswapused);
      
      pthread_mutex_unlock (&mutex);
    }
  }
}
*/
void setup_status()
{
  // setup uptime
  setup_uptime();
  
  // setup net
  setup_net();
  
  // setup audio
  setup_audio();
  
  // setup backlight
  backlight.on = True;

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
  
  // creating refresh Thread and init mutex
  //# pthread_t refresh;
  //# pthread_mutex_init (&mutex, NULL);
  
  // start thread
  //# pthread_create (&refresh, NULL, update_status, NULL);

  // get color
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
