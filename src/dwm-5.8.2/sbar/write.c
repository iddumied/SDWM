void wprint(const char * cstr);
void wprintln(const char * cstr);
void wprintcol(double percent, int length, double height_percent, int y_shifting); 
void wprintcolln(double percent, int length, double height_percent, int y_shifting);
void wprinttimeline(int bytes, int length, double fact, Timeline *timeline, unsigned int cline, unsigned int cpoint, int max);
void wprinttimelineln(int bytes, int length, double fact, Timeline *timeline, unsigned int cline, unsigned int cpoint, int max);
void wprintcols(double *percent, unsigned long *colors, int n, int length, double height_percent, int y_shifting);
void wprintcolsln(double *percent, unsigned long *colors, int n, int length, double height_percent, int y_shifting);


typedef struct {
  int xs, ys, w, h, xe, ye, xc, yc;
  GC *gc;
  Pixmap *drawable;
  struct {
    int ascent;
    int descent;
    int height;
    XFontSet set;
    XFontStruct *xfont;
  } font;
} Write;

Write stwwrite;


void wprinttimelineln(int bytes, int length, double fact, Timeline *timeline, unsigned int cline, unsigned int cpoint, int max)
{
  wprinttimeline(bytes, length, fact, timeline, cline, cpoint, max);
  
  stwwrite.xc = stwwrite.xs;
  stwwrite.yc += stwwrite.font.height;
}


void wprinttimeline(int bytes, int length, double fact, Timeline *timeline, unsigned int cline, unsigned int cpoint, int max)
{
  
  double percent;
  int i, x, py, ly, y, h = stwwrite.font.height*fact;
  y = stwwrite.yc - h;
  x = stwwrite.xc;

  
  XGCValues gcv;
  
  XDrawLine(dpy, stwwrite.drawable, stw.gc, x,y,x,stwwrite.yc);
  XDrawLine(dpy, stwwrite.drawable, stw.gc, x,stwwrite.yc,x+length,stwwrite.yc );
  x++;

 
  
  for(i = 1; i < length;i++){
    if(timeline->bytes[i] == 0) percent = 1.0;
    else percent = (1 - ((double)timeline->bytes[i] / (double)max));
    py = y + (int)((double)h * percent);
    ly = py+1;
    
    
   if(ly < stwwrite.yc){
    gcv.foreground = cline;
    XChangeGC(dpy, stwwrite.gc, GCForeground, &gcv);
    XDrawLine(dpy, stwwrite.drawable, stwwrite.gc, x+i-1, ly, x+i-1, stwwrite.yc-1);
   }
   if(py < stwwrite.yc){
    gcv.foreground = cpoint;
    XChangeGC(dpy, stwwrite.gc, GCForeground, &gcv);
    XDrawPoint(dpy, stwwrite.drawable, stwwrite.gc, x+i-1, py);
   }
   
   timeline->bytes[i-1] = timeline->bytes[i];
  }
  
  if(bytes == 0) percent = 1.0;
  else percent = (1 - ((double)bytes / (double)max));
  py = y + (int)((double)h * percent);
  ly = py+1;
    
  if(ly < stwwrite.yc){
    gcv.foreground = cline;
    XChangeGC(dpy, stwwrite.gc, GCForeground, &gcv);
    XDrawLine(dpy, stwwrite.drawable, stwwrite.gc, x+length-1, ly, x+length-1, stwwrite.yc-1);
  }
  if(py < stwwrite.yc){
    gcv.foreground = cpoint;
    XChangeGC(dpy, stwwrite.gc, GCForeground, &gcv);
    XDrawPoint(dpy, stwwrite.drawable, stwwrite.gc, x+length-1, py);
  }
   
  timeline->bytes[length-1] = bytes;
   
  gcv.foreground = stw.sel[ColFG];;
  XChangeGC(dpy, stw.gc, GCForeground, &gcv);

  stwwrite.xc += length;
}



void wprintcolln(double percent, int length, double height_percent, int y_shifting)
{
  wprintcol(percent, length, height_percent, y_shifting);
  
  stwwrite.xc = stwwrite.xs;
  stwwrite.yc += stwwrite.font.height;
}

