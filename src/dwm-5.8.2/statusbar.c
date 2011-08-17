#define NIL (0) 

void print_statusbar(Display *dpy, Window * barwin);

void print_statusbar(Display *dpy, Window * barwin)
{


    int blackColor = BlackPixel(dpy, DefaultScreen(dpy));
    int whiteColor = WhitePixel(dpy, DefaultScreen(dpy));
    int screenWidth = DisplayWidth(dpy, DefaultScreen(dpy));

/*
      Window barwin = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0, 
				     screenWidth, 100, 0, whiteColor, whiteColor);





       XSelectInput(dpy, barwin, StructureNotifyMask);

      // "Map" the window (that is, make it appear on the screen)

      XMapWindow(dpy, barwin);

      // Create a "Graphics Context"

      
      */
      GC gc = XCreateGC(dpy, *barwin, 0, NIL);



      // Draw the Statusbar 
  
  int i = 0;
  for(;i < BAR_LEN;i++)
  {
    XSetForeground(dpy, gc, statusbar_colors[i]);
    XDrawLine(dpy, *barwin, gc, 0, i, screenWidth, i);
  }
  // Send the "DrawLine" request to the server
  

  printf("  nach vorschleife ");

  // drawa left an rigth border
  XSetForeground(dpy, gc, blackColor);
  XDrawLine(dpy, *barwin, gc, 0, 0, 0, BAR_LEN);
  XDrawLine(dpy, *barwin, gc, screenWidth, 0, screenWidth, BAR_LEN);
  
  printf("   vor Flush    ");

  
  XFlush(dpy);


  printf(" nach flush   ");
}
