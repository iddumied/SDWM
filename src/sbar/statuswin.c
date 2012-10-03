DC stw;
Pixmap background;

typedef struct {
  int length;
  unsigned long long readges, writeges;
  Timeline read, write;
} DiskStatistic;

typedef struct {
  char line_seperator[512], read_prefix[32], write_prefix[32], readed_prefix[32], written_prefix[32], free_prefix[32];
  Bool free_percent;
  int max_char_len, max_chrs_per_line, max_chrs_per_halfln, min_status_win_width;
} DiskStatusWinUtils;


DiskStatistic diskstat[MAXPARTITIONS];
DiskStatusWinUtils diskstat_utils;
void setup_stw();
void setup_stw_disks();
void drawstw();
void calc_timline_max(Timeline *timeline, int bytes, int length);
void togglestw();


void togglestw(){
  draw_status_win = !draw_status_win;

  Monitor *m;

  for(m = mons; m; m = m->next){
    if(draw_status_win) XMapWindow(dpy,m->statuswin);
    else XUnmapWindow(dpy,m->statuswin);
  }
}


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
  

  // values
  char stwbuffer[100], hread[12], maxhread[12];
  int i;
  
  XGCValues gcv;
  gcv.foreground = stw.sel[ColFG];
  XChangeGC(dpy, stw.gc, GCForeground, &gcv);

    
  // statusmesage
  if(sbar_status_symbols[DrawUptime].active){
    sprintf(stwbuffer,"Chief @ ArchLinux  -  online since:  %s", uptime.since);
    wprintln("");
    wprintln(stwbuffer);
    wprintln("  |");
  }
  wprintln("  +--status");
  wprintln("  |    |");
  wprintln("  |    +--cpu");
  wprintln("  |    |    |");

  for(i = 0; i < cpuinfo.ncpus; i++){;
    sprintf(stwbuffer, "  |    |    +--cpu%d:  %d%c",i+1, (int)(cpuinfo.cpuloads[i]*100), '%');
    wprintln(stwbuffer);
  }
 
  wprintln("  |    |");
  if(sbar_status_symbols[DrawMemory].active){
    wprint("  |    +--ram:  ");
    double percent[3] = {memory.pused,memory.pbuffer,memory.pcached}; 
    unsigned long colors[3] = { stw.sel[ColFG],
    stw.sbar[SBarLine], stw.sbar[SBarCpuPoint] };
    wprintcolsln(&percent, &colors, 3, 100, 0.65,2);

    gcv.foreground = stw.sel[ColFG];
    XChangeGC(dpy, stwwrite.gc, GCForeground, &gcv);

    wprintln("  |    |    |");
    sprintf(stwbuffer, "  |    |    +--used:  %d MB / %d MB  -  %d%c", memory.used/1024, memory.total/1024,(int)(memory.pused*100),'%');
    wprintln(stwbuffer);  
    sprintf(stwbuffer, "  |    |    +--buffer:  %d MB", memory.buffer/1024);
    wprintln(stwbuffer);
    sprintf(stwbuffer, "  |    |    +--cached:  %d MB", memory.cached/1024);
    wprintln(stwbuffer);
    wprintln("  |    |");

    if(memory.swapused/1024 == 0){       sprintf(stwbuffer, "  |    +--swap:  %d MB / %d MB  -  %d%c", memory.swapused/1024, memory.swaptotal/1024, (int)(memory.pswapused*100),'%');
      wprintln(stwbuffer);  
    }else if(memory.swaptotal > 0){                                    
      wprint("  |    +--swap:  ");
      wprintcolln(memory.pswapused, 100, 0.65, 2 );
      wprintln("  |    |    |");
      sprintf(stwbuffer, "  |    |    +--used:  %d MB / %d MB  -  %d%c", memory.swapused/1024, memory.swaptotal/1024, (int)(memory.pswapused*100),'%');
      wprintln(stwbuffer);  
      wprintln("  |    |");
    }
  }  

  char pipe = (sbar_status_symbols[DrawBattery].active ? '|' : ' ');
  wprintln("  |    +--processes  ");
  sprintf(stwbuffer, "  |    %c    |", pipe);
  wprintln(stwbuffer);
  sprintf(stwbuffer, "  |    %c    +--amount:  %d",pipe , cpuinfo.processes.total);
  wprintln(stwbuffer);  
  sprintf(stwbuffer, "  |    %c    +--running:  %d",pipe , cpuinfo.processes.running);
  wprintln(stwbuffer);                                        
  sprintf(stwbuffer, "  |    %c    +--blocked:  %d",pipe , cpuinfo.processes.blocked);
  wprintln(stwbuffer);

  if(sbar_status_symbols[DrawBattery].active){
    wprintln("  |    |");
    sprintf(stwbuffer, "  |    +--battery:  %d%c", (int)(battery.stat*100), '%');
    wprintln(stwbuffer);
                                        wprintln("  |         |");
    if(battery.mode == CHARGING)                wprintln("  |         +--mode:  charging");
    else if(battery.mode == DISCHARGING)        wprintln("  |         +--mode:  discharging");    
    else if(battery.mode == CHARGED)            wprintln("  |         +--mode:  charged");    

    sprintf(stwbuffer, "  |         +--remain:  %02d:%02d", battery.remain.h,battery.remain.m);
    wprintln(stwbuffer);
    wprintln("  |");
  }

  if(sbar_status_symbols[DrawNet].active){
    wprintln("  +--net");
    wprintln("       |");

    if (!show_net_lo_interface && !strcmp(net.interfaces[net.num_interfaces - 1].name, "lo"))
      net.num_interfaces--;

    for (i = 0; i < net.num_interfaces - 1; i++) {
      if (!show_net_lo_interface && !strcmp(net.interfaces[i].name, "lo")) continue;


      sprintf(stwbuffer, "       +--%s", net.interfaces[i].name);
      wprint(stwbuffer);
     
      calc_timline_max(&net.interfaces[i].timeline.t, net.interfaces[i].between.transmit.bytes_per_sec, net.timeline_length);
      calc_timline_max(&net.interfaces[i].timeline.r, net.interfaces[i].between.receive.bytes_per_sec, net.timeline_length);
      if(net.interfaces[i].timeline.r.max == 0 && net.interfaces[i].timeline.t.max == 0){                                 
        if (net.interfaces[i].online || net.interfaces[i].easy_online)
          wprintln(":  inactive");
        else if (net.interfaces[i].state_unknowen)
          wprintln(":  unknowen");
        else 
          wprintln(":  down");
      }else{
        wprintln("");
        wprintln("       |    |");
        if(net.interfaces[i].timeline.t.max == 0)    
          wprintln("       |    +--up:  inactiv");
        else{  
          human_readable(net.interfaces[i].between.transmit.bytes_per_sec, False, hread);
          human_readable(net.interfaces[i].timeline.t.max, False, maxhread);
          sprintf(stwbuffer, "       |    +--up:  %s  @  %s ", hread, maxhread);
          wprintln(stwbuffer);
          wprintln("       |    |    |");
          wprint("       |    |    +--");
          wprinttimelineln(net.interfaces[i].between.transmit.bytes_per_sec, net.timeline_length, 1, 
                              &net.interfaces[i].timeline.t, stw.sbar[SBarLine], 
                                stw.sbar[SBarCpuPoint],net.interfaces[i].timeline.t.max);
          wprintln("       |    |");
        }
   
        if(net.interfaces[i].timeline.r.max == 0)    
          wprintln("       |    +--down:  inactiv");
        else{  
          human_readable(net.interfaces[i].between.receive.bytes_per_sec, False, hread);
          human_readable(net.interfaces[i].timeline.r.max, False, maxhread);
          sprintf(stwbuffer, "       |    +--down:  %s  @  %s ", hread, maxhread);
          wprintln(stwbuffer);
          wprintln("       |         |");
          wprint("       |         +--");
          wprinttimelineln(net.interfaces[i].between.receive.bytes_per_sec, net.timeline_length, 1, 
                            &net.interfaces[i].timeline.r, stw.sbar[SBarLine], 
                              stw.sbar[SBarCpuPoint],net.interfaces[i].timeline.r.max);
        }
        wprintln("       |");
      }                      
    }

    // las interface with out pipe slastes
    sprintf(stwbuffer, "       +--%s", net.interfaces[i].name);
    wprint(stwbuffer);
    
    calc_timline_max(&net.interfaces[i].timeline.t, net.interfaces[i].between.transmit.bytes_per_sec, net.timeline_length);
    calc_timline_max(&net.interfaces[i].timeline.r, net.interfaces[i].between.receive.bytes_per_sec, net.timeline_length);
    if(net.interfaces[i].timeline.r.max == 0 && net.interfaces[i].timeline.t.max == 0){                                 
      if (net.interfaces[i].online || net.interfaces[i].easy_online)
        wprintln(":  inactive");
      else if (net.interfaces[i].state_unknowen)
        wprintln(":  unknowen");
      else 
        wprintln(":  down");
    }else{
      wprintln("");
      wprintln("            |");
      if(net.interfaces[i].timeline.t.max == 0)    
        wprintln("            +--up:  inactiv");
      else{  
        human_readable(net.interfaces[i].between.transmit.bytes_per_sec, False, hread);
        human_readable(net.interfaces[i].timeline.t.max, False, maxhread);
        sprintf(stwbuffer, "            +--up:  %s  @  %s ", hread, maxhread);
        wprintln(stwbuffer);
        wprintln("            |    |");
        wprint("            |    +--");
        wprinttimelineln(net.interfaces[i].between.transmit.bytes_per_sec, net.timeline_length, 1, 
                            &net.interfaces[i].timeline.t, stw.sbar[SBarLine], 
                              stw.sbar[SBarCpuPoint],net.interfaces[i].timeline.t.max);
        wprintln("            |");
      }
   
      if(net.interfaces[i].timeline.r.max == 0)    
        wprintln("            +--down:  inactiv");
      else{  
        human_readable(net.interfaces[i].between.receive.bytes_per_sec, False, hread);
        human_readable(net.interfaces[i].timeline.r.max, False, maxhread);
        sprintf(stwbuffer, "            +--down:  %s  @  %s ", hread, maxhread);
        wprintln(stwbuffer);
        wprintln("                 |");
        wprint("                 +--");
        wprinttimelineln(net.interfaces[i].between.receive.bytes_per_sec, net.timeline_length, 1, 
                          &net.interfaces[i].timeline.r, stw.sbar[SBarLine], 
                            stw.sbar[SBarCpuPoint],net.interfaces[i].timeline.r.max);
      }
    }                      

  }else{
    wprintln("  |");
    wprintln("  +--net: error");
  }  

  /***********************************************
   *             Partition Part                  *
   ***********************************************/

  char ebuf[5][20], ebuf2[5][20], buff2[100], *tmp_ptr, *tmp_ptr2;
  int countdisks, mj, mi = 0, len;
  stwwrite.xs = stwwrite.xc  = screenWidth - gappx - status_win_width;// Statuswin pos and size to config.h
  stwwrite.ys = stwwrite.yc = (stw.font.height + gappx);
  stwwrite.xe = stwwrite.w = stw.w - gappx;
  stwwrite.ye = stwwrite.h = stw.h;
  
  for(i = 0;i < MAXPARTITIONS;i++)
    if(!disks[i].active) break;
  
  countdisks = i;
  
  wprintln("");
  sprintf(stwbuffer,"Volumes mounted:  %d  of  %d  Disks",mounted_volumes,countdisks);
  wprintln(stwbuffer);
  wprintln(diskstat_utils.line_seperator); 
  for(i = 0; i < MAXPARTITIONS; i++){
    if(!disks[i].active) break;
    
  
    if(disks[i].mountpoint != NULL){

      // path + mountpoint
      tmp_ptr = let_str_fitt_to(stwbuffer, disks[i].path, diskstat_utils.max_chrs_per_halfln, strlen(disks[i].path));
      wprint(tmp_ptr);
      stwwrite.xc = screenWidth - gappx - status_win_width / 2; // set cursor to half
      tmp_ptr = let_str_fitt_to(stwbuffer, disks[i].mountpoint, diskstat_utils.max_chrs_per_halfln, strlen(disks[i].path));
      wprintln(tmp_ptr);

      // usage
      //wprintcol(disks[i].pused, status_win_width / 2 - gappx, 0.65, 2);
      //stwwrite.xc += gappx;
      human_readable_disk(disks[i].avil, &ebuf[0]);
      human_readable_disk(disks[i].total, &ebuf[1]);
      sprintf(stwbuffer,"%s / %s", ebuf[0], ebuf[1]);
      wprint(stwbuffer);
      stwwrite.xc = screenWidth - gappx - status_win_width / 2; // set cursor to half
      wprintcolln(disks[i].pused, status_win_width / 2 - gappx, 0.65, 2);

      // read write

      // calculation inforamtions and make it readable
      diskstat[i].readges += disks[i].between.read;
      calc_timline_max(&diskstat[i].read, disks[i].between.read, diskstat[i].length);
      human_readable_disk(disks[i].now.read, &ebuf[0]);
      human_readable(disks[i].between.read, False, &ebuf[1]);
      human_readable(diskstat[i].read.max, False, &ebuf[2]);
      human_readable_disk(diskstat[i].readges, &ebuf[3]);

      diskstat[i].writeges += disks[i].between.write;
      calc_timline_max(&diskstat[i].write, disks[i].between.write, diskstat[i].length);
      human_readable_disk(disks[i].now.write, &ebuf2[0]);
      human_readable(disks[i].between.write, False, &ebuf2[1]);
      human_readable(diskstat[i].write.max, False, &ebuf2[2]);
      human_readable_disk(diskstat[i].writeges, &ebuf2[3]);

      sprintf(stwbuffer, "r: %s / %s", ebuf[3], ebuf[0]);
      wprint(stwbuffer);
      stwwrite.xc = screenWidth - gappx - status_win_width / 2; // set cursor to half
      sprintf(stwbuffer, "w: %s / %s", ebuf2[3], ebuf2[0]);
      wprintln(stwbuffer);

      if(diskstat[i].read.max > 0 || diskstat[i].write.max > 0){
        sprintf(stwbuffer, "%s%s @ %s", diskstat_utils.read_prefix, ebuf[1], ebuf[2]);
        wprint(stwbuffer);
        stwwrite.xc = screenWidth - gappx - status_win_width / 2; // set cursor to half
        sprintf(stwbuffer, "%s%s @ %s", diskstat_utils.write_prefix, ebuf2[1], ebuf2[2]);
        wprintln(stwbuffer);
        stwwrite.yc += 6;


        // printing timeline if max > 0
        wprinttimeline(disks[i].between.read, diskstat[i].length, 1, &diskstat[i].read, 
                      stw.sbar[SBarLine], stw.sbar[SBarCpuPoint], diskstat[i].read.max);
      

        stwwrite.xc = screenWidth - gappx - status_win_width / 2; // set cursor to half

        wprinttimelineln(disks[i].between.write, diskstat[i].length, 1, &diskstat[i].write, 
                      stw.sbar[SBarLine], stw.sbar[SBarCpuPoint], diskstat[i].write.max);
      

      }


      // line seperator
      wprintln(diskstat_utils.line_seperator); 


      

/*

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


      ****** RAED *****

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
                        stw.sbar[SBarLine], stw.sbar[SBarCpuPoint], diskstat[i].read.max);
        
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
                        stw.sbar[SBarLine], stw.sbar[SBarCpuPoint], diskstat[i].read.max);
          
          sprintf(stwbuffer,"   %s",ebuf[3]);
          wprintln(stwbuffer);
          wprintln("            |");

        }else{
          diskstat[i].readges = 0; // clearing history
          sprintf(stwbuffer,"            +--readed %s",ebuf[0]);
          wprintln(stwbuffer);
        }

      }


      ****** WRITE *****

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
                        stw.sbar[SBarLine], stw.sbar[SBarCpuPoint], diskstat[i].write.max);
        
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
                        stw.sbar[SBarLine], stw.sbar[SBarCpuPoint], diskstat[i].write.max);
          
          sprintf(stwbuffer,"   %s",ebuf[3]);
          wprintln(stwbuffer);
        }else{
          diskstat[i].writeges = 0; // clearing history
          sprintf(stwbuffer,"            +--writen: %s",ebuf[0]);
          wprintln(stwbuffer);
        }

      }

      if(mi < mounted_volumes) wprintln("  |");
    */
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

void setup_stw()
{
    int h, i, j;
  
    // initializing stw
    stw.norm[ColBorder]    = getcolor(themes[CurTheme].stw.normbordercolor);
    stw.norm[ColBG]        = getcolor(themes[CurTheme].stw.normbgcolor);
    stw.norm[ColFG]        = getcolor(themes[CurTheme].stw.normfgcolor);
    stw.sel[ColBorder]     = getcolor(themes[CurTheme].stw.selbordercolor);
    stw.sel[ColBG]         = getcolor(themes[CurTheme].stw.selbgcolor);
    stw.sel[ColFG]         = getcolor(themes[CurTheme].stw.selfgcolor);
    stw.sbar[SBarBorder]   = getcolor(themes[CurTheme].stw.botbordercolor);
    stw.sbar[SBarLine]  = getcolor(themes[CurTheme].stw.timeln_line_color);
    stw.sbar[SBarCpuPoint] = getcolor(themes[CurTheme].stw.timeln_point_color);
    
    
    stw.font.ascent  = dc.font.ascent;
    stw.font.descent = dc.font.descent;
    stw.font.height  = dc.font.height;
    stw.font.set     = dc.font.set;
    stw.font.xfont   = dc.font.xfont;
    
    stw.w = screenWidth;
    stw.y = dc.font.height + 3;
    stw.x = gappx;
    stw.h = screenHeight-bh;
    
    stw.drawable = XCreatePixmap(dpy, root, screenWidth, stw.h, DefaultDepth(dpy, screen));
    background  = XCreatePixmap(dpy, root, screenWidth, stw.h, DefaultDepth(dpy, screen));
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
    
   
    net.timeline_length = timeline_length;
    
    
    for (j = 0; j < MAX_NET_INTERFACES; j++) {
      net.interfaces[j].timeline.r.bytes = (int*)malloc(sizeof(int)*timeline_length);    
      net.interfaces[j].timeline.t.bytes = (int*)malloc(sizeof(int)*timeline_length);
     
      for(i = 0; i < net.timeline_length; i++){
        net.interfaces[j].timeline.r.bytes[i] = 0;
        net.interfaces[j].timeline.t.bytes[i] = 0;
      }
      
      net.interfaces[j].timeline.r.max = 0;
      net.interfaces[j].timeline.t.max = 0;
    }

    // seting up disks window
    setup_stw_disks();
}


void setup_stw_disks() {

    int i, j;

    // calculate minimum status_win_width
    const char *num[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9" };
    const char *byte[] = { "B/s","KB/s","MB/s" };
    char max_numi, max_bytei;


    for (i = 0, max_numi = 0; i < 10; i++) {
      if (max_numi < textnw(num[i], 1)) 
        max_numi = textnw(num[i], 1);
    }

    for (i = 0, max_bytei = 0; i < 3; i++) {
      if (max_bytei < textnw(byte[i], strlen(byte[i])))
        max_bytei = textnw(byte[i], strlen(byte[i]));
    }

    // maximum if minimal printable text is "000 MB/s @ 000 MB/s"
    diskstat_utils.min_status_win_width = (textnw("  @  ", 5) + 6 * max_numi + 2 * max_bytei) * 2 + gappx;

    if (status_win_width < diskstat_utils.min_status_win_width) {
      char logbuff[64];
      sprintf(logbuff, "status_win_width (%d) to smal => set to %d", status_win_width, diskstat_utils.min_status_win_width);
      log_str(logbuff, LOG_WARNING);

      status_win_width = diskstat_utils.min_status_win_width;
    } 

    // init prefixes
    sprintf(diskstat_utils.read_prefix, "%s", "read: ");
    sprintf(diskstat_utils.write_prefix,"%s", "write: ");

    int write_prefix_len = textnw(diskstat_utils.write_prefix, strlen(diskstat_utils.write_prefix));
    int read_prefix_len  = textnw(diskstat_utils.read_prefix, strlen(diskstat_utils.read_prefix));
    int max_prefix = (write_prefix_len > read_prefix_len) ? write_prefix_len : read_prefix_len;

    if ((max_prefix + diskstat_utils.min_status_win_width) > status_win_width) {
      sprintf(diskstat_utils.read_prefix, "%s", "r: ");
      sprintf(diskstat_utils.write_prefix,"%s", "w: ");

      write_prefix_len = textnw(diskstat_utils.write_prefix, strlen(diskstat_utils.write_prefix));
      read_prefix_len  = textnw(diskstat_utils.read_prefix, strlen(diskstat_utils.read_prefix));
      max_prefix = (write_prefix_len > read_prefix_len) ? write_prefix_len : read_prefix_len;

      if ((max_prefix + diskstat_utils.min_status_win_width) > status_win_width) {
        diskstat_utils.read_prefix[0]  = (char) 0;
        diskstat_utils.write_prefix[0] = (char) 0;
      }
    }

    //write_prefix[32], readed_prefix[32], written_prefix[32], free_prefix[32];
    

    // init diskstat (Distk Timelines and stuff)
    for(i = 0; i < MAXPARTITIONS;i++){
      diskstat[i].length      = status_win_width / 2 - gappx;
      diskstat[i].read.bytes  = (int*)malloc(sizeof(int)*timeline_length);
      diskstat[i].write.bytes = (int*)malloc(sizeof(int)*timeline_length);

      for(j = 0;j < diskstat[i].length;j++){
        diskstat[i].read.bytes[i]  = 0;
        diskstat[i].write.bytes[i] = 0;
      }
      diskstat[i].read.max  = 0;
      diskstat[i].write.max = 0;
      diskstat[i].writeges  = 0;
      diskstat[i].readges   = 0;
    }



    diskstat_utils.line_seperator[0] = '-';
    diskstat_utils.line_seperator[1] = (char) 0;
    
    int line_seperator_len = textnw(diskstat_utils.line_seperator, 1);
    
    for (i = 1; i < status_win_width / line_seperator_len; i++)
      add_char_to_str(diskstat_utils.line_seperator, '-', i);

    diskstat_utils.line_seperator[i - 1] = (char) 0;
    diskstat_utils.max_char_len = 0;
    int char_len = 0;

    // Calculate Max Char-Width
    
    #ifdef INFO
    char logbuffer[256];
    int max_char;
    #endif

    for (i = 0; i < 256; i++) {
      char_len = textnw(&i, 1);

      #ifdef INFO
      sprintf(logbuffer, "Char-Width: %c (0x%02X): %d", i, i, char_len);
      log_str(logbuffer, LOG_INFO);
      if (diskstat_utils.max_char_len < char_len)
        max_char = i;
      #endif

      if (diskstat_utils.max_char_len < char_len)
        diskstat_utils.max_char_len = char_len;
    }

    #ifdef INFO
    sprintf(logbuffer, "Max Char-Width: %c (0x%02X): %d", max_char, max_char, textnw(&max_char, 1));
    log_str(logbuffer, LOG_INFO);
    #endif

    diskstat_utils.max_chrs_per_line = status_win_width / diskstat_utils.max_char_len;
    diskstat_utils.max_chrs_per_halfln = (status_win_width / 2) / diskstat_utils.max_char_len;

}
