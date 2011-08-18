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

void setup_stw();
void drawstw();

DC stw;
Pixmap background;

void drawstw()
{
  XCopyArea(dpy, stw.drawable, selmon->statuswin, sw.gc, 0, 0, sw.w, sw.h, sw.x, sw.y );
}



// TODO REPLEACE SBarCpuLine ua. with SBarLine
void setup_stw()
{
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
    
    stw.w = swwidth;
    stw.y = dc.h;
    stw.x = ScrenWidth-sswidth;
    stw.h = ScreenHeight-bh;
    
    stw.drawable = XCreatePixmap(dpy, root, swwidth, ScreenHeight, DefaultDepth(dpy, screen));
    background  = XCreatePixmap(dpy, root, stwwidth, ScreenHeight, DefaultDepth(dpy, screen));
    stw.gc = XCreateGC(dpy, statuswin.drawable);
    
    //XCopyArea(dpy, root, stw.drawable, stw.gc, stw.x, stw.y, stw.w, st.h, 0,0 );
    XCopyArea(dpy, root, background, stw.gc, stw.x, stw.y, stw.w, stw.h, 0,0 );
}
