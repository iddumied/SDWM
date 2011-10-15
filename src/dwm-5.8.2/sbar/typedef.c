typedef struct {
  char date[30], mem[10], battery[10], uptime[20];  
} TBar;

typedef struct {
  unsigned long red, redlow;
  unsigned long green, greenlow;
  unsigned long white, black, blue, yellow;
  unsigned long membuff_line, memcach_line, memcach_point;
  unsigned long normbordercolor, normbgcolor, normfgcolor, selbordercolor, selbgcolor, selfgcolor, botbordercolor, cpu_line, cpu_point;
  unsigned long bnormbordercolor, bnormbgcolor, bnormfgcolor, bselbordercolor, bselbgcolor, bselfgcolor, bbotbordercolor, bcpu_line, bcpu_point;
  unsigned long bbnormbordercolor, bbnormbgcolor, bbnormfgcolor, bbselbordercolor, bbselbgcolor, bbselfgcolor, bbbotbordercolor, bbcpu_line, bbcpu_point;
} SBarColor;


typedef struct {
  int w, h, pos_x, pos_y;
  Pixmap has_win; // with_windows
  Pixmap has_activ_win; // with active window
  Pixmap empty_active; // without window and active
  Pixmap full_active; // witout window but active
} STab;


typedef struct {
  int w, h, ntabs;
  Bool topbar;
  Pixmap drawable;
  STab * tabs;  
} SBar;

typedef struct {
  int w, h, pos_x, pos_y;
  char * has_win; // with_windows
  char * has_activ_win; // with active window
  char * empty_active; // without window and active
  char * full_active; // witout window but active
} ConfTab;

typedef struct {
  int h, ntabs;
  Bool topbar;
  char *bg;
  ConfTab tabs[MAX_TABS];  
} ConfBar;

typedef struct {
  Bool battery, textbar, border, redroot;
  unsigned long bordercolor;
} SBarConf;

typedef struct {
  int (*func) (int, int);
  Bool active;
} SBarStatusSymbol;


enum { DrawTime, DrawBattery, DrawUptime, DrawMemory, DrawTermal, DrawBacklight, DrawAudio, DrawNet, DrawLast };



SBarStatusSymbol sbar_status_symbols[] = {
//  function        active
  { draw_time,      True },
  { draw_battery,   False },
  { draw_uptime,    True },
  { draw_memory,    True },
  { draw_termal,    True },
  { draw_backlight, True },
  { draw_audio,     True },
  { draw_net,       True },
}
