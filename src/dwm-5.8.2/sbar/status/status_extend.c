#include <errno.h>
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

void setup_statuswin();

DC statuswin;
// TODO REPLEACE SBarCpuLine ua. with SBarLine
void setup_statuswin()
{
    statuswin.norm[ColBorder]    = getcolor(swnormbordercolor);
    statuswin.norm[ColBG]        = getcolor(swnormbgcolor);
    statuswin.norm[ColFG]        = getcolor(swnormfgcolor);
    statuswin.sel[ColBorder]     = getcolor(swselbordercolor);
    statuswin.sel[ColBG]         = getcolor(swselbgcolor);
    statuswin.sel[ColFG]         = getcolor(swselfgcolor);
    statuswin.sbar[SBarBorder]   = getcolor(swbotbordercolor);
    statuswin.sbar[SBarCpuLine]  = getcolor(swcpu_line_color);
    statuswin.sbar[SBarCpuPoint] = getcolor(swcpu_point_color);
    
    
    
}
