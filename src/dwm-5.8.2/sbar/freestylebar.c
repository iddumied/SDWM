void setup_freestylebar();
void draw_freestylebar();

DC freestylebar;

void draw_freestylebar(){

  //XGCValues gcv;
  //gcv.foreground = freestylebar.norm[ColBorder];
  //XChangeGC(dpy, freestylebar.gc, GCForeground, &gcv);    

  //XFillRectangle(dpy, freestylebar.drawable, freestylebar.gc, 0,0,200,20);

	XCopyArea(dpy, freestylebar.drawable, dc.drawable, freestylebar.gc, 0,0, freestylebar.w, freestylebar.h, freestylebar.x, freestylebar.y);


}


void setup_freestylebar(){
  // initializing freestylebar
  freestylebar.norm[ColBorder]    = getcolor(themes[CurTheme].normal.normbordercolor);
  freestylebar.norm[ColBG]        = getcolor(themes[CurTheme].normal.normbgcolor);
  freestylebar.norm[ColFG]        = getcolor(themes[CurTheme].normal.normfgcolor);
  freestylebar.sel[ColBorder]     = getcolor(themes[CurTheme].normal.selbordercolor);
  freestylebar.sel[ColBG]         = getcolor(themes[CurTheme].normal.selbgcolor);
  freestylebar.sel[ColFG]         = getcolor(themes[CurTheme].normal.selfgcolor);
  freestylebar.sbar[SBarBorder]   = getcolor(themes[CurTheme].normal.botbordercolor);
  freestylebar.sbar[SBarCpuLine]  = getcolor(themes[CurTheme].normal.timeln_line_color);
  freestylebar.sbar[SBarCpuPoint] = getcolor(themes[CurTheme].normal.timeln_point_color);
    
    
  freestylebar.font.ascent  = dc.font.ascent;
  freestylebar.font.descent = dc.font.descent;
  freestylebar.font.height  = dc.font.height;
  freestylebar.font.set     = dc.font.set;
  freestylebar.font.xfont   = dc.font.xfont;

	int	x, i;
	x = TEXTW(mainsymbol);
    
	for(i = 0;i < LENGTH(tags);i++)
			x = x + TEXTW(tags[i]);
	


    
  freestylebar.w = cpu_posx - x;
  freestylebar.y = dc.y;
  freestylebar.x = x;

  if(bottomborder) //TODO rem bottomborder value bottom border always on!
    freestylebar.h = bh-1;
  else
    freestylebar.h = bh;

  printf("\n\nfree h,w: %d, %d\nx: %d   cpu_posx: %d\n\n",freestylebar.h,freestylebar.w, x, cpu_posx);
    
  freestylebar.drawable = XCreatePixmap(dpy, root, freestylebar.w, freestylebar.h, DefaultDepth(dpy, screen));
  freestylebar.gc = XCreateGC(dpy, root, 0, NULL);

  XGCValues gcv;
  gcv.foreground = freestylebar.norm[ColBorder];
  XChangeGC(dpy, freestylebar.gc, GCForeground, &gcv);    

  XFillRectangle(dpy, freestylebar.drawable, freestylebar.gc, 0,0,freestylebar.w,freestylebar.h);
}
