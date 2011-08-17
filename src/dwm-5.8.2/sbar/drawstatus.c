#include "drawsymbolstatus.c"

void drawstatus();

void drawstatus(){
  
  //clearing schreen 
  XGCValues gcv;
  gcv.foreground = dc.norm[ColBG];
  XChangeGC(dpy, dc.gc, GCForeground, &gcv);
  XFillRectangle(dpy, dc.drawable, dc.gc, 0, 0, screenWidth, bh);

  // draw border on nottom of bar
	gcv.foreground = dc.sbar[SBarBorder];
	XChangeGC(dpy, dc.gc, GCForeground, &gcv);
	XDrawLine(dpy, dc.drawable, dc.gc, 0, dc.h, screenWidth, dc.h );
	
	// CPU
	int i, cpu_curr_pos, statusheigh = (bh-distance_y*2 - 1), cpuload, timeline_lineheight, x1,y1,x2,y2;
	
	for(i = 0; i < ncpus;i++){
		cpu_curr_pos = (cpu_posx + cpu_length*i + distance_x*i);
		
    if(!cpumem_timeline){
      gcv.foreground = sbarcolor.white;
      XChangeGC(dpy, dc.gc, GCForeground, &gcv);
      XDrawRectangle(dpy, dc.drawable, dc.gc, cpu_curr_pos, 
                      distance_y, cpu_length, statusheigh);
    }
		// calculating how long to draw
		pthread_mutex_lock (&mutex);
		cpuload = ((int)((double)(cpu_length-3) * cpuloads[i]));
		pthread_mutex_unlock (&mutex);
		
    if(simply_style && !cpumem_timeline){
      gcv.foreground = dc.sel[ColFG];
      XChangeGC(dpy, dc.gc, GCForeground, &gcv);
      XFillRectangle(dpy, dc.drawable, dc.gc, cpu_curr_pos+2,
                      distance_y+2, cpuload, statusheigh-3);
      
      // because memory needs his white border color
      gcv.foreground = sbarcolor.white;
      XChangeGC(dpy, dc.gc, GCForeground, &gcv);
    }else if(cpumem_timeline){
      // shifting
      XCopyArea(dpy, timeline[i], timeline[i], dc.gc, (shift_left ? 1 : 0),
                0, cpu_length-1, bh-1, (shift_left ? 0 : 1), 0);
      
      if(shift_left){
        x1 = cpu_length-1;
        y1 = 0;
        x2 = cpu_length-1;
        y2 = bh-1;
      }
      else{
        x1 = 0;
        y1 = 0;
        x2 = 0;
        y2 = bh-1;
      }
      gcv.foreground = dc.sel[ColBG];
      XChangeGC(dpy, dc.gc, GCForeground, &gcv);
      XDrawLine(dpy, timeline[i], dc.gc, x1,y1,x2,y2);
      
      timeline_lineheight = (int)((double)(bh-1) * (1-cpuloads[i]));
      
      if(timeline_lineheight < (bh-1)){
        gcv.foreground = dc.sbar[SBarCpuPoint];
        XChangeGC(dpy, dc.gc, GCForeground, &gcv);
        XDrawPoint(dpy, timeline[i], dc.gc, x1, timeline_lineheight);
      }
      if(timeline_lineheight < (bh-2)){
        gcv.foreground = dc.sbar[SBarCpuLine];
        XChangeGC(dpy, dc.gc, GCForeground, &gcv);
        XDrawLine(dpy, timeline[i], dc.gc, x1,
                  timeline_lineheight+1 ,x2,y2);
      }
      
      XCopyArea(dpy, timeline[i], dc.drawable, dc.gc, 0, 0, cpu_length, bh-1, cpu_curr_pos, 0);
    }else
      XCopyArea(dpy, statusstyle.cpu_drawable, dc.drawable, dc.gc, 0, 0, cpuload, statusheigh-3, cpu_curr_pos+2, distance_y+2);
	}
	
	drawsymbolstatus();
	
	/*
   if(cpumem_timeline){
    gcv.foreground = sbarcolor.white;
    XChangeGC(dpy, dc.gc, GCForeground, &gcv);
   }
	
	// MEMORY + SWAP + Battery
	double used, buffer, cached, swapused, battstat;
  char batttime[6], battimesym[8],memstat[5], swapstat[4];
  int battre_h, battre_m, battime_len, y, h, battmode, swapusedper;
	// catching information
	pthread_mutex_lock (&mutex);
	used     = memory.pused;
	buffer   = memory.pbuffer;
	cached   = memory.pcached;
	swapused = memory.pswapused;
  battstat = battery.stat;
  battre_h = battery.remain.h;
  battre_m = battery.remain.m;
  sprintf(batttime,"%02d:%02d", battery.remain.h, battery.remain.m);
  battmode = battery.mode;
	pthread_mutex_unlock (&mutex);
	
  
	// memory border
  if(!cpumem_timeline){
	cpu_curr_pos += cpu_length + distance_x;
  XDrawRectangle(dpy, dc.drawable, dc.gc, cpu_curr_pos, 
										distance_y, mem_length, statusheigh);
  }
  
  
  // calc font height and battery text length
  battime_len = textnw(batttime, 5) + 2;
  h = dc.font.ascent + dc.font.descent;
  y = dc.y + (dc.h / 2) - (h / 2) + dc.font.ascent;
  
  // printing Arch symbol
  if(dc.font.set)
    XmbDrawString(dpy, dc.drawable, dc.font.set, dc.gc, 3, y+1, "\x18", 1);
  else
    XDrawString(dpy, dc.drawable, dc.gc, 3, y+1, "\x18", 1);
  
  //battery border + remain time
  if(!simply_style){
    if(battmode != CHARGED){
      if(dc.font.set)
        XmbDrawString(dpy, dc.drawable, dc.font.set, dc.gc, battery_pos_x, y, batttime, 5);
      else
        XDrawString(dpy, dc.drawable, dc.gc, battery_pos_x, y, batttime, 5);
    }
    
    XDrawRectangle(dpy, dc.drawable, dc.gc, battery_pos_x+battime_len+5, 
                    distance_y+2, 3, statusheigh-4);
    
    XDrawRectangle(dpy, dc.drawable, dc.gc, battery_pos_x+3+battime_len+5, 
                    distance_y, battery_length, statusheigh);
  }
  
  
  
  // memory usage
  if(!cpumem_timeline){
    int curr_pos = cpu_curr_pos+2;
    
    gcv.foreground = statusstyle.cmem_used;
    XChangeGC(dpy, dc.gc, GCForeground, &gcv);
    XFillRectangle(dpy, dc.drawable, dc.gc, curr_pos, 
                    distance_y+2, ((int)((double)mem_length-2) * used), statusheigh-3);
    
    curr_pos += ((int)((double)mem_length-2) * used);
    
    gcv.foreground = statusstyle.cmem_buffer;
    XChangeGC(dpy, dc.gc, GCForeground, &gcv);
    XFillRectangle(dpy, dc.drawable, dc.gc, curr_pos, 
                    distance_y+2, ((int)((double)mem_length-2) * buffer), statusheigh-3);
    
    curr_pos += ((int)((double)mem_length-2) * buffer);
    
    gcv.foreground = statusstyle.cmem_cached;
    XChangeGC(dpy, dc.gc, GCForeground, &gcv);
    XFillRectangle(dpy, dc.drawable, dc.gc, curr_pos, 
                    distance_y+2, ((int)((double)mem_length-2) * cached), statusheigh-3);
    
    // swap usage
    if(((int)((double)swap_length-2) * swapused) > 0){ // show onley if swap are used 
      gcv.foreground = sbarcolor.red;
      XChangeGC(dpy, dc.gc, GCForeground, &gcv);
      XFillRectangle(dpy, dc.drawable, dc.gc, cpu_curr_pos+2, 
                      (distance_y+2+(( statusheigh-3)/2)), ((int)((double)mem_length-2) * swapused), (statusheigh-3)/2);
    }
  }else{ //memory symbol
    gcv.foreground = sbarcolor.white;
    XChangeGC(dpy, dc.gc, GCForeground, &gcv);
    sprintf(memstat, "\x1a %d%c", (int)(used*100),'%');
    if(dc.font.set)
      XmbDrawString(dpy, dc.drawable, dc.font.set, dc.gc, mem_posx, y, memstat, 5);
    else
      XDrawString(dpy, dc.drawable, dc.gc, mem_posx, y, memstat, 5);
    
    swapusedper = (int)(swapused*100);
    if(swapusedper > 0){
      gcv.foreground = sbarcolor.red;
      XChangeGC(dpy, dc.gc, GCForeground, &gcv);
      sprintf(swapstat, " %d%c", swapusedper, '%');
      if(dc.font.set)
        XmbDrawString(dpy, dc.drawable, dc.font.set, dc.gc, mem_posx + 
                      ((used < 0.1) ? textnw(memstat,4) : textnw(memstat,5)), y, swapstat, 4);
      else
        XDrawString(dpy, dc.drawable, dc.gc, mem_posx + 
                    ((used < 0.1) ? textnw(memstat,4) : textnw(memstat,5)), y, swapstat, 4);
    }
  }
  // battery  
 
  if(battmode == CHARGING && !simply_style){
    gcv.foreground = sbarcolor.yellow;
    XChangeGC(dpy, dc.gc, GCForeground, &gcv);
    
    XFillRectangle(dpy, dc.drawable, dc.gc, battery_pos_x+battime_len+5+1, 
                  distance_y+2+1, 2, statusheigh-5);
  }else{
    gcv.foreground = ((int)(battstat * 255)) * statusstyle.gl + 
            (255 - ((int)(battstat * 255))) * statusstyle.rl;
    XChangeGC(dpy, dc.gc, GCForeground, &gcv);
  }
  
   if(!simply_style){
    XFillRectangle(dpy, dc.drawable, dc.gc, battery_pos_x+5+battime_len+5, 
                    distance_y+2, (battery_length-3)*battstat, statusheigh-3);
    
   }else if(battmode == CHARGING){
     sprintf(battimesym, "%c %s", (char)23, batttime);
    if(dc.font.set)
      XmbDrawString(dpy, dc.drawable, dc.font.set, dc.gc, battery_pos_x, y, battimesym, 7);
    else
      XDrawString(dpy, dc.drawable, dc.gc, battery_pos_x, y, battimesym, 7);
    
  }else if(battmode == CHARGED){
    sprintf(battimesym, "%c Full", (char)23);
    if(dc.font.set)
      XmbDrawString(dpy, dc.drawable, dc.font.set, dc.gc, battery_pos_x, y, battimesym, 6);
    else
      XDrawString(dpy, dc.drawable, dc.gc, battery_pos_x, y, battimesym, 6);
    

    
  }else{
    if(battstat < 0.25)
      sprintf(battimesym, "%c %s", (char)11, batttime);
    else if(battstat < 0.5)
      sprintf(battimesym, "%c %s", (char)12, batttime);
    else if(battstat < 0.75)
      sprintf(battimesym, "%c %s", (char)13, batttime);
    else
      sprintf(battimesym, "%c %s", (char)14, batttime);


      
    if(dc.font.set)
      XmbDrawString(dpy, dc.drawable, dc.font.set, dc.gc, battery_pos_x, y, battimesym, 7);
    else
      XDrawString(dpy, dc.drawable, dc.gc, battery_pos_x, y, battimesym, 7);
  }
  
    gcv.foreground = sbarcolor.yellow;
    XChangeGC(dpy, dc.gc, GCForeground, &gcv);
    if(dc.font.set)
      XmbDrawString(dpy, dc.drawable, dc.font.set, dc.gc, uptime_posx, y, tbar_uptime.uptime, tbar_uptime.len);
    else
      XDrawString(dpy, dc.drawable, dc.gc, uptime_posx, y, tbar_uptime.uptime, tbar_uptime.len);
  
    printf("\n\nstr %s len %d\n\n",tbar_uptime.uptime, tbar_uptime.len);
    */
}