/*#include <errno.h>
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
typedef struct {
  int x, y, w, h;
  unsigned long norm[ColLast];
  unsigned long sel[ColLast];
  unsigned long sbar[SBarLast];
  Drawable drawable;
  GC gc;
  struct {
    int ascent;
    int descent;
    int height;
    XFontSet set;
    XFontStruct *xfont;
  } font;
} DC;
*/


DC stw;
Pixmap background;
typedef struct {
  int *bytes, max;
} Timeline;

typedef struct {
  int length;
  Timeline lor, eth0r, wlan0r,lot, eth0t, wlan0t;
} NetStatistic;

typedef struct {
  int length;
  unsigned long long readges, writeges;
  Timeline read, write;
} DiskStatistic;

NetStatistic netstat;
DiskStatistic diskstat[MAXPARTITIONS];
void setup_stw();
void drawstw();
void calc_timline_max(Timeline *timeline, int bytes, int length);




#include "write.c"

void calc_timline_max(Timeline *timeline, int bytes, int length)
{
  int i;
  timeline->max = 0;
  for(i = 1; i < length;i++){
    if(timeline->bytes[i] > timeline->max) timeline->max = timeline->bytes[i];
  }
  if(bytes > timeline->max) timeline->max = bytes;
}

void drawstw()
{
  // clear
  XCopyArea(dpy, background, stw.drawable, stw.gc, 0, 0, stw.w, stw.h, 0, 0 );
  stwwrite.xc = stwwrite.xs;
  stwwrite.yc = stwwrite.ys;

  

  // catching values
  char stwbuffer[100], hread[12], maxhread[12];
  double battstat, light, audioper, wlanstat, *loadcpu, pused, pbuffer, pcached, pfree, pswapused, pswapfree;
  loadcpu = (double*)malloc(sizeof(double)*ncpus);
  int upd,uph,upm,ups,used, buffer, cached, swapused, swaptotal, ramtotal, i, battre_h, battre_m, battmode, swapusedper, pos, temperature, lor,lot,eth0r, eth0t, wlan0r,wlan0t, pro_total, pro_running, pro_blocked ;
  Bool audiomute, audiophones, netonline, ethonline, walnonline, wlaneasy;
  // catching information
  pthread_mutex_lock (&mutex);
  upd         = tbar_uptime.d;
  uph         = tbar_uptime.h;
  upm         = tbar_uptime.m;
  ups         = tbar_uptime.s;
  used        = memory.used;
  buffer      = memory.buffer;
  cached      = memory.cached;
  pused       = memory.pused;
  pfree       = memory.pfree;
  pbuffer     = memory.pbuffer;
  pcached     = memory.pcached;
  ramtotal    = memory.total;
  swapused    = memory.swapused;
  pswapused   = memory.pswapused;
  pswapfree   = memory.pswapfree;
  swaptotal   = memory.swaptotal;
  battstat    = battery.stat;
  battre_h    = battery.remain.h;
  battre_m    = battery.remain.m;
  temperature = thermal;
  battmode    = battery.mode;
  light       = backlight.per;
  audioper    = audio.percent;
  audiophones = audio.headphones;
  audiomute   = audio.mute;
  netonline   = net.connected;
  ethonline   = net.eth0.online;
  walnonline  = net.wlan0.online;
  wlanstat    = net.wlan0.strength;
  wlaneasy    = net.wlan0.easy_online;
  lor         = net.lo.between.receive.bytes_per_sec;
  lot         = net.lo.between.transmit.bytes_per_sec;
  eth0r       = net.eth0.between.receive.bytes_per_sec;
  eth0t       = net.eth0.between.transmit.bytes_per_sec;
  wlan0r      = net.wlan0.between.receive.bytes_per_sec;
  wlan0t      = net.wlan0.between.transmit.bytes_per_sec;
  pro_blocked = processes.blocked;
  pro_running = processes.running;
  pro_total   = processes.total;
  sprintf(stwbuffer,"Chief @ ArchLinux  -  online since:  %s", tbar_uptime.since);
  for(i = 0;i < ncpus;i++) loadcpu[i] = cpuloads[i];
  pthread_mutex_unlock (&mutex);
  
  XGCValues gcv;
  gcv.foreground = stw.sel[ColFG];
  XChangeGC(dpy, stw.gc, GCForeground, &gcv);


    
  // statusmesage
                                        wprintln("");
                                        wprintln(stwbuffer);
                                        wprintln("  |");
                                        wprintln("  +--status");
                                        wprintln("  |    |");
                                        wprintln("  |    +--cpu");
                                        wprintln("  |    |    |");
for(i = 0; i < ncpus; i++){;
                              sprintf(stwbuffer, "  |    |    +--cpu%d:  %d%c",i+1, (int)(loadcpu[i]*100), '%');
  wprintln(stwbuffer);
  //wprintcolln(loadcpu[i], 100, 0.65, 2);
} 
                                        wprintln("  |    |");
                                          wprint("  |    +--ram:  ");
double percent[3] = {pused,pbuffer,pcached}; 
unsigned long colors[3] = { stw.sel[ColFG],
  stw.sbar[SBarCpuLine], stw.sbar[SBarCpuPoint] };
wprintcolsln(&percent, &colors, 3, 100, 0.65,2);
                                        wprintln("  |    |    |");
                              sprintf(stwbuffer, "  |    |    +--used:  %d MB / %d MB  -  %d%c", used/1024, ramtotal/1024,(int)(pused*100),'%');
                              wprintln(stwbuffer);  
                              sprintf(stwbuffer, "  |    |    +--buffer:  %d MB", buffer/1024);
                              wprintln(stwbuffer);
                              sprintf(stwbuffer, "  |    |    +--cached:  %d MB", cached/1024);
                              wprintln(stwbuffer);
                                        wprintln("  |    |");
if(swapused/1024 == 0){       sprintf(stwbuffer, "  |    +--swap:  %d MB / %d MB  -  %d%c", swapused/1024, swaptotal/1024, (int)(pswapused*100),'%');
                              wprintln(stwbuffer);  
}else{                                    wprint("  |    +--swap:  ");
wprintcolln(pswapused, 100, 0.65, 2 );
                                        wprintln("  |    |    |");
                              sprintf(stwbuffer, "  |    |    +--used:  %d MB / %d MB  -  %d%c", swapused/1024, swaptotal/1024, (int)(pswapused*100),'%');
                              wprintln(stwbuffer);  
                                        wprintln("  |    |");
}
                                        wprintln("  |    +--processes  ");
                                        wprintln("  |    |    |");
                              sprintf(stwbuffer, "  |    |    +--amount:  %d", pro_total);
                              wprintln(stwbuffer);  
                              sprintf(stwbuffer, "  |    |    +--running:  %d", pro_running);
                              wprintln(stwbuffer);                                        
                              sprintf(stwbuffer, "  |    |    +--blocked:  %d", pro_blocked);
                              wprintln(stwbuffer);
                                        wprintln("  |    |");
                              sprintf(stwbuffer, "  |    +--battery:  %d%c", (int)(battstat*100), '%');
                              wprintln(stwbuffer);
                                        wprintln("  |         |");
if(battmode == CHARGING)                wprintln("  |         +--mode:  charging");
else if(battmode == DISCHARGING)        wprintln("  |         +--mode:  discharging");    
else if(battmode == CHARGED)            wprintln("  |         +--mode:  charged");    
                              sprintf(stwbuffer, "  |         +--remain:  %02d:%02d", battre_h,battre_m);
                              wprintln(stwbuffer);
                                        wprintln("  |");
                                        wprintln("  +--net");
                                        wprintln("       |");
                                          wprint("       +--lo");
calc_timline_max(&netstat.lot, lot, netstat.length);
calc_timline_max(&netstat.lor, lor, netstat.length);
if(netstat.lor.max == 0 && netstat.lot.max == 0){                                 
                                        wprintln(":  inactive");
}else{                                  wprintln("");
                                        wprintln("       |    |");
if(netstat.lot.max == 0)                wprintln("       |    +--up:  inactiv");
else{  
human_readable(lot, False, hread);
human_readable(netstat.lot.max, False, maxhread);
                              sprintf(stwbuffer, "       |    +--up:  %s  @  %s ", hread, maxhread);
                              wprintln(stwbuffer);
                                        wprintln("       |    |    |");
                                          wprint("       |    |    +--");
wprinttimelineln(lot, netstat.length, 1, 
        &netstat.lot,stw.sbar[SBarCpuLine], 
        stw.sbar[SBarCpuPoint],netstat.lot.max);
                                        wprintln("       |    |");
}
if(netstat.lor.max == 0)                wprintln("       |    +--down:  inactiv");
else{  
human_readable(lor, False, hread);
human_readable(netstat.lor.max, False, maxhread);
                              sprintf(stwbuffer, "       |    +--down:  %s  @  %s ", hread, maxhread);
                              wprintln(stwbuffer);
                                        wprintln("       |         |");
                                          wprint("       |         +--");
wprinttimelineln(lor, netstat.length, 1, 
                 &netstat.lor,stw.sbar[SBarCpuLine], 
                 stw.sbar[SBarCpuPoint],netstat.lor.max);
}
                                        wprintln("       |");
}                      
                                          wprint("       +--eth0");
calc_timline_max(&netstat.eth0r, eth0t, netstat.length);
calc_timline_max(&netstat.eth0t, eth0t, netstat.length);
if(netstat.eth0r.max == 0 && netstat.eth0t.max == 0){                                 
  if(ethonline)                         wprintln(":  inactive");  
  else                                  wprintln(":  down");
}else{ if(!ethonline)                   wprintln(":  down");
else                                    wprintln(":  connected");
                                        wprintln("       |    |");
if(netstat.eth0t.max == 0)              wprintln("       |    +--up:  inactiv");
else{
human_readable(eth0t, False, hread);
human_readable(netstat.eth0t.max, False, maxhread);
                              sprintf(stwbuffer, "       |    +--up:  %s  @  %s ", hread, maxhread);
                              wprintln(stwbuffer);
                                        wprintln("       |    |    |");
                                          wprint("       |    |    +--");
wprinttimelineln(eth0t, netstat.length, 1, 
        &netstat.eth0t,stw.sbar[SBarCpuLine], 
        stw.sbar[SBarCpuPoint],netstat.eth0t.max);
                                        wprintln("       |    |");
}
if(netstat.eth0r.max == 0)              wprintln("       |    +--down:  inactiv");
else{
human_readable(eth0r, False, hread);
human_readable(netstat.eth0r.max, False, maxhread);
                              sprintf(stwbuffer, "       |    +--down:  %s  @  %s ", hread, maxhread);
                              wprintln(stwbuffer);
                                        wprintln("       |         |");
                                          wprint("       |         +--");
wprinttimelineln(eth0r, netstat.length, 1, 
                 &netstat.eth0r,stw.sbar[SBarCpuLine], 
                 stw.sbar[SBarCpuPoint],netstat.eth0r.max);
}
                                        wprintln("       |");   
}
                                          wprint("       +--wlan0");
calc_timline_max(&netstat.wlan0t, wlan0t, netstat.length);
calc_timline_max(&netstat.wlan0r, wlan0r, netstat.length);
if(netstat.wlan0r.max == 0 && netstat.wlan0t.max == 0){                                 
  if(walnonline)                        wprintln(":  inactive");
  else if(wlaneasy)                     wprintln(":  disconnected");
  else                                  wprintln(":  down");
}else{ if(!wlaneasy)                    wprintln(":  down");
else if(!walnonline)                    wprintln(":  disconnected");
else                                    wprintln(":  connected");
                                        wprintln("            |");
if(walnonline){               sprintf(stwbuffer, "            +--signal:  %d%c",(int)(wlanstat*100),'%');
                              wprintln(stwbuffer);
}
if(netstat.wlan0t.max == 0)             wprintln("            +--up:  inactiv");
else{       
human_readable(wlan0t, False, hread);
human_readable(netstat.wlan0t.max, False, maxhread);
                              sprintf(stwbuffer, "            +--up:  %s  @  %s ", hread, maxhread);
                              wprintln(stwbuffer);
                                        wprintln("            |    |");
                                          wprint("            |    +--");
wprinttimelineln(wlan0t, netstat.length, 1, 
        &netstat.wlan0t,stw.sbar[SBarCpuLine], 
        stw.sbar[SBarCpuPoint],netstat.wlan0t.max);
                                        wprintln("            |");
}
if(netstat.wlan0r.max == 0)             wprintln("            +--down:  inactiv");
else{  
human_readable(wlan0r, False, hread);
human_readable(netstat.wlan0r.max, False, maxhread);
                              sprintf(stwbuffer, "            +--down:  %s  @  %s ", hread, maxhread);
                              wprintln(stwbuffer);
                                        wprintln("                 |");
                                          wprint("                 +--");
wprinttimelineln(wlan0r, netstat.length, 1, 
                 &netstat.wlan0r,stw.sbar[SBarCpuLine], 
                 stw.sbar[SBarCpuPoint],netstat.wlan0r.max);
}                                         




}

char ebuf[5][20];
int countdisks, mj, mi = 0;
stwwrite.xs = stwwrite.xc  = 1100;
stwwrite.ys = stwwrite.yc = (stw.font.height + gappx);
stwwrite.xe = stwwrite.w = stw.w - gappx;
stwwrite.ye = stwwrite.h = stw.h;

for(i = 0;i < MAXPARTITIONS;i++)
  if(!disks[i].active) break;

countdisks = i;

wprintln("");
sprintf(stwbuffer,"Volumes mounted:  %d  of  %d  Disks",mounted_volumes,countdisks);
wprintln(stwbuffer);
wprintln("  |");
for(i = 0; i < MAXPARTITIONS; i++){
  if(!disks[i].active) break;
  

  if(disks[i].mountpoint != NULL){
    mi++;
    sprintf(stwbuffer,"  +--%s  @  %s", disks[i].path, disks[i].mountpoint);  
    wprintln(stwbuffer);
    if(mi < mounted_volumes) wprintln("  |    |");
    else wprintln("       |");

    if(mi < mounted_volumes) wprint("  |    +--usage:  ");
    else wprint("       +--usage:  ");
    wprintcolln(disks[i].pused, 100, 0.65, 2 );
    if(mi < mounted_volumes) wprintln("  |         |");
    else wprintln("            |");

    human_readable_disk(disks[i].avil, &ebuf[0]);
    human_readable_disk(disks[i].total, &ebuf[1]);
    if(mi < mounted_volumes) sprintf(stwbuffer,"  |         +--free:  %s / %s  -  %d%c", ebuf[0], ebuf[1], (int)(disks[i].pavil*100),'%');
    else sprintf(stwbuffer,"            +--free:  %s / %s  -  %d%c", ebuf[0], ebuf[1], (int)(disks[i].pavil*100),'%');
    wprintln(stwbuffer);


    /****** RAED *****/

    // calculation inforamtions and make it readable
    diskstat[i].readges += disks[i].between.read;
    calc_timline_max(&diskstat[i].read, disks[i].between.read, diskstat[i].length);
    human_readable_disk(disks[i].now.read, &ebuf[0]);
    human_readable(disks[i].between.read, False, &ebuf[1]);
    human_readable(diskstat[i].read.max, False, &ebuf[2]);
    human_readable_disk(diskstat[i].readges, &ebuf[3]);

    if(mi < mounted_volumes){
      if(diskstat[i].read.max > 0){
        sprintf(stwbuffer,"  |         +--read:  %s @ %s", ebuf[1], ebuf[2]);
        wprintln(stwbuffer);

        wprintln("  |         |    |");
        wprint("  |         |    +--");

        // printing timeline if max > 0
        wprinttimeline(disks[i].between.read, diskstat[i].length, 1, &diskstat[i].read, 
                      stw.sbar[SBarCpuLine], stw.sbar[SBarCpuPoint], diskstat[i].read.max);
      
        sprintf(stwbuffer,"   %s",ebuf[3]);
        wprintln(stwbuffer);
        wprintln("  |         |");

      }else{
        diskstat[i].readges = 0; // clearing history
        sprintf(stwbuffer,"  |         +--readed: %s",ebuf[0]);
        wprintln(stwbuffer);
      }

    }else{
      if(diskstat[i].read.max > 0){
        sprintf(stwbuffer,"            +--read:  %s @ %s", ebuf[1], ebuf[2]);
        wprintln(stwbuffer);
  
        wprintln("            |    |");
        wprint("            |    +--");

        // printing timeline if max > 0
        wprinttimeline(disks[i].between.read, diskstat[i].length, 1, &diskstat[i].read, 
                      stw.sbar[SBarCpuLine], stw.sbar[SBarCpuPoint], diskstat[i].read.max);
        
        sprintf(stwbuffer,"   %s",ebuf[3]);
        wprintln(stwbuffer);
        wprintln("            |");

      }else{
        diskstat[i].readges = 0; // clearing history
        sprintf(stwbuffer,"            +--readed %s",ebuf[0]);
        wprintln(stwbuffer);
      }

    }


    /****** WRITE *****/

    // calculation inforamtions and make it readable
    diskstat[i].writeges += disks[i].between.write;
    calc_timline_max(&diskstat[i].write, disks[i].between.write, diskstat[i].length);
    human_readable_disk(disks[i].now.write, &ebuf[0]);
    human_readable(disks[i].between.write, False, &ebuf[1]);
    human_readable(diskstat[i].write.max, False, &ebuf[2]);
    human_readable_disk(diskstat[i].writeges, &ebuf[3]);

    if(mi < mounted_volumes){
      if(diskstat[i].write.max > 0){
        sprintf(stwbuffer,"  |         +--write:  %s @ %s", ebuf[1], ebuf[2]);
        wprintln(stwbuffer);

        wprintln("  |              |");
        wprint("  |              +--");

        // printing timeline if max > 0
        wprinttimeline(disks[i].between.write, diskstat[i].length, 1, &diskstat[i].write, 
                      stw.sbar[SBarCpuLine], stw.sbar[SBarCpuPoint], diskstat[i].write.max);
      
        sprintf(stwbuffer,"   %s",ebuf[3]);
        wprintln(stwbuffer);

      }else{
        diskstat[i].writeges = 0; // clearing history
        sprintf(stwbuffer,"  |         +--writen: %s",ebuf[0]);
        wprintln(stwbuffer);
      }

    }else{
      if(diskstat[i].write.max > 0){
        sprintf(stwbuffer,"            +--write:  %s @ %s", ebuf[1], ebuf[2]);
        wprintln(stwbuffer);
  
        wprintln("                 |");
        wprint("                 +--");

        // printing timeline if max > 0
        wprinttimeline(disks[i].between.write, diskstat[i].length, 1, &diskstat[i].write, 
                      stw.sbar[SBarCpuLine], stw.sbar[SBarCpuPoint], diskstat[i].write.max);
        
        sprintf(stwbuffer,"   %s",ebuf[3]);
        wprintln(stwbuffer);
      }else{
        diskstat[i].writeges = 0; // clearing history
        sprintf(stwbuffer,"            +--writen: %s",ebuf[0]);
        wprintln(stwbuffer);
      }

    }

    if(mi < mounted_volumes) wprintln("  |");
  }else if(diskstat[i].read.max > 0 || diskstat[i].write.max > 0){
    diskstat[i].read.max  = 0;
    diskstat[i].write.max = 0;
    diskstat[i].readges   = 0;
    diskstat[i].writeges   = 0;

    for(mj = 0;mj < diskstat[i].length;mj++){
      diskstat[i].read.bytes[mj]  = 0;
      diskstat[i].write.bytes[mj] = 0;
    }
  }
}


// set to original Coordinates
stwwrite.xs = stwwrite.xc  = gappx;
stwwrite.ys = stwwrite.yc = stw.font.height + gappx;
stwwrite.xe = stwwrite.w = stw.w - gappx;
stwwrite.ye = stwwrite.h = stw.h;

                                        
  XCopyArea(dpy, stw.drawable, selmon->statuswin, stw.gc, 0, 0, stw.w, stw.h, 0, 0 );
}



