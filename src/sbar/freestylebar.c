void setup_freestylebar();
void setup_freestylebar_shift();
void draw_freestylebar();
void write_on_freestylebar(const char *cstr, const char *colorstr);

DC freestylebar;

static int freestylebar_curr_x;
static int freestylebar_width;


void draw_freestylebar(){

  int src_x = (freestylebar_curr_x < 0) ? (-1) * freestylebar_curr_x : 0;
  int width = (freestylebar_curr_x < 0) ? freestylebar_curr_x + freestylebar.w :
                ((freestylebar_curr_x + freestylebar.w) < freestylebar_width) ? 
                  freestylebar.w : freestylebar_width - freestylebar_curr_x;
  int des_x = (freestylebar_curr_x < 0) ? freestylebar.x : freestylebar_curr_x + freestylebar.x;

  // copping freestylebar on bar
    XCopyArea(dpy, freestylebar.drawable, dc.drawable, freestylebar.gc, 
                src_x, 0,                                    // src x, y
                  width, freestylebar.h,                     // width, height
                    des_x, freestylebar.y);                  // des x, y

  freestylebar_curr_x++;
  if (freestylebar_curr_x >= freestylebar_width) 
    freestylebar_curr_x = (-1) * freestylebar.w;

//  // copping freestylebar on bar
//  XCopyArea(dpy, freestylebar.drawable, dc.drawable, freestylebar.gc, 0,0, freestylebar.w, freestylebar.h, freestylebar.x, freestylebar.y);
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
  freestylebar.sbar[SBarLine]  = getcolor(themes[CurTheme].normal.timeln_line_color);
  freestylebar.sbar[SBarCpuPoint] = getcolor(themes[CurTheme].normal.timeln_point_color);
    
  freestylebar.font.ascent  = dc.font.ascent;
  freestylebar.font.descent = dc.font.descent;
  freestylebar.font.height  = dc.font.height;
  freestylebar.font.set     = dc.font.set;
  freestylebar.font.xfont   = dc.font.xfont;

  #ifdef INFO
  log_str("Setup freestylebar appearance and font", LOG_INFO);
  #endif

  int  l = 0, x, tmp, i;
  x = SBARTEXTW(mainsymbol);

  for(i = 0;i < LENGTH(layouts);i++){
    tmp = SBARTEXTW(layouts[i].symbol);

    if(l < tmp) l = tmp;
  } 
  x += l+TEXTW("")/2 ;


  for(i = 0;i < LENGTH(tags);i++)
      x = x + TEXTW(tags[i]);
  
    
  freestylebar.w  = freestylebar_width = sbar.cpu_posx - x - TEXTW("");
  freestylebar.y = dc.y;
  freestylebar.x = x;

  // because of bottomborder heigh - 1
  freestylebar.h = bh-1;

    
  freestylebar.drawable = XCreatePixmap(dpy, root, freestylebar.w, freestylebar.h, DefaultDepth(dpy, screen));
  freestylebar.gc = XCreateGC(dpy, root, 0, NULL);

  XGCValues gcv;
  gcv.foreground = freestylebar.norm[ColFG];
  XChangeGC(dpy, freestylebar.gc, GCForeground, &gcv);    

  XFillRectangle(dpy, freestylebar.drawable, freestylebar.gc, 0,0,freestylebar.w,freestylebar.h);


  // Drawing test text
  write_on_freestylebar("Hallo Welt", "#FF0000");

  freestylebar_curr_x = (-1) * freestylebar.w;
}

void write_on_freestylebar(const char *cstr, const char *colorstr) {
  int str_len = strlen(cstr);
  int h = dc.font.ascent + dc.font.descent;
  int y = dc.y + (dc.h / 2) - (h / 2) + dc.font.ascent;
  
  // cangen lenx to fit to text
  freestylebar.w = textnw(cstr, str_len) + 2;
  freestylebar.drawable = XCreatePixmap(dpy, root, freestylebar.w, freestylebar.h, DefaultDepth(dpy, screen));
  
  // clear
  XGCValues gcv;
  gcv.foreground = freestylebar.norm[ColBG];
  XChangeGC(dpy, freestylebar.gc, GCForeground, &gcv);    
  XFillRectangle(dpy, freestylebar.drawable, freestylebar.gc, 0,0,freestylebar.w, freestylebar.h);

  // changing write color
  gcv.foreground = getcolor(colorstr);
  XChangeGC(dpy, freestylebar.gc, GCForeground, &gcv);    

  // writing text
  if(freestylebar.font.set)
    XmbDrawString(dpy, freestylebar.drawable, freestylebar.font.set, freestylebar.gc, 0, y+1, cstr, str_len); 
  else
    XDrawString(dpy, freestylebar.drawable, freestylebar.gc, 0, y+1, cstr, str_len); 

}
