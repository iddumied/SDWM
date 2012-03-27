#define ANZ_TEXT_BARS 1
#define ANZ_STYL_BARS 1
#define MAX_TABS 9
#include "typedef.c"

// order of SBar Status Symbols. first = right
SBarStatusSymbol sbar_status_symbols[] = {
  //  function      active
  { draw_time,      True },
  { draw_battery,   True },
  { draw_uptime,    True },
  { draw_memory,    True },
  { draw_termal,    True },
  { draw_backlight, True },
  { draw_audio,     True },
  { draw_net,       True },
};
// must be in the same order!
enum { DrawTime, DrawBattery, DrawUptime, DrawMemory, DrawTermal, DrawBacklight, DrawAudio, DrawNet, DrawLast };

/* tagging */
static const char *tags[] = { "main", "term", "dev", "img", "misc", "more" };

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "\x04",      ntile },    /* first entry is default */
	{ "\x03",      NULL },    /* no layout function means floating behavior */
	{ "\x01",      monocle },
  { "\x05",      nbstack },
  { "\x08",      bstackhoriz },
  { "\x09",      spiral },
  { "\x1b",      dwindle },
  { "\x07",      gaplessgrid },
// TODO remove title and bsack
};



static const Bool texttopbar        = True;       // False means bottom bar
static const Bool bottomborder      = True;	      // border of text bar
static const Bool readFromXroot     = False;      // if set sbar draws a collurfull statusbar in textmode
                                                  // with cpu and memory usage, battery clock and date
                                                  // controled by setings above
static const Bool showbattery       = True;
static const Bool cpu_in_middle     = True;       // if True cpu is placed in middel of bar
static int cpu_posx                 = 600;
static const int cpu_length         = 60;         // length of one cpu consider this if you have more
static const int distance_x         = 5;          // distance betwen cpu-cores if you have more than one
static const int distance_y         = 2;          // distance between symbols and top and bottom of bar

static const char memused_col[]      = "#00aaff";   //"#18B218";
static const char membuff_col[]      = "#ffffff";   //"#0F3FBA";
static const char memcach_col[]      = "#005588";   //"#FFFF00";

static const int status_refresh     = 1;           // refresh zyklus in seconds
static const Bool show_year         = True;

static const char mainsymbol[]      = "\x18";
static const Bool cpumem_timeline   = True;
static const Bool shift_left        = True;


static const Bool utime_seconds     = False;
static const int  tbar_distancex    = 12;

static const unsigned int gappx     = 6;
static const int max_link_quality   = 70; // max wlan quality found out by calling iwconfig

#define MAXPARTITIONS 20                    // max number of posible partitions mounted if you mount more it will crash


/******* SECTION INVORMATIONS *******/

// statuswin
static Bool draw_status_win           = True;
static const double blowstat          = 0.1;        // defines the warning stat of battery
static const double bverylowstat      = 0.12;      // 1 is full 0 is empty
static const double bat_suspend       = 0.075;       // percent when go to suspend
static const int  stwwidth            = 1366;




/********* SECTION COLORS **********/

// normal color
static const char normbordercolor[]   = "#005588";  // border of inactiv window
static const char normbgcolor[]       = "#000000";  // color of statsusbar
static const char normfgcolor[]       = "#ffffff";  // font color of statusbar
static const char selbordercolor[]    = "#00aaff";  // border of active window 
static const char selbgcolor[]        = "#000000";  // bgcolor of active text 
static const char selfgcolor[]        = "#00aaff";  // font color of active text
static const char botbordercolor[]    = "#00aaff";  // color of bottom border
static const char cpu_line_color[]    = "#21536a";  // color of cpu timeline line
static const char cpu_point_color[]   = "#00aaff";  // color of cpu timeline point

#include "themes.c"
