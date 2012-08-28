#include <time.h>
#include <sys/time.h>
#include "config.h"
#include "status/status.c"



double calc_time_div(struct timeval t_end, struct timeval t_star);
void setup_sbar();
void changeTheme();
  
static int screenWidth, screenHeight;

typedef struct {
  SBarColor colors;
  Bool topbar;    			                /* False means bottom bar */
  Bool sbarloaded;
  Bool status_symbol_mode[DrawLast];
  Pixmap *cpu_timeline;
  int cpu_posx;
} SBar;

SBar sbar;

#include "statuswin.c"
#include "freestylebar.c"

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


void setup_sbar()
{
  struct timeval start_time, end_time;
  gettimeofday(&start_time, 0);
  
  int i, j, status;


  screenWidth  = DisplayWidth(dpy, DefaultScreen(dpy));
  screenHeight = DisplayHeight(dpy, DefaultScreen(dpy));

  // int bar length
  bh++; // because of bottomborder height + 1
  sbar.topbar = sbartopbar;
  sbar.sbarloaded = False;

  // setup status calculating need bh
  setup_status();

  // cpu_posx calculation need setup_cpu
  sbar.cpu_posx = screenWidth/2 - (cpu_length*cpuinfo.ncpus  + (cpuinfo.ncpus-1)*distance_x)/2;

  setup_stw();
  setup_freestylebar();    


  sbar.colors.red            = getcolor("#FF0000");
  sbar.colors.green          = getcolor("#00FF00");
  sbar.colors.redlow         = getcolor("#010000");
  sbar.colors.greenlow       = getcolor("#000100");
  sbar.colors.black          = getcolor("#000000");
  sbar.colors.white          = getcolor("#ffffff");
  sbar.colors.blue           = getcolor("#0000ff");
  sbar.colors.yellow         = getcolor("#ffff00");
  sbar.colors.membuff_line   = getcolor("#aaaaaa");
  sbar.colors.memcach_line   = getcolor("#005588");
  sbar.colors.memcach_point  = getcolor("#002a44");
  sbar.colors.cpu_line       = getcolor(cpu_line_color);
    
  // normal apperance
  sbar.colors.normbordercolor = dc.norm[ColBorder]    = getcolor(themes[CurTheme].normal.normbordercolor);
  sbar.colors.normbgcolor     = dc.norm[ColBG]        = getcolor(themes[CurTheme].normal.normbgcolor);
  sbar.colors.normfgcolor     = dc.norm[ColFG]        = getcolor(themes[CurTheme].normal.normfgcolor);
  sbar.colors.selbordercolor  = dc.sel[ColBorder]     = getcolor(themes[CurTheme].normal.selbordercolor);
  sbar.colors.selbgcolor      = dc.sel[ColBG]         = getcolor(themes[CurTheme].normal.selbgcolor);
  sbar.colors.selfgcolor      = dc.sel[ColFG]         = getcolor(themes[CurTheme].normal.selfgcolor);
  sbar.colors.botbordercolor  = dc.sbar[SBarBorder]   = getcolor(themes[CurTheme].normal.botbordercolor);
  sbar.colors.cpu_line        = dc.sbar[SBarLine]  = getcolor(themes[CurTheme].normal.timeln_line_color);
  sbar.colors.cpu_point       = dc.sbar[SBarCpuPoint] = getcolor(themes[CurTheme].normal.timeln_point_color);
    
  // apperance by low battery
  sbar.colors.bnormbgcolor      = getcolor(themes[CurTheme].low.normbgcolor);
  sbar.colors.bnormbordercolor  = getcolor(themes[CurTheme].low.normbordercolor);
  sbar.colors.bnormfgcolor      = getcolor(themes[CurTheme].low.normfgcolor);
  sbar.colors.bselbgcolor       = getcolor(themes[CurTheme].low.selbgcolor);
  sbar.colors.bselbordercolor   = getcolor(themes[CurTheme].low.selbordercolor);
  sbar.colors.bselfgcolor       = getcolor(themes[CurTheme].low.selfgcolor);
  sbar.colors.bbotbordercolor   = getcolor(themes[CurTheme].low.botbordercolor);
  sbar.colors.bcpu_line         = getcolor(themes[CurTheme].low.timeln_line_color);
  sbar.colors.bcpu_point        = getcolor(themes[CurTheme].low.timeln_point_color);

  // apperance by very low battery
  sbar.colors.bbnormbgcolor     = getcolor(themes[CurTheme].verylow.normbgcolor);
  sbar.colors.bbnormbordercolor = getcolor(themes[CurTheme].verylow.normbordercolor);
  sbar.colors.bbnormfgcolor     = getcolor(themes[CurTheme].verylow.normfgcolor);
  sbar.colors.bbnormfgcolor     = getcolor(themes[CurTheme].verylow.normfgcolor);
  sbar.colors.bbselbgcolor      = getcolor(themes[CurTheme].verylow.selbgcolor);
  sbar.colors.bbselbordercolor  = getcolor(themes[CurTheme].verylow.selbordercolor);
  sbar.colors.bbselfgcolor      = getcolor(themes[CurTheme].verylow.selfgcolor);
  sbar.colors.bbbotbordercolor  = getcolor(themes[CurTheme].verylow.botbordercolor);
  sbar.colors.bbcpu_line        = getcolor(themes[CurTheme].verylow.timeln_line_color);
  sbar.colors.bbcpu_point       = getcolor(themes[CurTheme].verylow.timeln_point_color);
    
  sbar.cpu_timeline = (Pixmap*)malloc(sizeof(Pixmap)*cpuinfo.ncpus+1);
      
  for(i = 0;i < cpuinfo.ncpus+1;i++)
    sbar.cpu_timeline[i] = XCreatePixmap(dpy, root, cpu_length, bh-1, DefaultDepth(dpy, screen));

  gettimeofday(&end_time, 0);
  printf("\nsbar Setup needed:  %f Seconds\n", calc_time_div(end_time, start_time));  
}

