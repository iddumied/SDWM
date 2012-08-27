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
	int i, cpu_curr_pos, statusheigh = (bh-distance_y*2 - 1), cpuload, timeline_lineheight, x1,y1,x2,y2;
	
	for(i = 0; i < ncpus;i++){
		cpu_curr_pos = (cpu_posx + cpu_length*i + distance_x*i);
		
		// calculating how long to draw
		cpuload = ((int)((double)(cpu_length-3) * cpuloads[i]));
		
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
      gcv.foreground = dc.sbar[SBarLine];
      XChangeGC(dpy, dc.gc, GCForeground, &gcv);
      XDrawLine(dpy, timeline[i], dc.gc, x1,
                timeline_lineheight+1 ,x2,y2);
    }
    
    XCopyArea(dpy, timeline[i], dc.drawable, dc.gc, 0, 0, cpu_length, bh-1, cpu_curr_pos, 0);
	}
	
	drawsymbolstatus();
	
}