// TODO REPLEACE SBarCpuLine ua. with SBarLine
void setup_stw()
{
    int h, i, j;
  
    // initializing stw
    stw.norm[ColBorder]    = getcolor(swnormbordercolor);
    stw.norm[ColBG]        = getcolor(swnormbgcolor);
    stw.norm[ColFG]        = getcolor(swnormfgcolor);
    stw.sel[ColBorder]     = getcolor(swselbordercolor);
    stw.sel[ColBG]         = getcolor(swselbgcolor);
    stw.sel[ColFG]         = getcolor(swselfgcolor);
    stw.sbar[SBarBorder]   = getcolor(swbotbordercolor);
    stw.sbar[SBarCpuLine]  = getcolor(swcpu_line_color);
    stw.sbar[SBarCpuPoint] = getcolor(swcpu_point_color);
    
    
    stw.font.ascent  = dc.font.ascent;
    stw.font.descent = dc.font.descent;
    stw.font.height  = dc.font.height;
    stw.font.set     = dc.font.set;
    stw.font.xfont   = dc.font.xfont;
    
    stw.w = stwwidth;
    stw.y = dc.font.height + 3;
    stw.x = screenWidth-stwwidth;
    stw.h = screenHeight-bh;
    
    stw.drawable = XCreatePixmap(dpy, root, stwwidth, stw.h, DefaultDepth(dpy, screen));
    background  = XCreatePixmap(dpy, root, stwwidth, stw.h, DefaultDepth(dpy, screen));
    stw.gc = XCreateGC(dpy, root, 0, NULL);

    XGCValues gcv;
    gcv.foreground = stw.norm[ColFG];
    XChangeGC(dpy, stw.gc, GCForeground, &gcv);    
    
    XCopyArea(dpy, root, stw.drawable, stw.gc, stw.x, stw.y, stw.w, stw.h, 0,0 );
    XCopyArea(dpy, root, background, stw.gc, stw.x, stw.y, stw.w, stw.h, 0,0 );
    
    // initializing write
    stwwrite.xs = stwwrite.xc  = gappx;
    stwwrite.ys = stwwrite.yc = stw.font.height + gappx;
    stwwrite.xe = stwwrite.w = stw.w - gappx;
    stwwrite.ye = stwwrite.h = stw.h;
    stwwrite.drawable = stw.drawable;
    stwwrite.gc = stw.gc;
    stwwrite.font.ascent  = stw.font.ascent;
    stwwrite.font.descent = stw.font.descent;
    stwwrite.font.height  = stw.font.height;
    stwwrite.font.set     = stw.font.set;
    stwwrite.font.xfont   = stw.font.xfont;
    
   
    netstat.length = 100;
    
    
    netstat.lor.bytes    = (int*)malloc(sizeof(int)*netstat.length);
    netstat.eth0r.bytes  = (int*)malloc(sizeof(int)*netstat.length);
    netstat.wlan0r.bytes = (int*)malloc(sizeof(int)*netstat.length);    
    netstat.lot.bytes    = (int*)malloc(sizeof(int)*netstat.length);
    netstat.eth0t.bytes  = (int*)malloc(sizeof(int)*netstat.length);
    netstat.wlan0t.bytes = (int*)malloc(sizeof(int)*netstat.length);

   
    for(i = 0; i < netstat.length;i++){
      netstat.lor.bytes[i]    = 0;
      netstat.eth0r.bytes[i]  = 0;
      netstat.wlan0r.bytes[i] = 0;
      netstat.lot.bytes[i]    = 0;
      netstat.eth0t.bytes[i]  = 0;
      netstat.wlan0t.bytes[i] = 0;
    }
    
    netstat.lor.max      = 0;
    netstat.eth0r.max    = 0;
    netstat.wlan0r.max   = 0;
    netstat.lot.max      = 0;
    netstat.eth0t.max    = 0;
    netstat.wlan0t.max   = 0;

    for(i = 0; i < MAXPARTITIONS;i++){
      diskstat[i].length      = 100;
      diskstat[i].read.bytes  = (int*)malloc(sizeof(int)*netstat.length);
      diskstat[i].write.bytes = (int*)malloc(sizeof(int)*netstat.length);

      for(j = 0;j < diskstat[i].length;j++){
        diskstat[i].read.bytes[i]  = 0;
        diskstat[i].write.bytes[i] = 0;
      }
      diskstat[i].read.max  = 0;
      diskstat[i].write.max = 0;
      diskstat[i].writeges  = 0;
      diskstat[i].readges   = 0;
    }
}