void toggletheme(){
  CurTheme = (CurTheme+1)%ThemeLast;
  changeTheme();
}

void changeTheme(){

  char *bgimgcmd;
  bgimgcmd = (char *)malloc(sizeof(char) * (strlen(themes[CurTheme].image) + 20));
  sprintf(bgimgcmd,"feh --bg-fill '%s'",themes[CurTheme].image);

  FILE *fp;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  Client *c;

  // destroying all Clients
	for(c = selmon->stack; c; c = c->snext)
    XDestroyWindow(dpy,c->win);

  // unmap status win if mapped
  if(draw_status_win)
    togglestw();

  // applaing new bg-image
  fp = popen(bgimgcmd,"r");
  while((read = getline(&line,&len,fp)) != -1);
  free(line);
  fclose(fp);


	XSync(dpy, False);
  XFlush(dpy);
  sleep(1);
  // coping new Image to stws bg
  XCopyArea(dpy, root, background, stw.gc, stw.x, stw.y, stw.w, stw.h, 0,0 );

  // status win apperance
  stw.norm[ColBorder]    = getcolor(themes[CurTheme].stw.normbordercolor);
  stw.norm[ColBG]        = getcolor(themes[CurTheme].stw.normbgcolor);
  stw.norm[ColFG]        = getcolor(themes[CurTheme].stw.normfgcolor);
  stw.sel[ColBorder]     = getcolor(themes[CurTheme].stw.selbordercolor);
  stw.sel[ColBG]         = getcolor(themes[CurTheme].stw.selbgcolor);
  stw.sel[ColFG]         = getcolor(themes[CurTheme].stw.selfgcolor);
  stw.sbar[SBarBorder]   = getcolor(themes[CurTheme].stw.botbordercolor);
  stw.sbar[SBarLine]  = getcolor(themes[CurTheme].stw.timeln_line_color);
  stw.sbar[SBarCpuPoint] = getcolor(themes[CurTheme].stw.timeln_point_color);

  // normal apperance
  sbar.colors.normbordercolor = dc.norm[ColBorder]    = getcolor(themes[CurTheme].normal.normbordercolor);
  sbar.colors.normbgcolor     = dc.norm[ColBG]        = getcolor(themes[CurTheme].normal.normbgcolor);
  sbar.colors.normfgcolor     = dc.norm[ColFG]        = getcolor(themes[CurTheme].normal.normfgcolor);
  sbar.colors.selbordercolor  = dc.sel[ColBorder]     = getcolor(themes[CurTheme].normal.selbordercolor);
  sbar.colors.selbgcolor      = dc.sel[ColBG]         = getcolor(themes[CurTheme].normal.selbgcolor);
  sbar.colors.selfgcolor      = dc.sel[ColFG]         = getcolor(themes[CurTheme].normal.selfgcolor);
  sbar.colors.botbordercolor  = dc.sbar[SBarBorder]   = getcolor(themes[CurTheme].normal.botbordercolor);
  sbar.colors.cpu_line        = dc.sbar[SBarLine]  = getcolor(themes[CurTheme].normal.timeln_line_color);
  sbar.colors.cpu_point       = dc.sbar[SBarCpuPoint] = getcolor(themes[CurTheme].normal.timeln_point_color);
    
  // apperance by low battery
  sbar.colors.bnormbgcolor      = getcolor(themes[CurTheme].low.normbgcolor);
  sbar.colors.bnormbordercolor  = getcolor(themes[CurTheme].low.normbordercolor);
  sbar.colors.bnormfgcolor      = getcolor(themes[CurTheme].low.normfgcolor);
  sbar.colors.bselbgcolor       = getcolor(themes[CurTheme].low.selbgcolor);
  sbar.colors.bselbordercolor   = getcolor(themes[CurTheme].low.selbordercolor);
  sbar.colors.bselfgcolor       = getcolor(themes[CurTheme].low.selfgcolor);
  sbar.colors.bbotbordercolor   = getcolor(themes[CurTheme].low.botbordercolor);
  sbar.colors.bcpu_line         = getcolor(themes[CurTheme].low.timeln_line_color);
  sbar.colors.bcpu_point        = getcolor(themes[CurTheme].low.timeln_point_color);

  // apperance by very low battery
  sbar.colors.bbnormbgcolor     = getcolor(themes[CurTheme].verylow.normbgcolor);
  sbar.colors.bbnormbordercolor = getcolor(themes[CurTheme].verylow.normbordercolor);
  sbar.colors.bbnormfgcolor     = getcolor(themes[CurTheme].verylow.normfgcolor);
  sbar.colors.bbnormfgcolor     = getcolor(themes[CurTheme].verylow.normfgcolor);
  sbar.colors.bbselbgcolor      = getcolor(themes[CurTheme].verylow.selbgcolor);
  sbar.colors.bbselbordercolor  = getcolor(themes[CurTheme].verylow.selbordercolor);
  sbar.colors.bbselfgcolor      = getcolor(themes[CurTheme].verylow.selfgcolor);
  sbar.colors.bbbotbordercolor  = getcolor(themes[CurTheme].verylow.botbordercolor);
  sbar.colors.bbcpu_line        = getcolor(themes[CurTheme].verylow.timeln_line_color);
  sbar.colors.bbcpu_point       = getcolor(themes[CurTheme].verylow.timeln_point_color);

  // map status win
  if(!draw_status_win)
    togglestw();

}