void wprintcol(double percent, int length, double height_percent, int y_shifting)
{
  int y, h = (int)((double)stwwrite.font.height * height_percent);
  y =  stwwrite.yc - ((stwwrite.font.height - h)/2 + h) + y_shifting;
  
  if((stwwrite.xc + length) > stwwrite.xe) length = stwwrite.xe - stwwrite.xc;
  
  XDrawRectangle(dpy, stwwrite.drawable, stwwrite.gc, stwwrite.xc, 
                 y, length, h);
  
  XFillRectangle(dpy, stwwrite.drawable, stwwrite.gc, stwwrite.xc+2, 
                 y+2, (int)(((double)length-4) * percent), 
                 h-3);
  
  stwwrite.xc += length+1;
}

void wprintcolsln(double *percent, unsigned long *colors, int n, int length, double height_percent, int y_shifting)
{
  wprintcols(percent, colors, n, length, height_percent, y_shifting);
  
  stwwrite.xc = stwwrite.xs;
  stwwrite.yc += stwwrite.font.height;
}

void wprintcols(double *percent, unsigned long *colors, int n, int length, double height_percent, int y_shifting)
{
  int i, y, h = (int)((double)stwwrite.font.height * height_percent);
  XGCValues gcv;
  y =  stwwrite.yc - ((stwwrite.font.height - h)/2 + h) + y_shifting;
  
  if((stwwrite.xc + length) > stwwrite.xe) length = stwwrite.xe - stwwrite.xc;
  
  XDrawRectangle(dpy, stwwrite.drawable, stwwrite.gc, stwwrite.xc, 
                 y, length, h);
  
  for(i = 0;i < n; i++){
    gcv.foreground = colors[i];
    XChangeGC(dpy, stwwrite.gc, GCForeground, &gcv);
    
    XFillRectangle(dpy, stwwrite.drawable, stwwrite.gc, stwwrite.xc+2, 
                  y+2, (int)(((double)length-4) * percent[i]), 
                  h-3);
    stwwrite.xc += (int)(((double)length-4) * percent[i]);
  }
}


void wprintln(const char* cstr)
{
  wprint(cstr);
  stwwrite.xc = stwwrite.xs;
  stwwrite.yc += stwwrite.font.height;
}



void wprint(const char * cstr)
{
  int str_len = strlen(cstr);
  
  if(stwwrite.yc > stwwrite.ye) return;
  
  int i, nextxpos;

  nextxpos = textnw(cstr, str_len);
  
  for(i = 0;((nextxpos + stwwrite.xc) > stwwrite.xe) && (i < str_len);i++){
    nextxpos = textnw(cstr, str_len - i);
  }
  if(i > 0){
    if((str_len - i) > 0){
      if(stwwrite.font.set)
        XmbDrawString(dpy, stwwrite.drawable, stwwrite.font.set, stwwrite.gc, stwwrite.xc, stwwrite.yc, cstr, (str_len - i)); 
      else
        XDrawString(dpy, stwwrite.drawable, stwwrite.gc, stwwrite.xc, stwwrite.yc, cstr, (str_len - i)); 
    }
    
    stwwrite.xc = stwwrite.xs;
    stwwrite.yc += stwwrite.font.height;
    
    nextxpos = textnw( cstr+(str_len - i), i);
    if((nextxpos+stwwrite.xc) > stwwrite.xe){
      wprint(cstr+(str_len - i));
    
    // end of drawingare reatched
    }else if(stwwrite.yc > stwwrite.ye) return;
    else if(i > 0){
      if(stwwrite.font.set)
        XmbDrawString(dpy, stwwrite.drawable, stwwrite.font.set, stwwrite.gc, stwwrite.xc, stwwrite.yc, cstr+(str_len - i), i);
      else
        XDrawString(dpy, stwwrite.drawable, stwwrite.gc, stwwrite.xc, stwwrite.yc, cstr+(str_len - i), i);
      stwwrite.xc += nextxpos;
    }
  }else if(str_len > 0){
    if(stwwrite.font.set)
      XmbDrawString(dpy, stwwrite.drawable, stwwrite.font.set, stwwrite.gc, stwwrite.xc, stwwrite.yc, cstr, str_len); 
    else
      XDrawString(dpy, stwwrite.drawable, stwwrite.gc, stwwrite.xc, stwwrite.yc, cstr, str_len); 
    stwwrite.xc += nextxpos;
  }
}
