void print_statusbar(void);

void print_statusbar(void)
{
  GC gc = XCreateGC(dpy, DefaultRootWindow(dpy), 0, NIL);
  
  int i = 0;
  for(;i < BAR_LEN;i++)
  {
    XSetForeground(dpy, gc, statusbar_colors[i]);
    XDrawLine(dpy, w, gc, 0, i, screenWidth, i);
  }
  // Send the "DrawLine" request to the server
  
  // drawa left an rigth border
  XSetForeground(dpy, gc, blackColor);
  XDrawLine(dpy, w, gc, 0, 0, 0, BAR_LEN);
  XDrawLine(dpy, w, gc, screenWidth, 0, screenWidth, BAR_LEN);
  
  
  XFlush(dpy);
}
