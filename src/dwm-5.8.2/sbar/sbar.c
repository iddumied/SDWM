#include <time.h>
#include <sys/time.h>
#include "config.h"
#include "status/status.c"



double calc_time_div(struct timeval t_end, struct timeval t_star);
void setup_sbar();
void changeTheme();

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
  sbarcolor.normbordercolor = dc.norm[ColBorder]    = getcolor(themes[CurTheme].normal.normbordercolor);
  sbarcolor.normbgcolor     = dc.norm[ColBG]        = getcolor(themes[CurTheme].normal.normbgcolor);
  sbarcolor.normfgcolor     = dc.norm[ColFG]        = getcolor(themes[CurTheme].normal.normfgcolor);
  sbarcolor.selbordercolor  = dc.sel[ColBorder]     = getcolor(themes[CurTheme].normal.selbordercolor);
  sbarcolor.selbgcolor      = dc.sel[ColBG]         = getcolor(themes[CurTheme].normal.selbgcolor);
  sbarcolor.selfgcolor      = dc.sel[ColFG]         = getcolor(themes[CurTheme].normal.selfgcolor);
  sbarcolor.botbordercolor  = dc.sbar[SBarBorder]   = getcolor(themes[CurTheme].normal.botbordercolor);
  sbarcolor.cpu_line        = dc.sbar[SBarCpuLine]  = getcolor(themes[CurTheme].normal.timeln_line_color);
  sbarcolor.cpu_point       = dc.sbar[SBarCpuPoint] = getcolor(themes[CurTheme].normal.timeln_point_color);
    
  // apperance by low battery
  sbarcolor.bnormbgcolor      = getcolor(themes[CurTheme].low.normbgcolor);
  sbarcolor.bnormbordercolor  = getcolor(themes[CurTheme].low.normbordercolor);
  sbarcolor.bnormfgcolor      = getcolor(themes[CurTheme].low.normfgcolor);
  sbarcolor.bselbgcolor       = getcolor(themes[CurTheme].low.selbgcolor);
  sbarcolor.bselbordercolor   = getcolor(themes[CurTheme].low.selbordercolor);
  sbarcolor.bselfgcolor       = getcolor(themes[CurTheme].low.selfgcolor);
  sbarcolor.bbotbordercolor   = getcolor(themes[CurTheme].low.botbordercolor);
  sbarcolor.bcpu_line         = getcolor(themes[CurTheme].low.timeln_line_color);
  sbarcolor.bcpu_point        = getcolor(themes[CurTheme].low.timeln_point_color);

  // apperance by very low battery
  sbarcolor.bbnormbgcolor     = getcolor(themes[CurTheme].verylow.normbgcolor);
  sbarcolor.bbnormbordercolor = getcolor(themes[CurTheme].verylow.normbordercolor);
  sbarcolor.bbnormfgcolor     = getcolor(themes[CurTheme].verylow.normfgcolor);
  sbarcolor.bbnormfgcolor     = getcolor(themes[CurTheme].verylow.normfgcolor);
  sbarcolor.bbselbgcolor      = getcolor(themes[CurTheme].verylow.selbgcolor);
  sbarcolor.bbselbordercolor  = getcolor(themes[CurTheme].verylow.selbordercolor);
  sbarcolor.bbselfgcolor      = getcolor(themes[CurTheme].verylow.selfgcolor);
  sbarcolor.bbbotbordercolor  = getcolor(themes[CurTheme].verylow.botbordercolor);
  sbarcolor.bbcpu_line        = getcolor(themes[CurTheme].verylow.timeln_line_color);
  sbarcolor.bbcpu_point       = getcolor(themes[CurTheme].verylow.timeln_point_color);
    
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
  stw.sbar[SBarCpuLine]  = getcolor(themes[CurTheme].stw.timeln_line_color);
  stw.sbar[SBarCpuPoint] = getcolor(themes[CurTheme].stw.timeln_point_color);

  // normal apperance
  sbarcolor.normbordercolor = dc.norm[ColBorder]    = getcolor(themes[CurTheme].normal.normbordercolor);
  sbarcolor.normbgcolor     = dc.norm[ColBG]        = getcolor(themes[CurTheme].normal.normbgcolor);
  sbarcolor.normfgcolor     = dc.norm[ColFG]        = getcolor(themes[CurTheme].normal.normfgcolor);
  sbarcolor.selbordercolor  = dc.sel[ColBorder]     = getcolor(themes[CurTheme].normal.selbordercolor);
  sbarcolor.selbgcolor      = dc.sel[ColBG]         = getcolor(themes[CurTheme].normal.selbgcolor);
  sbarcolor.selfgcolor      = dc.sel[ColFG]         = getcolor(themes[CurTheme].normal.selfgcolor);
  sbarcolor.botbordercolor  = dc.sbar[SBarBorder]   = getcolor(themes[CurTheme].normal.botbordercolor);
  sbarcolor.cpu_line        = dc.sbar[SBarCpuLine]  = getcolor(themes[CurTheme].normal.timeln_line_color);
  sbarcolor.cpu_point       = dc.sbar[SBarCpuPoint] = getcolor(themes[CurTheme].normal.timeln_point_color);
    
  // apperance by low battery
  sbarcolor.bnormbgcolor      = getcolor(themes[CurTheme].low.normbgcolor);
  sbarcolor.bnormbordercolor  = getcolor(themes[CurTheme].low.normbordercolor);
  sbarcolor.bnormfgcolor      = getcolor(themes[CurTheme].low.normfgcolor);
  sbarcolor.bselbgcolor       = getcolor(themes[CurTheme].low.selbgcolor);
  sbarcolor.bselbordercolor   = getcolor(themes[CurTheme].low.selbordercolor);
  sbarcolor.bselfgcolor       = getcolor(themes[CurTheme].low.selfgcolor);
  sbarcolor.bbotbordercolor   = getcolor(themes[CurTheme].low.botbordercolor);
  sbarcolor.bcpu_line         = getcolor(themes[CurTheme].low.timeln_line_color);
  sbarcolor.bcpu_point        = getcolor(themes[CurTheme].low.timeln_point_color);

  // apperance by very low battery
  sbarcolor.bbnormbgcolor     = getcolor(themes[CurTheme].verylow.normbgcolor);
  sbarcolor.bbnormbordercolor = getcolor(themes[CurTheme].verylow.normbordercolor);
  sbarcolor.bbnormfgcolor     = getcolor(themes[CurTheme].verylow.normfgcolor);
  sbarcolor.bbnormfgcolor     = getcolor(themes[CurTheme].verylow.normfgcolor);
  sbarcolor.bbselbgcolor      = getcolor(themes[CurTheme].verylow.selbgcolor);
  sbarcolor.bbselbordercolor  = getcolor(themes[CurTheme].verylow.selbordercolor);
  sbarcolor.bbselfgcolor      = getcolor(themes[CurTheme].verylow.selfgcolor);
  sbarcolor.bbbotbordercolor  = getcolor(themes[CurTheme].verylow.botbordercolor);
  sbarcolor.bbcpu_line        = getcolor(themes[CurTheme].verylow.timeln_line_color);
  sbarcolor.bbcpu_point       = getcolor(themes[CurTheme].verylow.timeln_point_color);

  // map status win
  if(!draw_status_win)
    togglestw();

}
