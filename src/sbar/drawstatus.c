#include "drawsymbolstatus.c"

void drawstatus(){
  
  //clearing schreen 
  XGCValues gcv;
  gcv.foreground = dc.norm[ColBG];
  XChangeGC(dpy, dc.gc, GCForeground, &gcv);
  XFillRectangle(dpy, dc.drawable, dc.gc, 0, 0, screenWidth, bh);

  // draw border on bottom of bar
	gcv.foreground = dc.sbar[SBarBorder];
	XChangeGC(dpy, dc.gc, GCForeground, &gcv);
	XDrawLine(dpy, dc.drawable, dc.gc, 0, dc.h, screenWidth, dc.h );
	
	// CPU
	int i, cpu_curr_pos, statusheigh = (bh-symbol_distance_y*2 - 1), cpuload, cpu_timeline_lineheight, x1,y1,x2,y2;
	
	for(i = 0; i < cpuinfo.ncpus;i++){
		cpu_curr_pos = (sbar.cpu_posx + sbar.cpu_length*i + cpu_distance_x*i);
		
		// calculating how long to draw
		cpuload = ((int)((double)(sbar.cpu_length-3) * cpuinfo.cpuloads[i]));
		
    // shifting
    XCopyArea(dpy, sbar.cpu_timeline[i], sbar.cpu_timeline[i], dc.gc, (cpu_shift_left ? 1 : 0),
              0, sbar.cpu_length-1, bh-1, (cpu_shift_left ? 0 : 1), 0);
    
    if(cpu_shift_left){
      x1 = sbar.cpu_length-1;
      y1 = 0;
      x2 = sbar.cpu_length-1;
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
    XDrawLine(dpy, sbar.cpu_timeline[i], dc.gc, x1,y1,x2,y2);
    
    cpu_timeline_lineheight = (int)((double)(bh-1) * (1-cpuinfo.cpuloads[i]));
    
    if(cpu_timeline_lineheight < (bh-1)){
      gcv.foreground = dc.sbar[SBarCpuPoint];
      XChangeGC(dpy, dc.gc, GCForeground, &gcv);
      XDrawPoint(dpy, sbar.cpu_timeline[i], dc.gc, x1, cpu_timeline_lineheight);
    }
    if(cpu_timeline_lineheight < (bh-2)){
      gcv.foreground = dc.sbar[SBarLine];
      XChangeGC(dpy, dc.gc, GCForeground, &gcv);
      XDrawLine(dpy, sbar.cpu_timeline[i], dc.gc, x1,
                cpu_timeline_lineheight+1 ,x2,y2);
    }
    
    XCopyArea(dpy, sbar.cpu_timeline[i], dc.drawable, dc.gc, 0, 0, sbar.cpu_length, bh-1, cpu_curr_pos, 0);
	}
	
	drawsymbolstatus();
	
}
