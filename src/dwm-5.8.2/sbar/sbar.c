#include <time.h>
#include <sys/time.h>
#include "config.h"
#include "status/status.c"



double calc_time_div(struct timeval t_end, struct timeval t_star);
void setup_sbar();

static SBarColor sbarcolor;
static SBar sbars[ANZ_STYL_BARS];
static TBar tbar;
static Bool topbar;    			 /* False means bottom bar */
static int screenWidth, screenHeight;
static Bool sbarloaded;
static Bool tbarloaded = False;
static Pixmap *timeline;
static Bool status_symbol_mode[DrawLast];

#include "statuswin.c"

double calc_time_div(struct timeval t_end, struct timeval t_star)
{
  double res = (double)t_end.tv_sec - (long)t_star.tv_sec;
  res = res * 1000000;
  double temp = (double)t_end.tv_usec - (long)t_star.tv_usec;
  if(temp < 0)
  {
    res -= 1000000;
    temp = 1000000 + temp;
  }
  res += temp;
  res /= 1000000;
  return res;
}


  static struct timeval start_time, end_time;

void setup_sbar()
{
  //struct timeval start_time, end_time;
  gettimeofday(&start_time, 0);
  
  int i, j, status;

  tbarloaded = True;
    

  screenWidth  = DisplayWidth(dpy, DefaultScreen(dpy));
  screenHeight = DisplayHeight(dpy, DefaultScreen(dpy));
     
  setup_stw();
    

  sbarcolor.red            = getcolor("#FF0000");
  sbarcolor.green          = getcolor("#00FF00");
  sbarcolor.redlow         = getcolor("#010000");
  sbarcolor.greenlow       = getcolor("#000100");
  sbarcolor.black          = getcolor("#000000");
  sbarcolor.white          = getcolor("#ffffff");
  sbarcolor.blue           = getcolor("#0000ff");
  sbarcolor.yellow         = getcolor("#ffff00");
  sbarcolor.membuff_line   = getcolor("#aaaaaa");
  sbarcolor.memcach_line   = getcolor("#005588");
  sbarcolor.memcach_point  = getcolor("#002a44");
  sbarcolor.cpu_line       = getcolor(cpu_line_color);
    
  // normal apperance
  sbarcolor.normbordercolor = dc.norm[ColBorder]    = getcolor(normbordercolor);
  sbarcolor.normbgcolor     = dc.norm[ColBG]        = getcolor(normbgcolor);
  sbarcolor.normfgcolor     = dc.norm[ColFG]        = getcolor(normfgcolor);
  sbarcolor.selbordercolor  = dc.sel[ColBorder]     = getcolor(selbordercolor);
  sbarcolor.selbgcolor      = dc.sel[ColBG]         = getcolor(selbgcolor);
  sbarcolor.selfgcolor      = dc.sel[ColFG]         = getcolor(selfgcolor);
  sbarcolor.botbordercolor  = dc.sbar[SBarBorder]   = getcolor(botbordercolor);
  sbarcolor.cpu_line        = dc.sbar[SBarCpuLine]  = getcolor(cpu_line_color);
  sbarcolor.cpu_point       = dc.sbar[SBarCpuPoint] = getcolor(cpu_point_color);
    
  // apperance by low battery
  sbarcolor.bnormbgcolor      = getcolor(bnormbgcolor);
  sbarcolor.bnormbordercolor  = getcolor(bnormbordercolor);
  sbarcolor.bnormfgcolor      = getcolor(bnormfgcolor);
  sbarcolor.bselbgcolor       = getcolor(bselbgcolor);
  sbarcolor.bselbordercolor   = getcolor(bselbordercolor);
  sbarcolor.bselfgcolor       = getcolor(bselfgcolor);
  sbarcolor.bbotbordercolor   = getcolor(bbotbordercolor);
  sbarcolor.bcpu_line         = getcolor(bcpu_line_color);
  sbarcolor.bcpu_point        = getcolor(bcpu_point_color);

  // apperance by very low battery
  sbarcolor.bbnormbgcolor     = getcolor(bbnormbgcolor);
  sbarcolor.bbnormbordercolor = getcolor(bbnormbordercolor);
  sbarcolor.bbnormfgcolor     = getcolor(bbnormfgcolor);
  sbarcolor.bbnormfgcolor     = getcolor(bbnormfgcolor);
  sbarcolor.bbselbgcolor      = getcolor(bbselbgcolor);
  sbarcolor.bbselbordercolor  = getcolor(bbselbordercolor);
  sbarcolor.bbselfgcolor      = getcolor(bbselfgcolor);
  sbarcolor.bbbotbordercolor  = getcolor(bbbotbordercolor);
  sbarcolor.bbcpu_line        = getcolor(bbcpu_line_color);
  sbarcolor.bbcpu_point       = getcolor(bbcpu_point_color);
    
  // int bar length
  bh = dc.h = dc.font.height + 2;
  if(bottomborder) bh++;
  topbar = texttopbar;
  sbarloaded = False;
  // setup status calculating need bh
  setup_status();
      
  if(cpu_in_middle) cpu_posx = screenWidth/2 - (cpu_length*ncpus  + (ncpus-1)*distance_x)/2;

  if(cpumem_timeline){
    timeline = (Pixmap*)malloc(sizeof(Pixmap)*ncpus+1);
        
  for(i = 0;i < ncpus+1;i++)
    timeline[i] = XCreatePixmap(dpy, root, cpu_length, bh-1, DefaultDepth(dpy, screen));
  }//hier timline initialisieren mit fillrectangle

  

  gettimeofday(&end_time, 0);
  printf("\nsbar Setup needed:  %f Seconds\n", calc_time_div(end_time, start_time));  
}

